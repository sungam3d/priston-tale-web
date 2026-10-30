// ptgl.h - the OpenGL functions ptgfx uses.
//
// Only the OpenGL ES 3.0 / WebGL2 subset of OpenGL 3.3 core, so the same
// renderer later builds for the browser (Emscripten provides these natively).
// On Windows the functions are loaded at runtime from opengl32.dll /
// wglGetProcAddress (no extra libraries or downloads needed).
#pragma once

#ifdef PT_WEB
// Browser: WebGL2 through Emscripten's OpenGL ES 3.0 header; nothing to load.
#include <windows.h>
#include <GLES3/gl3.h>
#define glClearDepth	glClearDepthf
#define glDepthRange	glDepthRangef
#else

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>		// OpenGL 1.1 (types + functions exported by opengl32.dll)

typedef char			GLchar;
typedef ptrdiff_t		GLsizeiptr;
typedef ptrdiff_t		GLintptr;

#define GL_ARRAY_BUFFER				0x8892
#define GL_STREAM_DRAW				0x88E0
#define GL_DYNAMIC_DRAW				0x88E8
#define GL_FRAGMENT_SHADER			0x8B30
#define GL_VERTEX_SHADER			0x8B31
#define GL_COMPILE_STATUS			0x8B81
#define GL_LINK_STATUS				0x8B82
#define GL_INFO_LOG_LENGTH			0x8B84
#define GL_TEXTURE0					0x84C0
#define GL_CLAMP_TO_EDGE			0x812F
#define GL_MIRRORED_REPEAT			0x8370
#define GL_FRAMEBUFFER				0x8D40
#define GL_READ_FRAMEBUFFER			0x8CA8
#define GL_DRAW_FRAMEBUFFER			0x8CA9
#define GL_RENDERBUFFER				0x8D41
#define GL_COLOR_ATTACHMENT0		0x8CE0
#define GL_DEPTH_ATTACHMENT			0x8D00
#define GL_DEPTH_STENCIL_ATTACHMENT	0x821A
#define GL_DEPTH24_STENCIL8			0x88F0
#define GL_DEPTH_COMPONENT24		0x81A6
#define GL_FRAMEBUFFER_COMPLETE		0x8CD5
#define GL_RGBA8					0x8058
#define GL_BGRA						0x80E1
#define GL_FUNC_ADD					0x8006
#define GL_UNPACK_ROW_LENGTH		0x0CF2
#define GL_TEXTURE_COMPARE_MODE		0x884C
#define GL_TEXTURE_COMPARE_FUNC		0x884D
#define GL_COMPARE_REF_TO_TEXTURE	0x884E

#define PTGL_FUNCTIONS(X) \
	X( void,	glGenBuffers,				(GLsizei n, GLuint *buffers) ) \
	X( void,	glDeleteBuffers,			(GLsizei n, const GLuint *buffers) ) \
	X( void,	glBindBuffer,				(GLenum target, GLuint buffer) ) \
	X( void,	glBufferData,				(GLenum target, GLsizeiptr size, const void *data, GLenum usage) ) \
	X( void,	glBufferSubData,			(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) ) \
	X( void,	glGenVertexArrays,			(GLsizei n, GLuint *arrays) ) \
	X( void,	glBindVertexArray,			(GLuint array) ) \
	X( void,	glVertexAttribPointer,		(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) ) \
	X( void,	glEnableVertexAttribArray,	(GLuint index) ) \
	X( void,	glDisableVertexAttribArray,	(GLuint index) ) \
	X( void,	glVertexAttrib4f,			(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) ) \
	X( GLuint,	glCreateShader,				(GLenum type) ) \
	X( void,	glShaderSource,				(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length) ) \
	X( void,	glCompileShader,			(GLuint shader) ) \
	X( void,	glGetShaderiv,				(GLuint shader, GLenum pname, GLint *params) ) \
	X( void,	glGetShaderInfoLog,			(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) ) \
	X( void,	glDeleteShader,				(GLuint shader) ) \
	X( GLuint,	glCreateProgram,			(void) ) \
	X( void,	glAttachShader,				(GLuint program, GLuint shader) ) \
	X( void,	glBindAttribLocation,		(GLuint program, GLuint index, const GLchar *name) ) \
	X( void,	glLinkProgram,				(GLuint program) ) \
	X( void,	glGetProgramiv,				(GLuint program, GLenum pname, GLint *params) ) \
	X( void,	glGetProgramInfoLog,		(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) ) \
	X( void,	glUseProgram,				(GLuint program) ) \
	X( GLint,	glGetUniformLocation,		(GLuint program, const GLchar *name) ) \
	X( void,	glUniform1i,				(GLint location, GLint v0) ) \
	X( void,	glUniform1f,				(GLint location, GLfloat v0) ) \
	X( void,	glUniform2f,				(GLint location, GLfloat v0, GLfloat v1) ) \
	X( void,	glUniform3f,				(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) ) \
	X( void,	glUniform4f,				(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) ) \
	X( void,	glUniform1iv,				(GLint location, GLsizei count, const GLint *value) ) \
	X( void,	glUniformMatrix3fv,			(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) ) \
	X( void,	glDrawBuffers,				(GLsizei n, const GLenum *bufs) ) \
	X( void,	glActiveTexture,			(GLenum texture) ) \
	X( void,	glGenerateMipmap,			(GLenum target) ) \
	X( void,	glBlendFuncSeparate,		(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) ) \
	X( void,	glGenFramebuffers,			(GLsizei n, GLuint *framebuffers) ) \
	X( void,	glDeleteFramebuffers,		(GLsizei n, const GLuint *framebuffers) ) \
	X( void,	glBindFramebuffer,			(GLenum target, GLuint framebuffer) ) \
	X( void,	glFramebufferTexture2D,		(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) ) \
	X( void,	glFramebufferRenderbuffer,	(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) ) \
	X( GLenum,	glCheckFramebufferStatus,	(GLenum target) ) \
	X( void,	glGenRenderbuffers,			(GLsizei n, GLuint *renderbuffers) ) \
	X( void,	glDeleteRenderbuffers,		(GLsizei n, const GLuint *renderbuffers) ) \
	X( void,	glBindRenderbuffer,			(GLenum target, GLuint renderbuffer) ) \
	X( void,	glRenderbufferStorage,		(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) ) \
	X( void,	glBlitFramebuffer,			(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) )

#define PTGL_DECLARE(ret, name, args) typedef ret (APIENTRY *PFN_##name) args; extern PFN_##name pt_##name;
PTGL_FUNCTIONS(PTGL_DECLARE)
#undef PTGL_DECLARE

// Use the loaded pointers under their normal GL names.
#define glGenBuffers				pt_glGenBuffers
#define glDeleteBuffers				pt_glDeleteBuffers
#define glBindBuffer				pt_glBindBuffer
#define glBufferData				pt_glBufferData
#define glBufferSubData				pt_glBufferSubData
#define glGenVertexArrays			pt_glGenVertexArrays
#define glBindVertexArray			pt_glBindVertexArray
#define glVertexAttribPointer		pt_glVertexAttribPointer
#define glEnableVertexAttribArray	pt_glEnableVertexAttribArray
#define glDisableVertexAttribArray	pt_glDisableVertexAttribArray
#define glVertexAttrib4f			pt_glVertexAttrib4f
#define glCreateShader				pt_glCreateShader
#define glShaderSource				pt_glShaderSource
#define glCompileShader				pt_glCompileShader
#define glGetShaderiv				pt_glGetShaderiv
#define glGetShaderInfoLog			pt_glGetShaderInfoLog
#define glDeleteShader				pt_glDeleteShader
#define glCreateProgram				pt_glCreateProgram
#define glAttachShader				pt_glAttachShader
#define glBindAttribLocation		pt_glBindAttribLocation
#define glLinkProgram				pt_glLinkProgram
#define glGetProgramiv				pt_glGetProgramiv
#define glGetProgramInfoLog			pt_glGetProgramInfoLog
#define glUseProgram				pt_glUseProgram
#define glGetUniformLocation		pt_glGetUniformLocation
#define glUniform1i					pt_glUniform1i
#define glUniform1f					pt_glUniform1f
#define glUniform2f					pt_glUniform2f
#define glUniform3f					pt_glUniform3f
#define glUniform4f					pt_glUniform4f
#define glUniform1iv				pt_glUniform1iv
#define glUniformMatrix3fv			pt_glUniformMatrix3fv
#define glDrawBuffers				pt_glDrawBuffers
#define glActiveTexture				pt_glActiveTexture
#define glGenerateMipmap			pt_glGenerateMipmap
#define glBlendFuncSeparate			pt_glBlendFuncSeparate
#define glGenFramebuffers			pt_glGenFramebuffers
#define glDeleteFramebuffers		pt_glDeleteFramebuffers
#define glBindFramebuffer			pt_glBindFramebuffer
#define glFramebufferTexture2D		pt_glFramebufferTexture2D
#define glFramebufferRenderbuffer	pt_glFramebufferRenderbuffer
#define glCheckFramebufferStatus	pt_glCheckFramebufferStatus
#define glGenRenderbuffers			pt_glGenRenderbuffers
#define glDeleteRenderbuffers		pt_glDeleteRenderbuffers
#define glBindRenderbuffer			pt_glBindRenderbuffer
#define glRenderbufferStorage		pt_glRenderbufferStorage
#define glBlitFramebuffer			pt_glBlitFramebuffer

#endif // !PT_WEB

// Creates an OpenGL 3.3 core context on the window and loads the functions.
// Returns FALSE (and writes ptgfx.log) if the machine cannot do it.
BOOL PtGL_CreateContext( HWND hWnd );
void PtGL_DestroyContext();
void PtGL_SwapBuffers();
void PtGL_Log( const char *szFormat, ... );
