/***************************************************************************
 *
 *  Copyright (C) 1995, 1996 Microsoft Corporation. All Rights Reserved.
 *
 *  File: directx.cpp
 *
 ***************************************************************************/
#include "dxWav.h"
#include <windowsx.h>
#include <mmsystem.h>
#include <conio.h>
#ifdef PT_GFX_GL
#include "../ptsound/ptsound.h"		// the music stream gets the master music volume
#endif
//#include "..\cl_main\defines.h"
//#include "..\cl_main\initgd.h"

HRESULT     rval_g_lpDs;				// pluto 비스타에서 채팅 때문에 윈도우 생성전에 사운드 만든다
HRESULT     dsrval_lpDirectSound;		// pluto 비스타에서 채팅 때문에 윈도우 생성전에 사운드 만든다

int numSound = 0;

char dataDir[64];

//mouth관련
int mouThRead;
BYTE *mouThdataOfs = NULL;

BOOL wav_opt = 1;           //wav option check(Dxwav.cpp)

LPDIRECTSOUND           g_lpDS = NULL;              // DirectSound object
BOOL                    g_bSoundPresent = FALSE;    // Do we have sound capabilites?
LPDIRECTSOUNDBUFFER     g_lpSounds[NUM_SOUNDS];     // Sound buffers
BOOL                    g_bSoundPaused = FALSE;     // Defined in WINMAIN.CPP


char sangJuWav[NUM_SOUNDS][64];
int  sangJuWav_F[NUM_SOUNDS] = {1,};		//플레그셋팅

/*
int chk_load_sangJuWav(char *wavName) 
{
	int i;
	for(i=0; i<NUM_SOUNDS; i++) {
		if(sangJuWav[i][0]==wavName[0]) {
			if ( _stricmp(sangJuWav[i], wavName) == 0 ) {
				if(sangJuWav_F[i] == 1) return 1;			//이미 로드됨

				sangJuWav_F[i] = 1;		//플레그 셋팅
				CreateBufferFromWaveFile(sangJuWav[i], i);
				return 1;
			}
		}
		
	}
	return -1;
}
*/


//플레그를 이용해 중간 로드함.
int chk_load_sangJuWav(int wavN) 
{
	if(sangJuWav_F[wavN-1] == 1) return 1;			//이미 로드됨
	sangJuWav_F[wavN-1] = 1;		//플레그 셋팅
	CreateBufferFromWaveFile(sangJuWav[wavN-1], wavN);
	
	return 1;
}


char *sangJuWav2[NUM_SOUNDS]= {
"wav\\rain01.wav",
"wav\\thunder1.wav",
"wav\\Button.wav",
"wav\\New.wav",
"Wav\\CharacterDelete.wav",
"Wav\\ButtonBack.wav",
"Wav\\CharacterSelect.wav",

"Wav\\NewChar.wav",
"Wav\\Fire.wav",
"Wav\\Thunder.wav",
"Wav\\GameStart.wav",
"Wav\\Login.wav",
NULL
};

char *wnameBuf[NUM_SOUNDS]= {
"wav\\bgm0.wav",
"wav\\rain01.wav",
"wav\\thunder1.wav",
"wav\\step01.wav",
"Wav\\Effects\\weapon\\one hand swing - small 3.wav",
    NULL
};

int isPlaying(int i);

//=============== 배경뮤직 관련 함수 ========================//
char *backMusicWav[] = {   //임시로 넣은것임.
"wav\\622s.wav",	//0
NULL,
NULL,
};

/*///////////////////////////////////////////////////////////////////////////
: 함수  명 :  int  BackMusic_Load( int stageNumber )
: 하는  일 :  wav의 0번버퍼로 백wav를 미리 로드 시켜둠.
: 서    식 :  wavfile name이나 버퍼로부터 wav데이타를 로드하는 함수
: 인    자 :  주의 : 무조건 wav의 0번 버퍼로 로드하게되있음에 주의
: 반환  값 :  InitDX_wav2함수를 이용해 미리 셋팅후 시켜둔후 사용을 해야함.(winmain.cpp)
: 논    평 :
///////////////////////////////////////////////////////////////////////////*/
int  BackMusic_Load( int stageNumber ) //1번스테이지 부터시작함에 주의
{
	if(g_bSoundPresent == FALSE) return 0;

    char strFilename[128];
    strcpy( strFilename, dataDir);

    strcat( strFilename, backMusicWav[stageNumber] );

    if( !CreateBufferFromWaveFile(strFilename, 0) )
    {
        return FALSE;
    }
    return 1;
}

void BackMusicPlay()
{
    PlayWav(0, 1);
}

//메모리로 로드된 wav를 없앤다.
void BackMusicClr()
{
	StopAllWav();
    sBufferClear(0);
}

void PlayWavEffect(int num)
{
    PlayWav(num);
}

void BackMusic_Stop()
{
	StopWav(0);
}



//----------------------------------------------------------------------
// 
// Function     : InitDirectX()
//
// Purpose      : Initialises DirectX (DirectDraw, Direct3D, Direct3DRM, DirectSound)
//
//----------------------------------------------------------------------
BOOL InitDX_Wav(HWND g_hwnd)  //초기화
{
    if(!wav_opt) {
            g_bSoundPresent = FALSE;
            return 0;
    }

	int i;
    //FILE        *fp;
    //BYTE        pal[768];
    //HRESULT     rval;
    //DDSURFACEDESC ddsd;

    // Create DirectSound object
    //rval = DirectSoundCreate(NULL, &g_lpDS, NULL);	// pluto 비스타에서 채팅 때문에 옮겼다

    // Determine whether sound is present
    if(rval_g_lpDs == DS_OK)
        g_bSoundPresent = TRUE;
    else
        g_bSoundPresent = FALSE;
    //g_bSoundPresent = rval == DS_OK ? TRUE : FALSE;

    if (g_bSoundPresent)
    {
        // Set the DirectSound cooperative level
        //TRY_DS(g_lpDS->SetCooperativeLevel(g_hWnd, DSSCL_NORMAL))

        g_lpDS->SetCooperativeLevel(g_hwnd, DSSCL_NORMAL);

        // Null out all the sound pointers
        for (i = 0; i < NUM_SOUNDS; i ++)
        {
            g_lpSounds[i] = NULL;
        }

        for(i=0; i<NUM_SOUNDS; i++)
        {
            if( wnameBuf[i] != NULL) {
                if (!CreateBufferFromWaveFile(wnameBuf[i], i))
                    //RegError("Couldn't load INTRO.WAV!");
                    return FALSE;
                numSound++;
            }
        }
    }
    // Yahoo!
    return TRUE;
}


//----------------------------------------------------------------------
// 
// Function     : TermDX_Wav();
//
// Purpose      : Destroys all the DirectX sound objects
//
//----------------------------------------------------------------------
void TermDX_Wav()
{
    if (!g_bSoundPresent) return;

    // Destroy sounds
    for (int i = 0; i < NUM_SOUNDS; i ++)
    {
        if (g_lpSounds[i])
        {
            g_lpSounds[i]->Release();
            g_lpSounds[i] = NULL;
        }
    }

    // Destroy DirectSound object
    if (g_lpDS)
    {
        g_lpDS->Release();
        g_lpDS = NULL;
    }
}


//----------------------------------------------------------------------
//
// Function     : CreateSoundBuffer()
//
// Purpose      : Creates a DirectSound buffer
//
//----------------------------------------------------------------------
BOOL CreateSoundBuffer(DWORD dwBuf, DWORD dwBufSize, DWORD dwFreq, DWORD dwBitsPerSample, DWORD dwBlkAlign, BOOL bStereo)
{
    PCMWAVEFORMAT pcmwf;
    DSBUFFERDESC dsbdesc;
    
    // Set up wave format structure.
    memset( &pcmwf, 0, sizeof(PCMWAVEFORMAT) );
    pcmwf.wf.wFormatTag         = WAVE_FORMAT_PCM;      
    pcmwf.wf.nChannels          = bStereo ? 2 : 1;
    pcmwf.wf.nSamplesPerSec     = dwFreq;
    pcmwf.wf.nBlockAlign        = (WORD)dwBlkAlign;
    pcmwf.wf.nAvgBytesPerSec    = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
    pcmwf.wBitsPerSample        = (WORD)dwBitsPerSample;

    // Set up DSBUFFERDESC structure.
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));  // Zero it out. 
    dsbdesc.dwSize              = sizeof(DSBUFFERDESC);
    //dsbdesc.dwFlags             = DSBCAPS_CTRLDEFAULT;  // Need default controls (pan, volume, frequency).
    dsbdesc.dwFlags             = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
		//DSBCAPS_CTRLDEFAULT;  // Need default controls (pan, volume, frequency).
    dsbdesc.dwBufferBytes       = dwBufSize; 
    dsbdesc.lpwfxFormat         = (LPWAVEFORMATEX)&pcmwf;

    g_lpDS->CreateSoundBuffer(&dsbdesc, &g_lpSounds[dwBuf], NULL);
    return TRUE;
}

//----------------------------------------------------------------------
// 
// Function     : ReadData()
//
// Purpose      : Reads in data from a wave file
//
//----------------------------------------------------------------------
BOOL ReadData(LPDIRECTSOUNDBUFFER lpDSB, FILE* pFile, DWORD dwSize, DWORD dwPos)
{
    // Seek to correct position in file (if necessary)
    if (dwPos != 0xffffffff) 
    {
        if (fseek(pFile, dwPos, SEEK_SET) != 0) 
        {
            return FALSE;
        }
    }

    // Lock data in buffer for writing
    LPVOID pData1;
    DWORD  dwData1Size;
    LPVOID pData2;
    DWORD  dwData2Size;
    HRESULT rval;
    rval = lpDSB->Lock(0, dwSize, &pData1, &dwData1Size, &pData2, &dwData2Size, DSBLOCK_FROMWRITECURSOR);
    if (rval != DS_OK)
    {
        return FALSE;
    }

    // Read in first chunk of data
    if (dwData1Size > 0) 
    {
        if (fread(pData1, dwData1Size, 1, pFile) != 1) 
        {               
            return FALSE;
        }
		//ktj :마우스땜에 임시로 넣은것
		if(mouThRead ==0) {
			mouThdataOfs = (BYTE*)pData1;
		}

    }
    // read in second chunk if necessary
    if (dwData2Size > 0) 
    {
        if (fread(pData2, dwData2Size, 1, pFile) != 1) 
        {
            return FALSE;
        }
    }

    // Unlock data in buffer
    rval = lpDSB->Unlock(pData1, dwData1Size, pData2, dwData2Size);
    if (rval != DS_OK)
    {
        return FALSE;
    }

    // Yahoo!
    return TRUE;
}

//----------------------------------------------------------------------
// 
// Function     : CreateSoundBufferFromWaveFile()
//
// Purpose      : Creates a DirectSound buffer from a wave file
//
//----------------------------------------------------------------------

BOOL CreateBufferFromWaveFile(char* FileName, DWORD dwBuf)
{
    DWORD dwSize;
    BOOL bStereo;
	// Open the wave file       
    FILE* pFile = fopen(FileName,"rb");
    if (pFile == NULL) return FALSE;

    // Read in the wave header
    WaveHeader1 wavHdr1;    //DATA[4]
    WaveHeader2 wavHdr2;    //DATA[16]

    WaveHeader0 wavHdr;	//일단 DATA[18]로 읽는다.
    if (fread(&wavHdr, sizeof(wavHdr), 1, pFile) != 1) 
    {
        fclose(pFile);
        return NULL;
    }

	if(wavHdr.DATA[0]=='d' && wavHdr.DATA[1] == 'a') {

		fseek( pFile, 0, SEEK_SET);	//다시 처음부터 DATA[4]로 읽는다.
		fread(&wavHdr1, sizeof(wavHdr1), 1, pFile);
	    dwSize = wavHdr1.dwDSize;
	    // Is this a stereo or mono file?
		bStereo = wavHdr1.wChnls > 1 ? TRUE : FALSE;
		// Create the sound buffer for the wave file
		if (!CreateSoundBuffer(dwBuf, dwSize, wavHdr1.dwSRate, wavHdr1.BitsPerSample, wavHdr1.wBlkAlign, bStereo))
		{
			// Close the file
			fclose(pFile);
	        return FALSE;
		}
		if ( !g_lpSounds[dwBuf] ) {
			fclose(pFile);
			return FALSE;
		}
		//마우스를 읽어야하기땜에 임시로 넣은것
		mouThRead = dwBuf;
		// Read the data for the wave file into the sound buffer
		if (!ReadData(g_lpSounds[dwBuf], pFile, dwSize, sizeof(wavHdr1))) 
		{
	        fclose(pFile);
		    return FALSE;
		}
	}
	else if(wavHdr.DATA[0]=='f' && wavHdr.DATA[1] == 'a') {

		fseek( pFile, 0, SEEK_SET);	//다시 처음부터 DATA[16]로 읽는다.
		fread(&wavHdr2, sizeof(wavHdr2), 1, pFile);

	    dwSize = wavHdr2.dwDSize;
	    // Is this a stereo or mono file?
		bStereo = wavHdr2.wChnls > 1 ? TRUE : FALSE;

		// Create the sound buffer for the wave file
		if (!CreateSoundBuffer(dwBuf, dwSize, wavHdr2.dwSRate, wavHdr2.BitsPerSample, wavHdr2.wBlkAlign, bStereo))
		{
			// Close the file
			fclose(pFile);
	        return FALSE;
		}

		//마우스를 읽어야하기땜에 임시로 넣은것
		mouThRead = dwBuf;
		// Read the data for the wave file into the sound buffer
		if (!ReadData(g_lpSounds[dwBuf], pFile, dwSize, sizeof(wavHdr2))) 
		{
	        fclose(pFile);
		    return FALSE;
		}
	}
	else {

	    // Figure out the size of the data region
		dwSize = wavHdr.dwDSize;
		// Is this a stereo or mono file?
	    bStereo = wavHdr.wChnls > 1 ? TRUE : FALSE;

		// Create the sound buffer for the wave file
		if (!CreateSoundBuffer(dwBuf, dwSize, wavHdr.dwSRate, wavHdr.BitsPerSample, wavHdr.wBlkAlign, bStereo))
		{
			// Close the file
			fclose(pFile);
	        return FALSE;
		}

		//마우스를 읽어야하기땜에 임시로 넣은것
		mouThRead = dwBuf;
		// Read the data for the wave file into the sound buffer
		if (!ReadData(g_lpSounds[dwBuf], pFile, dwSize, sizeof(wavHdr))) 
		{
			fclose(pFile);
			return FALSE;
		}
	}

    // Close out the wave file
    fclose(pFile);
    // Yahoo!
    return TRUE;
}







//파일이 아닌 메모리에서 웨이브를 읽도록 한다.
//-----------------------------------------------------------------------------------//
BOOL ReadDataWavBuf(LPDIRECTSOUNDBUFFER lpDSB, BYTE *wavBUF, DWORD dwSize) 
{
	/*
    // Seek to correct position in file (if necessary)
    if (dwPos != 0xffffffff) 
    {
        if (fseek(pFile, dwPos, SEEK_SET) != 0) 
        {
            return FALSE;
        }
    }
	*/

	int cnt=0;
    // Lock data in buffer for writing
    LPVOID pData1;
    DWORD  dwData1Size;
    LPVOID pData2;
    DWORD  dwData2Size;
    HRESULT rval;

    rval = lpDSB->Lock(0, dwSize, &pData1, &dwData1Size, &pData2, &dwData2Size, DSBLOCK_FROMWRITECURSOR);
    if (rval != DS_OK)
    {
        return FALSE;
    }

    // Read in first chunk of data
    if (dwData1Size > 0) 
    {
        //if (fread(pData1, dwData1Size, 1, pFile) != 1) 
        //{               
        //    return FALSE;
        //}
		memcpy(pData1, &wavBUF[cnt], dwData1Size);
		cnt+=dwData1Size;
		//ktj :마우스땜에 임시로 넣은것
		if(mouThRead ==0) {
			mouThdataOfs = (BYTE*)pData1;
		}

    }

    // read in second chunk if necessary
    if (dwData2Size > 0) 
    {
        //if (fread(pData2, dwData2Size, 1, pFile) != 1) 
        //{
        //    return FALSE;
        //}
		memcpy(pData2, &wavBUF[cnt], dwData2Size);
    }

    // Unlock data in buffer
    rval = lpDSB->Unlock(pData1, dwData1Size, pData2, dwData2Size);
    if (rval != DS_OK)
    {
        return FALSE;
    }

    // Yahoo!
    return TRUE;
}

BOOL CreateBufferFromWavBUF(unsigned char* wavBUf, unsigned int dwBuf)
{
    DWORD dwSize;
    BOOL bStereo;
	BYTE cnt=0;

	// Open the wave file       
    //FILE* pFile = fopen(FileName,"rb");
    //if (pFile == NULL) return FALSE;

    // Read in the wave header
	WaveHeader1 wavHdr1;	//DATA[4]
	WaveHeader2 wavHdr2;	//DATA[16]

    WaveHeader0 wavHdr;	//일단 DATA[18]로 읽는다.
    //if (fread(&wavHdr, sizeof(wavHdr), 1, pFile) != 1) 
    //{
    //    fclose(pFile);
    //    return NULL;
    //}

	memcpy(&wavHdr, &wavBUf[cnt], sizeof(wavHdr) );
	cnt+=sizeof(wavHdr);


	if(wavHdr.DATA[0]=='d' && wavHdr.DATA[1] == 'a') {
		//fseek( pFile, 0, SEEK_SET);	//다시 처음부터 DATA[4]로 읽는다.
		//fread(&wavHdr1, sizeof(wavHdr1), 1, pFile);
		cnt=0;
		memcpy(&wavHdr1, &wavBUf[cnt], sizeof(wavHdr1) );
		cnt+=sizeof(wavHdr1);


	    dwSize = wavHdr1.dwDSize;
	    // Is this a stereo or mono file?
		bStereo = wavHdr1.wChnls > 1 ? TRUE : FALSE;

		//fd2("0000  size %d freq %d %d %d %d  ",dwSize, wavHdr1.dwSRate, wavHdr1.BitsPerSample, wavHdr1.wBlkAlign, bStereo);
		// Create the sound buffer for the wave file
		if (!CreateSoundBuffer(dwBuf, dwSize, wavHdr1.dwSRate, wavHdr1.BitsPerSample, wavHdr1.wBlkAlign, bStereo))
		{
			// Close the file
			//fclose(pFile);
	        return FALSE;
		}


		//마우스를 읽어야하기땜에 임시로 넣은것
		mouThRead = dwBuf;
		// Read the data for the wave file into the sound buffer
		//if (!ReadDataWavBuf(g_lpSounds[dwBuf], pFile, dwSize, sizeof(wavHdr1))) 
		if( !ReadDataWavBuf(g_lpSounds[dwBuf], &wavBUf[cnt],dwSize ) )
		{
	        //fclose(pFile);
		    return FALSE;
		}
	}

	else if(wavHdr.DATA[0]=='f' && wavHdr.DATA[1] == 'a') {
		//fseek( pFile, 0, SEEK_SET);	//다시 처음부터 DATA[16]로 읽는다.
		//fread(&wavHdr2, sizeof(wavHdr2), 1, pFile);
		cnt=0;
		memcpy(&wavHdr2, &wavBUf[cnt], sizeof(wavHdr2) );
		cnt+=sizeof(wavHdr2);


	    dwSize = wavHdr2.dwDSize;
	    // Is this a stereo or mono file?
		bStereo = wavHdr2.wChnls > 1 ? TRUE : FALSE;

		
//fd2("111  size %d freq %d %d %d %d  ",dwSize, wavHdr2.dwSRate, wavHdr2.BitsPerSample, wavHdr2.wBlkAlign, bStereo);		
		// Create the sound buffer for the wave file
		if (!CreateSoundBuffer(dwBuf, dwSize, wavHdr2.dwSRate, wavHdr2.BitsPerSample, wavHdr2.wBlkAlign, bStereo))
		{
			// Close the file
			//fclose(pFile);
	        return FALSE;
		}

		//마우스를 읽어야하기땜에 임시로 넣은것
		mouThRead = dwBuf;
		// Read the data for the wave file into the sound buffer
		//if (!ReadData(g_lpSounds[dwBuf], pFile, dwSize, sizeof(wavHdr2))) 
		if( !ReadDataWavBuf(g_lpSounds[dwBuf], &wavBUf[cnt], dwSize ) )
		{
	        //fclose(pFile);
		    return FALSE;
		}
	}
	else {
	    // Figure out the size of the data region
		dwSize = wavHdr.dwDSize;
		// Is this a stereo or mono file?
	    bStereo = wavHdr.wChnls > 1 ? TRUE : FALSE;

//fd2("2222 size %d freq %d %d %d %d  ",dwSize, wavHdr.dwSRate, wavHdr.BitsPerSample, wavHdr.wBlkAlign, bStereo);

		// Create the sound buffer for the wave file
		if (!CreateSoundBuffer(dwBuf, dwSize, wavHdr.dwSRate, wavHdr.BitsPerSample, wavHdr.wBlkAlign, bStereo))
		{
			// Close the file
			//fclose(pFile);
	        return FALSE;
		}

		//마우스를 읽어야하기땜에 임시로 넣은것
		mouThRead = dwBuf;
		// Read the data for the wave file into the sound buffer
		//if (!ReadData(g_lpSounds[dwBuf], pFile, dwSize, sizeof(wavHdr))) 
		if( !ReadDataWavBuf(g_lpSounds[dwBuf], &wavBUf[cnt], dwSize ) )
		{
			//fclose(pFile);
			return FALSE;
		}
	}
    // Close out the wave file
    //fclose(pFile);

    // Yahoo!
    return TRUE;
}
//-----------------------------------------------------------------------------------//








//----------------------------------------------------------------------
// 
// Function     : StopAllSounds()
//
// Purpose      : Stops all sounds
//
//----------------------------------------------------------------------

BOOL StopAllWav()
{
	if (!g_bSoundPresent) return FALSE;

    // Make sure we have a valid sound buffer
    for (int i = 0; i < NUM_SOUNDS; i ++)
    {
        if (g_lpSounds[i])
        {
            DWORD dwStatus;
            //TRY_DS(g_lpSounds[i]->GetStatus(&dwStatus));
			g_lpSounds[i]->GetStatus(&dwStatus);

            if ((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
            {
                // Play the sound
                //TRY_DS(g_lpSounds[i]->Stop())
				g_lpSounds[i]->Stop();

            }
        }
    }

    // Yahoo!
    return TRUE;
}





#define VOL_SHIFT (-400)
#define VOL_MULT 10

#define PAN_SHIFT (-400)
#define PAN_MULT  10

#define FRQ_MULT  10
//----------------------------------------------------------------------
// 
// Function     : PlaySoundDS()
//                dwSound : wav의 번호,  dwFlags : 0-한번만소리내기,1-루프를 돌아 계속소리내기
//				  dwVol (0~400) (실제로는 -400~0이고 *10한값)
//				  dwfreq : 441~4410(실제로는 4410~44100HZ까지)
// Purpose      : Plays a sound using direct sound
//
//----------------------------------------------------------------------
BOOL PlayWav(DWORD dwSound, DWORD dwFlags, DWORD dwVol, DWORD dwfreq)
{
    if (g_bSoundPaused) return TRUE;
    if (!g_bSoundPresent) return TRUE;

    // Make sure the sound is valid
    if (dwSound >= NUM_SOUNDS) return FALSE; 

	//중간로드
	chk_load_sangJuWav( (int) dwSound) ;


    // Make sure we have a valid sound buffer
    if (g_lpSounds[dwSound])
    {
        DWORD dwStatus;
        g_lpSounds[dwSound]->GetStatus(&dwStatus);
        if ((dwStatus & DSBSTATUS_PLAYING) != DSBSTATUS_PLAYING)
        {
            // Play the sound
            g_lpSounds[dwSound]->SetVolume( (dwVol+VOL_SHIFT) * VOL_MULT);
			g_lpSounds[dwSound]->SetFrequency(dwfreq*FRQ_MULT);
			g_lpSounds[dwSound]->Play(0, 0, dwFlags);

        }
	
		//백뮤직은 처음부터
		if(dwSound==0)
			g_lpSounds[dwSound]->SetCurrentPosition(0); 

    }

    // Yahoo!
    return TRUE;
}


//소리가 나고 있는 도중이라도 계속 소리냄.
BOOL PlayWav2(DWORD dwSound, DWORD dwFlags, DWORD dwVol, DWORD dwfreq)
{
    if (g_bSoundPaused) return TRUE;
    if (!g_bSoundPresent) return TRUE;

    // Make sure the sound is valid
    if (dwSound >= NUM_SOUNDS) return FALSE;

	//중간로드
	chk_load_sangJuWav( (int) dwSound) ;


    // Make sure we have a valid sound buffer
    if (g_lpSounds[dwSound])
    {
		
		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);
		if ((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)	//일단 멈춤
			//g_lpSounds[dwSound]->Stop();
			g_lpSounds[dwSound]->SetCurrentPosition(0);

		g_lpSounds[dwSound]->SetVolume( (dwVol+VOL_SHIFT) * VOL_MULT); 
		g_lpSounds[dwSound]->SetFrequency(dwfreq*FRQ_MULT);
		g_lpSounds[dwSound]->Play(0, 0, dwFlags);
    }

    return TRUE;
}

//소리가 연주중인지 확인
BOOL IsPlayWav( DWORD dwSound )
{
	DWORD dwStatus;

    if (g_bSoundPaused) return FALSE;
    if (!g_bSoundPresent) return FALSE;
    if (dwSound >= NUM_SOUNDS) return FALSE;
	if ( !g_lpSounds[dwSound] ) return FALSE;

	g_lpSounds[dwSound]->GetStatus(&dwStatus);

	if ((dwStatus & DSBSTATUS_PLAYING) != DSBSTATUS_PLAYING) return FALSE;

	return TRUE;
}


//소리가 나고 있는 도중이라도 계속 소리냄.
//dwfreq : 441~4410
//dwPan : 0~800(0:왼쪽 스피커, 400:중간 800:오른쪽 스피커쪽으로 소리가 더난다.)
BOOL PlayWav3(DWORD dwSound, DWORD dwFlags, DWORD dwVol, DWORD dwfreq, DWORD dwPan)
{
    if (g_bSoundPaused) return TRUE;
    if (!g_bSoundPresent) return TRUE;

    // Make sure the sound is valid
    if (dwSound >= NUM_SOUNDS) return FALSE;

	//중간로드
	chk_load_sangJuWav( (int) dwSound) ;

    // Make sure we have a valid sound buffer
    if (g_lpSounds[dwSound])
    {
		
		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);
		if ((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)	//일단 멈춤
			//g_lpSounds[dwSound]->Stop();
			g_lpSounds[dwSound]->SetCurrentPosition(0);

		//0~800(0:왼쪽 스피커, 400:중간 800:오른쪽 스피커)
		g_lpSounds[dwSound]->SetPan( (dwPan+PAN_SHIFT)*PAN_MULT);//4410);

		//441(*10) ~4410(*10)
		g_lpSounds[dwSound]->SetFrequency(dwfreq*FRQ_MULT);

		//0~400
		g_lpSounds[dwSound]->SetVolume( (dwVol+VOL_SHIFT) * VOL_MULT); 
		g_lpSounds[dwSound]->Play(0, 0, dwFlags);
    }

    return TRUE;
}



//플레이도중에 volume change
BOOL PlayWav_VolChange(DWORD dwSound, DWORD dwVol)
{
    if (g_bSoundPaused) return TRUE;
    if (!g_bSoundPresent) return TRUE;

    // Make sure the sound is valid
    if (dwSound >= NUM_SOUNDS) return FALSE;

	//중간로드
	chk_load_sangJuWav( (int) dwSound) ;

    // Make sure we have a valid sound buffer
    if (g_lpSounds[dwSound])
    {
		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);
		if ((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING) {
			//0~400
			g_lpSounds[dwSound]->SetVolume( (dwVol+VOL_SHIFT) * VOL_MULT); 
		}

    }
    return TRUE;
}

//플레이도중에 Frequency Change
BOOL PlayWav_FrequencyChange(DWORD dwSound, DWORD dwfreq)
{
    if (g_bSoundPaused) return TRUE;
    if (!g_bSoundPresent) return TRUE;

    // Make sure the sound is valid
    if (dwSound >= NUM_SOUNDS) return FALSE;

    // Make sure we have a valid sound buffer
    if (g_lpSounds[dwSound])
    {
		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);
		if ((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
			//441(*10) ~4410(*10)
			g_lpSounds[dwSound]->SetFrequency(dwfreq*FRQ_MULT);
    }
    return TRUE;
}


//플레이도중에 Panning Change
BOOL PlayWav_PanChange(DWORD dwSound, DWORD dwPan)
{
    if (g_bSoundPaused) return TRUE;
    if (!g_bSoundPresent) return TRUE;

    // Make sure the sound is valid
    if (dwSound >= NUM_SOUNDS) return FALSE;

    // Make sure we have a valid sound buffer
    if (g_lpSounds[dwSound])
    {
		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);
		if ((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
			//0~800(0:왼쪽 스피커, 400:중간 800:오른쪽 스피커)
			g_lpSounds[dwSound]->SetPan( (dwPan+PAN_SHIFT)*PAN_MULT);//4410);
    }
    return TRUE;
}



BOOL StopWav(int i);

//ktj
void check_backMus(int flag)
{
	if(flag==0) {
		StopWav(0);
	}
	else {
		StopWav(0);
		PlayWav(0, 1);
	}
}

/*
//아래 웨이브는 g_lpSounds[..]의 1번부터들어가므로 번호가 1번부터 시작됨에 주의
char *sangJuWav[NUM_SOUNDS] = {      //상주 wav를 뽑는다.
"wav\\Ready.wav",              //게임모드 ready
"wav\\go.wav",                 //게임모드 gameing
"wav\\hurry.wav",              //특정시간안에 안눌르면 초당한번씩 나온다.
"wav\\공나가기.WAV",               //던질때 사용


"wav\\게임끝.wav",			//game끝났을때.
"wav\\자리확인.WAV",               //붙을때 사용
"wav\\알터질때.WAV",               //우수수떨어질때
"wav\\공튀기기.WAV",               //공튀길때

"wav\\포인트.WAV",               //점수가 양수일때

NULL,
NULL,
};
*/
/*
char *sangJuWav[NUM_SOUNDS] = {     //상주 wav를 뽑는다.
"wav\\start1.wav",					//게임모드 시작
"wav\\clock.wav",					//특정시간안에 안눌르면 초당한번씩 나온다.
"wav\\bf.wav",					//던질때 사용

"wav\\bplace.wav",              //붙을때 사용
"wav\\bd.wav",					//우수수떨어질때
"wav\\wall_br.wav",             //벽에 부딫혀 공튀길때


"wav\\win_hc.wav",             //점수가 양수일때(승리박수)
"wav\\win1.wav",

"wav\\loose1.wav",				//2등이하부터는 진것임.
"wav\\loose2.wav",
"wav\\loose3.wav",
"wav\\loose4.wav",

"wav\\e_bu1.wav",						//적의공 올라감.
"wav\\e_bu2.wav",

"wav\\bt_yes.wav",				//공떨어뜨릴때 많이 떨어지면 나옴.
"wav\\bt_wow.wav",


"wav\\ldown.wav",			//한줄내려옴.

"wav\\aim.wav",			//조준기


"wav\\win_m.wav",			//이겼을때 음악
"wav\\lose_m.wav",			//졌을때 음악.


NULL,
NULL,
};

*/












//배경웨이브를 위해 0번은 항상  비워두고 1번 이하부터 상주용 웨이브를 읽는다.
BOOL InitDX_Wav2(HWND g_hwnd)  //초기화
{
    char strFilename[128];
    if(!wav_opt) {
            g_bSoundPresent = FALSE;
            return 0;
    }

    int i;
    HRESULT     rval;
	


    // Create DirectSound object
    rval = DirectSoundCreate(NULL, &g_lpDS, NULL);

    // Determine whether sound is present
    if(rval == DS_OK)
            g_bSoundPresent = TRUE;
    else
            g_bSoundPresent = FALSE;
    
    /*
    //상주 wav를 로드하기위해 바꾼다.
    if (g_bSoundPresent)
    {
        // Set the DirectSound cooperative level
        // Null out all the sound pointers
        for (i = 0; i < NUM_SOUNDS; i ++) {
            g_lpSounds[i] = NULL;
        }
    }
    */

    if (g_bSoundPresent)
    {
        // Set the DirectSound cooperative level
        g_lpDS->SetCooperativeLevel(g_hwnd, DSSCL_NORMAL);
        // Null out all the sound pointers
        for (i = 0; i < NUM_SOUNDS; i ++) {
            g_lpSounds[i] = NULL;
        }

		
        for(i=1; i<NUM_SOUNDS; i++)
        {
            if( sangJuWav[i-1] != NULL && sangJuWav[i-1][0] ) {
				if(  sangJuWav_F[i-1 ] == 0 ) 
					continue;

                strcpy ( strFilename , dataDir);
                strcat ( strFilename , sangJuWav[i-1] );

                if (!CreateBufferFromWaveFile(strFilename, i))
                {
					
					//fd2( " wav %s break ", strFilename );
                        //RegError("Couldn't load INTRO.WAV!");
                        //return FALSE;
                }
                //numSound++;
            }
        }
    }
    // Yahoo!






	Init_BGM(g_hwnd);		//배경wav이용하기
    return TRUE;
}





void TermDX_Wav2()
{
	if (!g_bSoundPresent) return;
    
    for (int i = 0; i < NUM_SOUNDS; i ++) {		// Destroy sounds
        if (g_lpSounds[i]) {
            PlayWav_VolChange(i, 0);
            g_lpSounds[i]->Release();	g_lpSounds[i] = NULL;
        }
    }
    if (g_lpDS) {								// Destroy DirectSound object
        g_lpDS->Release();
        g_lpDS = NULL;
    }

	Uninit_BGM();		//배경wav이용하기
}


BOOL CreateBufferFromWaveFile2(char* FileName, int sbufN)
{
	if (!g_bSoundPresent) {
		return FALSE;
	}
        StopWav(sbufN); //혹시 소리가 나오는 도중이면 없앰.
	sBufferClear(sbufN);

	if (!CreateBufferFromWaveFile(FileName, sbufN))	{
                sBufferClear(sbufN);
                return FALSE;
        }
        return TRUE;
}


BOOL sBufferClear(int sbufN)
{
        if (!g_bSoundPresent) {
            return FALSE;
        }

	if (g_lpSounds[sbufN]) {
                g_lpSounds[sbufN]->Release();
		g_lpSounds[sbufN] = NULL;
		return TRUE;
        }
        return FALSE;
}


BOOL StopWav(int i)
{
        if (!g_bSoundPresent) {
            return FALSE;
        }

    // Make sure we have a valid sound buffer
     if (g_lpSounds[i])
	{
            DWORD dwStatus;
            //TRY_DS(g_lpSounds[i]->GetStatus(&dwStatus));
			g_lpSounds[i]->GetStatus(&dwStatus);

            if ((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
            {
                // Play the sound
                //TRY_DS(g_lpSounds[i]->Stop())
                g_lpSounds[i]->Stop();
            }
    }
    // Yahoo!
    return TRUE;
}


//ktj
void allWAVE_playORstop(int flag)
{
	int i;
	if(flag==0) {
		for(i=0; i<NUM_SOUNDS; i++) {		//stop wave
			if (g_lpSounds[i]) {
				StopWav(i);
			}
		}
	}
        else {

                if (g_lpSounds[0]) {    //back music으로 쓰이는 것만 소리낸다.
                                StopWav(0);
                                PlayWav(0);
                }
        }
}





//-------------------------- ktj
// 현재플레이중의 포지션, 쓰기포지션찾기 함수
			//read position, write position
//GetCurrentPosition(&position, NULL); 
//현재 프리콴씨를 갖는다.
//HRESULT GetFrequency(  LPDWORD lpdwFrequency  );
//---------------------------------------
DWORD get_pos(DWORD dwSound)
{
	DWORD pos;
    if (g_bSoundPaused) return TRUE;
    if (!g_bSoundPresent) return TRUE;

    // Make sure the sound is valid
    if (dwSound >= NUM_SOUNDS) return FALSE;

    // Make sure we have a valid sound buffer
    if (g_lpSounds[dwSound])
    {
		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);
		
		if ((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING) {
			//441(*10) ~4410(*10)
			g_lpSounds[dwSound]->GetCurrentPosition(&pos, NULL); 

			

			return pos;
		}
    }

    
	return 0;
}




//----------------------- 0번은 발음하는 버퍼이므로 그것의 위치를 알아내
//입을 움직일지 여부를 알아내기 위한 함수
//일단 데이타를 읽은 버퍼포지션을 가지고있음.
BYTE getmouTH()
{
	if(mouThdataOfs ==NULL) return 0;

	DWORD pos = get_pos(0);				//마우스데이타는 0번임.
	BYTE b = mouThdataOfs[pos];
	return b;
}

int isPlaying(int i)
{
	if (!g_bSoundPresent) return FALSE;

    // Make sure we have a valid sound buffer
    if (g_lpSounds[i])   
	{
            DWORD dwStatus;
            //TRY_DS(g_lpSounds[i]->GetStatus(&dwStatus));
			g_lpSounds[i]->GetStatus(&dwStatus);

            if ((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
            {
                // Play the sound
                //TRY_DS(g_lpSounds[i]->Stop())
				return 1;
            }
			else  {
				return 0;
			}
    }
    // Yahoo!
    return 0;
}







































































//=======================================================================start
//sound 배경 wav를 타이머로 읽어서 연주
//=======================================================================start

#include "wave.h"
#include "shell.h"
#include "stream.h"


/*
BOOL Init_BGM(HWND hwnd);		//init
void Uninit_BGM(void);			//uninit
BOOL OpenBGM(LPSTR lpszFileName);	//bgm 운영
void PlayBGM(void);
void StopBGM(void);
*/

HRESULT SetupStreamBuffer(LPSTR lpszFilename);


LPDIRECTSOUND       lpDirectSound = NULL;

LPDIRECTSOUNDBUFFER lpDSBStreamBuffer = NULL;


HWND    m_hWnd;

LONG  m_lInTimer        = FALSE;
BOOL  m_bTimerInit      = FALSE;
BOOL  m_bPlaying        = FALSE;
BOOL  m_bIsCloseOnRun   = FALSE;
BOOL  m_bFileOpen       = FALSE;
UINT  m_uTimerID        = 0;
DWORD m_SecondBuffFocus = 0;

WAVEINFOCA  m_wiWave;



void LSStopBackground(DWORD dwState);
void LSResetWavePlayer(void);
void LSFillDataBuffer(void);
void CALLBACK LSTimeRunFuncForDSound(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);	


//=======================================================================
//       배경음악을 멈춘다
//       Input.
//           DWORD dwState - 플래그
//-----------------------------------------------------------------------
void LSStopBackground(DWORD dwState)
{
    m_wiWave.bDonePlaying = TRUE;
    if(m_bTimerInit){
        timeKillEvent(m_uTimerID);
        timeEndPeriod(PLAYBACK_TIMER_PERIOD / PLAYBACK_OVERSAMPLE);
        // Busy wait for timer func to exit
        while(InterlockedExchange(&m_lInTimer, TRUE)) Sleep(100);
        m_bTimerInit = FALSE;
    }
    if(m_bPlaying){
        m_bPlaying = FALSE;
        lpDSBStreamBuffer->Stop();
    }
    // Short circuit to allow command-line forced shutdown
    if(!(dwState & DSSTREAM_STOPF_NOREOPEN) && !m_bIsCloseOnRun){
        LSResetWavePlayer();
        return;
    }else{
        if(m_bFileOpen){
             WaveCloseReadFile(&m_wiWave.hmmio, &m_wiWave.pwfx);
             if(lpDSBStreamBuffer){
                 lpDSBStreamBuffer->Release();
             }

             lpDSBStreamBuffer = NULL;

             m_bFileOpen = FALSE;
         }
    }
}

//=======================================================================
//       웨이브 출력을 초기화 한다.
//-----------------------------------------------------------------------
void LSResetWavePlayer(void)
{
    WaveStartDataRead(&m_wiWave.hmmio, &m_wiWave.mmck, &m_wiWave.mmckInRIFF);
    m_wiWave.bFoundEnd        = FALSE;
    m_wiWave.dwBytesRemaining = 0;

    LSFillDataBuffer();

    m_wiWave.bDonePlaying     = FALSE;
}

//=======================================================================
//       다이렉트 사운드 버퍼를 채운다.
//-----------------------------------------------------------------------
void LSFillDataBuffer(void)
{
    LPBYTE   lpWrite1, lpWrite2;
    DWORD    dwLen1, dwLen2;
    UINT     uActualBytesWritten;
    HRESULT  dsrval;
    dsrval = lpDSBStreamBuffer->Lock(0, m_wiWave.dwBufferSize, 
		//&((LPVOID)lpWrite1), 
		(LPVOID*) &lpWrite1, 
		&dwLen1,
        //&((LPVOID)lpWrite2), 
		(LPVOID *) &lpWrite2, 
		
		&dwLen2, 0);
    if(dsrval != DS_OK) return;
//    Assert( NULL == lpWrite2 );
//    Assert( 0 == dwLen2 );
    if(dwLen1){
//        Assert( NULL != lpWrite1 );
        WaveReadFile(m_wiWave.hmmio, (UINT)dwLen1, lpWrite1, &m_wiWave.mmck, &uActualBytesWritten);
        if(uActualBytesWritten < dwLen1){
            if(m_wiWave.bLoopFile){
                // If the file is shorter than the buffer and we're looping, we need to
                // read the file in again so that we don't get a block of silence before
                // the timer loops playback.
                LPBYTE lpTemp          = lpWrite1;
                DWORD  cbReadLoopTotal = dwLen1;
                do{
                    // Continue decrementing our count and moving our temp
                    // pointer forward until we've read the file enough times
                    // to fill the buffer.  NOTE: It's probably not efficient
                    // to bother with the overhead of streaming a file that's
                    // not at least as large as the buffer...
                    lpTemp          += uActualBytesWritten;
                    cbReadLoopTotal -= uActualBytesWritten;
                    WaveStartDataRead(&m_wiWave.hmmio, &m_wiWave.mmck, &m_wiWave.mmckInRIFF);
                    WaveReadFile(m_wiWave.hmmio, (UINT)cbReadLoopTotal, lpTemp, &m_wiWave.mmck, &uActualBytesWritten);
                }while(uActualBytesWritten < cbReadLoopTotal);
            }else{
                m_wiWave.bFoundEnd        = TRUE;
                m_wiWave.dwBytesRemaining = (DWORD)uActualBytesWritten;
                // Fill in silence
                FillMemory(lpWrite1+uActualBytesWritten, dwLen1-uActualBytesWritten,
                    (BYTE)(m_wiWave.pwfx->wBitsPerSample == 8 ? 128 : 0));
            }
        }
    }
    dsrval = lpDSBStreamBuffer->Unlock((LPVOID)lpWrite1, dwLen1, (LPVOID)lpWrite2, 0);
    m_wiWave.dwNextWriteOffset = m_wiWave.dwProgress = 0;
}


//=======================================================================
//       타이머 이벤트 핸들러
//-----------------------------------------------------------------------
//void fd2( char *fmt, ...);
void CALLBACK LSTimeRunFuncForDSound(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
    LPBYTE  lpWrite1, lpWrite2, lpTemp;
    DWORD   dwLen1, dwLen2, dwPlay, dwWrite, dwPlayedLength, dwWriteLength;
    DWORD   dwLeftToRead, dwStatus;
    UINT    uChkErr;
    BOOL    fRefillLostBuffer = FALSE;
    HRESULT dsrval;

	//static cnt=0;
	//fd2( "timer : %d ", cnt++);

    if(InterlockedExchange(&m_lInTimer, TRUE)) return;

    // See if the buffer has been lost
    m_wiWave.lpDSBStreamBuffer->GetStatus(&dwStatus);
    if(DSBSTATUS_BUFFERLOST & dwStatus){
        // Restore the buffer and set some variables that will cause it
        // to be filled again and replayed
        dsrval = m_wiWave.lpDSBStreamBuffer->Restore();
        if(FAILED(dsrval)){
            InterlockedExchange(&m_lInTimer, FALSE);
            return;
        }
        m_wiWave.dwNextWriteOffset = 0;
        fRefillLostBuffer        = TRUE;
    }

    // Get and print the current position of the play cursor
    m_wiWave.lpDSBStreamBuffer->GetCurrentPosition(&dwPlay, &dwWrite);

    // If the play cursor is at the same spot as the last call, there are two
    // possibilities.  The first is that we were called extremely late and
    // happened to land on an integer number of complete buffer cycles.  This
    // is not very likely.  The other is that the play cursor didn't move.
    // Since we're oversampling, this is very likely.  In this case, we should
    // bail.
    if(dwPlay == m_wiWave.dwNextWriteOffset && !fRefillLostBuffer){
        InterlockedExchange(&m_lInTimer, FALSE);
        return;
    }
    // Have we found the end of the file and passed the buffer end?
    if(m_wiWave.bFoundEnd && !m_wiWave.dwBytesRemaining){
        if(!m_wiWave.bDonePlaying){
            m_wiWave.bDonePlaying = TRUE;
			LSStopBackground(0L);
        }
        InterlockedExchange(&m_lInTimer, FALSE);
        return;
    }

    if(dwPlay < m_wiWave.dwNextWriteOffset){
        // Calculate how many writeable bytes there are behind the play cursor
        dwPlayedLength = (dwPlay + m_wiWave.dwBufferSize - m_wiWave.dwNextWriteOffset);
    }else{
        // Calculate how many writeable bytes there are behind the play cursor
        dwPlayedLength = (dwPlay - m_wiWave.dwNextWriteOffset);
    }

    // If the buffer was lost, then we need to start filling data at the start of
    // the buffer, but we can decrease startup latency by only filling a segment
    // or two this time around.
    if(fRefillLostBuffer){
        dwWriteLength = 2 * m_wiWave.dwBufferSize / NUM_BUFFER_SEGMENTS;
    }else{
        dwWriteLength = dwPlayedLength;
    }

    m_wiWave.dwProgress += dwPlayedLength;

    // If m_wiWave.bFoundEnd == TRUE, then we've finished reading in the file,
    // but we need to wait until the buffer's play cursor passes the point we
    // were at when we found out we were done reading.
    if(m_wiWave.bFoundEnd && m_wiWave.dwBytesRemaining){
        // Decrement the count of how many bytes we have to wait for before
        // we can kill the timer procedure safely
        if(dwPlayedLength > m_wiWave.dwBytesRemaining){
            m_wiWave.dwBytesRemaining = 0;
        }else{
            m_wiWave.dwBytesRemaining -= dwPlayedLength;
        }
        if(m_wiWave.lpDSBStreamBuffer->Lock(m_wiWave.dwNextWriteOffset, dwWriteLength,
                                         //&((LPVOID)lpWrite1), 
										 (LPVOID *)&lpWrite1, 
										 &dwLen1,
                                         //&((LPVOID)lpWrite2), 
										 (LPVOID *) &lpWrite2, 
										 
										 &dwLen2, 0) != 0){
            OutputDebugString("LSTimeRunFuncForDSound() could not lock DirectSoundBuffer");
            InterlockedExchange(&m_lInTimer, FALSE);
            return;
        }

        // Silence out both parts of the locked buffer
        memset(lpWrite1, m_wiWave.pwfx->wBitsPerSample == 8 ? 128 : 0, dwLen1);

        if(lpWrite2 && dwLen2){
            memset(lpWrite2, m_wiWave.pwfx->wBitsPerSample == 8 ? 128 : 0, dwLen2);
        }
        m_wiWave.lpDSBStreamBuffer->Unlock((LPVOID)lpWrite1, dwLen1, (LPVOID)lpWrite2, dwLen2);
        // This code is stolen from the end of the routine -- we need to keep
        // zeroing out buffer segments while we're waiting for the play cursor to
        // catch up to the end of the WAVE data.
        m_wiWave.dwNextWriteOffset += dwWriteLength;
        if(m_wiWave.dwNextWriteOffset >= m_wiWave.dwBufferSize){
            m_wiWave.dwNextWriteOffset -= m_wiWave.dwBufferSize;
        }
        InterlockedExchange(&m_lInTimer, FALSE);
        return;
    }

    // Lock a segment of memory that is behind the play cursor
    if(m_wiWave.lpDSBStreamBuffer->Lock(m_wiWave.dwNextWriteOffset, dwWriteLength,
                                      //&((LPVOID)lpWrite1), 
									  (LPVOID *) &lpWrite1, 
									  &dwLen1,
                                      //&((LPVOID)lpWrite2), 
									  (LPVOID *) &lpWrite2, 
									  &dwLen2, 0) != 0){
        OutputDebugString("LSTimeRunFuncForDSound() could not lock DirectSoundBuffer");
        InterlockedExchange(&m_lInTimer, FALSE);
        return;
    }

    if(dwLen1 && !m_wiWave.bDonePlaying){
        WaveReadFile(m_wiWave.hmmio, (UINT)dwLen1, lpWrite1, &m_wiWave.mmck, &uChkErr);
        if(uChkErr < (UINT)dwLen1){
            if(!m_wiWave.bLoopFile){
                // Zero out the rest of this block
                if(m_wiWave.pwfx->wBitsPerSample == 8){
                    memset(lpWrite1+uChkErr, 128, ((UINT)dwLen1-uChkErr));
                }else if(m_wiWave.pwfx->wBitsPerSample == 16){
                    memset(lpWrite1+uChkErr, 0, ((UINT)dwLen1-uChkErr));
                }

                // Enable play completion detection code at the beginning of the next call
                m_wiWave.bFoundEnd = TRUE;
                if(dwPlay > m_wiWave.dwNextWriteOffset){
                    m_wiWave.dwBytesRemaining = (m_wiWave.dwNextWriteOffset + m_wiWave.dwBufferSize - dwPlay);
                }else{
                    m_wiWave.dwBytesRemaining = (m_wiWave.dwNextWriteOffset - dwPlay);
                }
            }else{
                lpTemp       = lpWrite1;
                dwLeftToRead = dwLen1;
                do{
                    // Continue decrementing our count and moving our temp
                    // pointer forward until we've read the file enough times
                    // to fill the buffer.  NOTE: It's probably not efficient
                    // to bother with the overhead of streaming a file that's
                    // not at least as large as the buffer...
                    lpTemp       += uChkErr;
                    dwLeftToRead -= uChkErr;
                    WaveStartDataRead(&m_wiWave.hmmio, &m_wiWave.mmck, &m_wiWave.mmckInRIFF);
                    WaveReadFile(m_wiWave.hmmio, (UINT)dwLeftToRead, lpTemp, &m_wiWave.mmck, &uChkErr);
                }while(uChkErr < dwLeftToRead);
            }
        }
    }
    // The bDonePlaying flag is set by the caller if the user stops playback
    // before the end of the WAVE file is encountered.  It tells us to cut this
    // racket out and play nothing in case it takes the caller a couple
    // interrupts to shut off the timer.
    else if(dwLen1 && m_wiWave.bDonePlaying){
        // Set the appropriate silence value
        _fmemset(lpWrite1, m_wiWave.pwfx->wBitsPerSample == 8 ? 128 : 0, dwLen1);
    }

    if(dwLen2 && !m_wiWave.bDonePlaying){
        WaveReadFile(m_wiWave.hmmio, (UINT)dwLen2, lpWrite2, &m_wiWave.mmck, &uChkErr);
        if(uChkErr < (UINT)dwLen2){
            if(!m_wiWave.bLoopFile){
                // Zero out the rest of this block
                if(m_wiWave.pwfx->wBitsPerSample == 8){
                    memset(lpWrite2+uChkErr, 128, ((UINT)dwLen2-uChkErr));
                }else if(m_wiWave.pwfx->wBitsPerSample == 16){
                    memset(lpWrite2+uChkErr, 0, ((UINT)dwLen2-uChkErr));
                }
                // Enable play completion detection code at the beginning
                // of the next call
                m_wiWave.bFoundEnd = TRUE;
                if(dwPlay > m_wiWave.dwNextWriteOffset){
                    m_wiWave.dwBytesRemaining = (m_wiWave.dwNextWriteOffset + m_wiWave.dwBufferSize - dwPlay);
                }else{
                    m_wiWave.dwBytesRemaining = (m_wiWave.dwNextWriteOffset - dwPlay);
                }
            }else{
                lpTemp       = lpWrite2;
                dwLeftToRead = dwLen2;
                do{
                    // Continue decrementing our count and moving our temp
                    // pointer forward until we've read the file enough times
                    // to fill the buffer.  NOTE: It's probably not efficient
                    // to bother with the overhead of streaming a file that's
                    // not at least as large as the buffer...
                    lpTemp += uChkErr;
                    dwLeftToRead -= uChkErr;
                    WaveStartDataRead(&m_wiWave.hmmio, &m_wiWave.mmck, &m_wiWave.mmckInRIFF);
                    WaveReadFile(m_wiWave.hmmio, (UINT)dwLeftToRead, lpTemp, &m_wiWave.mmck, &uChkErr);
                }while(uChkErr < dwLeftToRead);
            }
        }
    }else if(lpWrite2 && dwLen2 && m_wiWave.bDonePlaying){
        // Set the appropriate silence value
        _fmemset(lpWrite2, m_wiWave.pwfx->wBitsPerSample == 8 ? 128 : 0, dwLen2);
    }
    m_wiWave.lpDSBStreamBuffer->Unlock((LPVOID)lpWrite1, dwLen1, (LPVOID)lpWrite2, dwLen2);
    m_wiWave.dwNextWriteOffset += dwWriteLength;
    if(m_wiWave.dwNextWriteOffset >= m_wiWave.dwBufferSize){
        m_wiWave.dwNextWriteOffset -= m_wiWave.dwBufferSize;
    }
    if(fRefillLostBuffer){
        m_wiWave.lpDSBStreamBuffer->Play(0, 0, DSBPLAY_LOOPING);
    }
    InterlockedExchange(&m_lInTimer, FALSE);

    return;
}









BOOL OpenBGM(LPSTR lpszFileName)
{
	if(LSEnable == TRUE){
        if(m_bFileOpen){
            m_bIsCloseOnRun = FALSE;
            // 파일을 오픈하기 전에 연주중이라면 멈춰야 한다.
            LSStopBackground(DSSTREAM_STOPF_NOREOPEN);
        }
        if((SetupStreamBuffer(lpszFileName)) != DS_OK){
            return FALSE;
        }else{
            m_bFileOpen = TRUE;
        }
    }
	return TRUE;
}

HRESULT SetupStreamBuffer(LPSTR lpszFilename)
{
    DSBUFFERDESC dsbd;
    HRESULT      dsRetVal;

    // This portion of the WAVE I/O is patterned after what's in DSTRWAVE, which
    // was in turn adopted from WAVE.C which is part of the DSSHOW sample.
    if(WaveOpenFile(lpszFilename, &m_wiWave.hmmio, &m_wiWave.pwfx, &m_wiWave.mmckInRIFF) != 0){
        return ERR_WAVE_OPEN_FAILED;
    }

    if(m_wiWave.pwfx->wFormatTag != WAVE_FORMAT_PCM){
        WaveCloseReadFile(&m_wiWave.hmmio, &m_wiWave.pwfx);
        return ERR_WAVE_INVALID_FORMAT;
    }
    // Seek to the data chunk
    if(WaveStartDataRead(&m_wiWave.hmmio, &m_wiWave.mmck, &m_wiWave.mmckInRIFF) != 0){
        WaveCloseReadFile(&m_wiWave.hmmio, &m_wiWave.pwfx);
        return ERR_WAVE_CORRUPTED_FILE;
    }
    // As a side note, mmck.ckSize will be the size of all the data in this file.
    // That's something which might be handy when calculating the length...

    // Calculate a buffer length, making sure it is an exact multiple of the
    // buffer segment size.
    m_wiWave.dwBufferSize = ((DWORD)m_wiWave.pwfx->nAvgBytesPerSec * (((NUM_BUFFER_SEGMENTS * PLAYBACK_TIMER_PERIOD) / 10)) / 100);

    // Create the secondary DirectSoundBuffer object to receive our sound data.
    memset(&dsbd, 0, sizeof(DSBUFFERDESC));
    dsbd.dwSize        = sizeof(DSBUFFERDESC);
    dsbd.dwFlags       = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GETCURRENTPOSITION2 | m_SecondBuffFocus;
    dsbd.dwBufferBytes = m_wiWave.dwBufferSize;
    dsbd.lpwfxFormat   = m_wiWave.pwfx;



	//ktj 고침
    dsRetVal = lpDirectSound->CreateSoundBuffer(&dsbd, &lpDSBStreamBuffer, NULL);
#ifdef PT_GFX_GL
	if ( dsRetVal==DS_OK ) PtSound_MarkMusic( lpDSBStreamBuffer );
#endif

    if(dsRetVal != DS_OK){
        return ERR_CREATEDSB_FAILED;
    }

    m_wiWave.lpDSBStreamBuffer = lpDSBStreamBuffer;
    m_wiWave.bFoundEnd         = FALSE;
    m_wiWave.dwBytesRemaining  = 0;
    m_wiWave.bLoopFile         = TRUE;

    LSFillDataBuffer();

    m_wiWave.bDonePlaying      = FALSE;

    if(lpDSBStreamBuffer){
		lpDSBStreamBuffer->SetVolume((((VOL_MAX+VOL_SHIFT) * VOL_MULT)));
		lpDSBStreamBuffer->SetPan(((((PAN_MAX/2)+PAN_SHIFT) * PAN_MULT)));
		lpDSBStreamBuffer->SetFrequency(m_wiWave.pwfx->nSamplesPerSec);
    }
    return DS_OK;
}


void PlayBGM(void)
{
	if(LSEnable == TRUE){
        if(m_bPlaying){
            m_bIsCloseOnRun = FALSE;
            LSStopBackground(0L);
        }
        if(m_bFileOpen && lpDSBStreamBuffer){
            // 연주 시작위치를 파일 맨 처음으로...
            lpDSBStreamBuffer->SetCurrentPosition(0);
            lpDSBStreamBuffer->Play(0, 0, DSBPLAY_LOOPING);
        }else{
            m_bPlaying   = FALSE;
            m_bTimerInit = FALSE;
            return;
        }

        if(timeBeginPeriod(PLAYBACK_TIMER_PERIOD / PLAYBACK_OVERSAMPLE ) != 0){
            // 타이머를 만들지 않는다.
            lpDSBStreamBuffer->Stop();
            m_bPlaying   = FALSE;
            m_bTimerInit = FALSE;
            return;
        }else{
            m_lInTimer = FALSE;
            if((m_uTimerID = timeSetEvent(PLAYBACK_TIMER_PERIOD / PLAYBACK_OVERSAMPLE,
                                        PLAYBACK_TIMER_ACCURACY, LSTimeRunFuncForDSound,
                                        (DWORD)0, TIME_PERIODIC )) != 0){
                m_bTimerInit = TRUE;
            }
        }
        m_bPlaying = TRUE;
    }
}

void StopBGM(void)
{
	if(LSEnable == TRUE && m_bPlaying)
		LSStopBackground(0);
}

void SetVolumeBGM( DWORD dwVol )
{
	if(LSEnable == TRUE && m_bPlaying) {
		lpDSBStreamBuffer->SetVolume( (dwVol+VOL_SHIFT) * VOL_MULT); 
	}
}




BOOL Init_BGM(HWND hwnd)
{
    m_hWnd          = hwnd;

	lpDirectSound = NULL;

	lpDSBStreamBuffer = NULL;

	m_lInTimer        = FALSE;
	m_bTimerInit      = FALSE;
	m_bPlaying        = FALSE;
	m_bIsCloseOnRun   = FALSE;
	m_bFileOpen       = FALSE;
	m_uTimerID        = 0;
	m_SecondBuffFocus = 0;

    HRESULT         dsrval;

	dsrval = DirectSoundCreate(NULL, &lpDirectSound, NULL);

    if(SUCCEEDED(dsrval) && (NULL != lpDirectSound)){
        //dsrval = lpDirectSound->Initialize(NULL);

        //if(SUCCEEDED(dsrval)){
            dsrval = lpDirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
		//}else{
		//	dsrval = FALSE;
		//}

		if(!SUCCEEDED(dsrval)){
        	dsrval        = FALSE;
            lpDirectSound->Release();
	        lpDirectSound = NULL;
	    }
    }
	
	else{
		//ktj: 사운드카드없는사람을위해 메시지박스삭제함.
		//MessageBox(hwnd, "Failed to create DirectSound COM object", "Lemon Sound Error!", MB_OK | MB_ICONSTOP);
    }
	

    if(SUCCEEDED(dsrval)){
        LSEnable = TRUE;
		return TRUE;
    }else{
		//CoUninitialize();
        LSEnable = FALSE;
		return FALSE;
    }
	



	LSEnable = TRUE;

	return TRUE;
}


void Uninit_BGM(void)
{
	if(LSEnable == TRUE){
		
        LSStopBackground(DSSTREAM_STOPF_NOREOPEN | DSSTREAM_STOPF_NOEXIT);


        if(m_bTimerInit){
            timeKillEvent(m_uTimerID);
            timeEndPeriod(PLAYBACK_TIMER_PERIOD / PLAYBACK_OVERSAMPLE);
            // Busy wait for timer func to exit
            while(InterlockedExchange(&m_lInTimer, TRUE)) Sleep(100);
            m_bTimerInit = FALSE;
        }
		
		
		if (lpDSBStreamBuffer) {
			lpDSBStreamBuffer->Stop();
            lpDSBStreamBuffer->Release();
			lpDSBStreamBuffer = NULL;
        }


        // DSOUND 오브젝트 제거
        if(lpDirectSound != NULL){
            lpDirectSound->Release();
            lpDirectSound = NULL;
            //CoUninitialize();
        }

		
    }
}

// pluto 비스타에서 채팅 때문에 윈도우 생성전에 사운드 만든다
void InitDirectSound()
{
	lpDirectSound = NULL;

	rval_g_lpDs = DirectSoundCreate(NULL, &g_lpDS, NULL);

	dsrval_lpDirectSound = DirectSoundCreate(NULL, &lpDirectSound, NULL);
}

//=======================================================================end
//sound 배경 wav를 타이머로 읽어서 연주
//=======================================================================end