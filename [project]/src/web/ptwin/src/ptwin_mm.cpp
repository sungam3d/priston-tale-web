// ptwin_mm.cpp - winmm for the web build: multimedia file I/O (mmio, used to
// stream background music .wav files) and multimedia timers (timeSetEvent, which
// drives the music streamer from its own thread, as on Windows).
#include "ptwin_internal.h"
#include <mmsystem.h>
#include <unistd.h>
#include <atomic>

std::string PtFs_Resolve( const char *szPath );

//////////////////////////////////////////////////////////////////////////////
// mmio: read side (RIFF chunk navigation, direct/buffered reads)
//////////////////////////////////////////////////////////////////////////////
#define PTMMIO_BUF 16384

struct PtMmio {
	FILE	*fp;
	LONG	pos;					// logical file position
	LONG	bufFileOff;				// file offset of buffer[0] (buffered access)
	char	buffer[PTMMIO_BUF];
};

static PtMmio *M( HMMIO h ) { return (PtMmio *)h; }

HMMIO WINAPI mmioOpenA( LPSTR szFile, LPMMIOINFO, DWORD flags )
{
	if ( !szFile ) return NULL;
	const char *mode = ( flags & MMIO_CREATE ) ? "w+b" : ( flags & ( MMIO_WRITE | MMIO_READWRITE ) ) ? "r+b" : "rb";
	FILE *fp = fopen( PtFs_Resolve( szFile ).c_str(), mode );
	if ( !fp ) return NULL;
	PtMmio *m = new PtMmio;
	m->fp = fp;
	m->pos = 0;
	m->bufFileOff = 0;
	return (HMMIO)m;
}

MMRESULT WINAPI mmioClose( HMMIO h, UINT )
{
	PtMmio *m = M( h );
	if ( !m ) return MMIOERR_CANNOTCLOSE;
	fclose( m->fp );
	delete m;
	return 0;
}

LONG WINAPI mmioRead( HMMIO h, HPSTR buf, LONG n )
{
	PtMmio *m = M( h );
	if ( !m || n<0 ) return -1;
	fseek( m->fp, m->pos, SEEK_SET );
	LONG got = (LONG)fread( buf, 1, n, m->fp );
	m->pos += got;
	return got;
}

LONG WINAPI mmioWrite( HMMIO h, const char *buf, LONG n )
{
	PtMmio *m = M( h );
	if ( !m || n<0 ) return -1;
	fseek( m->fp, m->pos, SEEK_SET );
	LONG put = (LONG)fwrite( buf, 1, n, m->fp );
	m->pos += put;
	return put;
}

LONG WINAPI mmioSeek( HMMIO h, LONG off, int origin )
{
	PtMmio *m = M( h );
	if ( !m ) return -1;
	if ( origin==SEEK_SET ) m->pos = off;
	else if ( origin==SEEK_CUR ) m->pos += off;
	else { fseek( m->fp, 0, SEEK_END ); m->pos = ftell( m->fp ) + off; }
	return m->pos;
}

static bool ReadDword( PtMmio *m, DWORD *p ) { return mmioRead( (HMMIO)m, (HPSTR)p, 4 )==4; }

MMRESULT WINAPI mmioDescend( HMMIO h, LPMMCKINFO ck, const MMCKINFO *parent, UINT flags )
{
	PtMmio *m = M( h );
	if ( !m || !ck ) return MMIOERR_CHUNKNOTFOUND;
	LONG end = parent ? (LONG)( parent->dwDataOffset + parent->cksize ) : 0x7FFFFFFF;
	FOURCC wantId = ck->ckid, wantType = ck->fccType;
	for ( ;; ) {
		if ( m->pos + 8 > end ) return MMIOERR_CHUNKNOTFOUND;
		MMCKINFO c;
		ZeroMemory( &c, sizeof(c) );
		if ( !ReadDword( m, &c.ckid ) || !ReadDword( m, &c.cksize ) ) return MMIOERR_CHUNKNOTFOUND;
		c.dwDataOffset = (DWORD)m->pos;
		bool isList = c.ckid==FOURCC_RIFF || c.ckid==FOURCC_LIST;
		if ( isList && !ReadDword( m, &c.fccType ) ) return MMIOERR_CHUNKNOTFOUND;
		bool match = true;
		if ( flags & MMIO_FINDCHUNK ) match = c.ckid==wantId;
		else if ( flags & MMIO_FINDRIFF ) match = c.ckid==FOURCC_RIFF && c.fccType==wantType;
		else if ( flags & MMIO_FINDLIST ) match = c.ckid==FOURCC_LIST && c.fccType==wantType;
		if ( match ) { *ck = c; return 0; }
		m->pos = (LONG)( c.dwDataOffset + ( ( c.cksize + 1 ) & ~1u ) );		// skip (chunks are word aligned)
	}
}

MMRESULT WINAPI mmioAscend( HMMIO h, LPMMCKINFO ck, UINT )
{
	PtMmio *m = M( h );
	if ( !m || !ck ) return MMIOERR_CANNOTSEEK;
	m->pos = (LONG)( ck->dwDataOffset + ( ( ck->cksize + 1 ) & ~1u ) );
	return 0;
}

MMRESULT WINAPI mmioCreateChunk( HMMIO h, LPMMCKINFO ck, UINT flags )
{
	PtMmio *m = M( h );
	if ( !m || !ck ) return MMIOERR_CANNOTWRITE;
	if ( flags & MMIO_CREATERIFF ) ck->ckid = FOURCC_RIFF;
	if ( flags & MMIO_CREATELIST ) ck->ckid = FOURCC_LIST;
	mmioWrite( h, (const char *)&ck->ckid, 4 );
	mmioWrite( h, (const char *)&ck->cksize, 4 );
	ck->dwDataOffset = (DWORD)m->pos;
	if ( ck->ckid==FOURCC_RIFF || ck->ckid==FOURCC_LIST ) mmioWrite( h, (const char *)&ck->fccType, 4 );
	return 0;
}

// Buffered access: the caller reads straight out of our buffer between
// pchNext and pchEndRead and calls mmioAdvance for more.
static void FillBuffer( PtMmio *m, LPMMIOINFO info )
{
	m->bufFileOff = m->pos;
	fseek( m->fp, m->pos, SEEK_SET );
	LONG got = (LONG)fread( m->buffer, 1, PTMMIO_BUF, m->fp );
	info->pchBuffer = m->buffer;
	info->cchBuffer = PTMMIO_BUF;
	info->pchNext = m->buffer;
	info->pchEndRead = m->buffer + got;
	info->pchEndWrite = m->buffer + PTMMIO_BUF;
	info->lBufOffset = m->bufFileOff;
	info->lDiskOffset = m->bufFileOff + got;
}

MMRESULT WINAPI mmioGetInfo( HMMIO h, LPMMIOINFO info, UINT )
{
	PtMmio *m = M( h );
	if ( !m || !info ) return MMSYSERR_INVALHANDLE;
	ZeroMemory( info, sizeof(*info) );
	info->hmmio = h;
	FillBuffer( m, info );
	return 0;
}

MMRESULT WINAPI mmioAdvance( HMMIO h, LPMMIOINFO info, UINT flags )
{
	PtMmio *m = M( h );
	if ( !m || !info ) return MMSYSERR_INVALHANDLE;
	if ( flags & MMIO_WRITE ) {
		LONG n = (LONG)( info->pchNext - m->buffer );
		fseek( m->fp, m->bufFileOff, SEEK_SET );
		fwrite( m->buffer, 1, n, m->fp );
		m->pos = m->bufFileOff + n;
	}
	else m->pos = m->bufFileOff + (LONG)( info->pchNext - m->buffer );
	FillBuffer( m, info );
	return 0;
}

MMRESULT WINAPI mmioSetInfo( HMMIO h, LPCMMIOINFO info, UINT )
{
	PtMmio *m = M( h );
	if ( !m || !info ) return MMSYSERR_INVALHANDLE;
	if ( info->dwFlags & MMIO_DIRTY ) {
		LONG n = (LONG)( info->pchNext - m->buffer );
		fseek( m->fp, m->bufFileOff, SEEK_SET );
		fwrite( m->buffer, 1, n, m->fp );
	}
	m->pos = m->bufFileOff + (LONG)( info->pchNext - m->buffer );
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Multimedia timers: a thread per periodic timer
//////////////////////////////////////////////////////////////////////////////
struct PtMmTimer {
	UINT				id;
	UINT				period;
	LPTIMECALLBACK		proc;
	DWORD_PTR			user;
	UINT				flags;
	std::atomic<bool>	stop;
	std::atomic<bool>	selfDelete;		// killed from its own callback: the thread deletes it
	pthread_t			th;
};

static pthread_mutex_t				s_TimerLock = PTHREAD_MUTEX_INITIALIZER;
static std::map<UINT, PtMmTimer *>	s_MmTimers;
static UINT							s_NextTimerId = 1;

static void *TimerThread( void *p )
{
	PtMmTimer *t = (PtMmTimer *)p;
	do {
		usleep( t->period*1000 );
		if ( t->stop ) break;
		t->proc( t->id, 0, t->user, 0, 0 );
	} while ( ( t->flags & TIME_PERIODIC ) && !t->stop );
	if ( t->selfDelete ) { pthread_detach( pthread_self() ); delete t; }
	return NULL;
}

MMRESULT WINAPI timeSetEvent( UINT delay, UINT, LPTIMECALLBACK proc, DWORD_PTR user, UINT flags )
{
	PtMmTimer *t = new PtMmTimer;
	t->period = delay ? delay : 1;
	t->proc = proc;
	t->user = user;
	t->flags = flags;
	t->stop = false;
	t->selfDelete = false;
	pthread_mutex_lock( &s_TimerLock );
	t->id = s_NextTimerId++;
	s_MmTimers[t->id] = t;
	pthread_mutex_unlock( &s_TimerLock );
	if ( pthread_create( &t->th, NULL, TimerThread, t ) ) {
		PtWin_Log( "timeSetEvent: no thread available" );
		pthread_mutex_lock( &s_TimerLock );
		s_MmTimers.erase( t->id );
		pthread_mutex_unlock( &s_TimerLock );
		delete t;
		return 0;
	}
	return t->id;
}

MMRESULT WINAPI timeKillEvent( UINT id )
{
	pthread_mutex_lock( &s_TimerLock );
	auto it = s_MmTimers.find( id );
	PtMmTimer *t = it==s_MmTimers.end() ? NULL : it->second;
	if ( t ) s_MmTimers.erase( it );
	pthread_mutex_unlock( &s_TimerLock );
	if ( !t ) return MMSYSERR_INVALPARAM;
	t->stop = true;
	// From the timer's own callback (the music streamer stops itself at the end of a track):
	// a thread cannot wait for itself - it was deleted here while still running, and went on
	// reading the freed timer. Now it ends after the callback and deletes the timer itself.
	if ( pthread_equal( pthread_self(), t->th ) ) { t->selfDelete = true; return 0; }
	pthread_join( t->th, NULL );
	delete t;
	return 0;
}

MMRESULT WINAPI timeBeginPeriod( UINT ) { return 0; }
MMRESULT WINAPI timeEndPeriod( UINT ) { return 0; }
