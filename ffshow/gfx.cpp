/*
 * Copyright 2016 Joey  <joeyye@foxmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/resource.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <unistd.h>

#include <EGLUtils.h>
#include <ui/GraphicBuffer.h>
#include <ui/DisplayInfo.h>
#include "data.h"

float calculate_A( float src[][4], int n )  
{  
    int i,j,k,x,y;  
    float tmp[4][4], t;  
    float result = 0.0;  
       
    if( n == 1 )  
    {  
        return src[0][0];  
    }  
       
    for( i = 0; i < n; ++i )  
    {  
        for( j = 0; j < n - 1; ++j )  
        {  
            for( k = 0; k < n - 1; ++k )  
            {  
                x = j + 1;  
                y = k >= i ? k + 1 : k;  
                   
                tmp[j][k] = src[x][y];  
            }  
        }  
           
        t = calculate_A( tmp, n - 1 );  
           
        if( i % 2 == 0 )  
        {  
            result += src[0][i] * t;  
        }  
        else 
        {  
            result -= src[0][i] * t;  
        }  
    }  
   
    return result;  
}  
   
void calculate_A_adjoint( float src[4][4], float dst[4][4], int n )  
{  
    int i, j, k, t, x, y;  
    float tmp[4][4];  
   
    if( n == 1 )  
    {  
        dst[0][0] = 1;  
        return;  
    }  
       
    for( i = 0; i < n; ++i )  
    {  
        for( j = 0; j < n; ++j )  
        {  
            for( k = 0; k < n - 1; ++k )  
            {  
                for( t = 0; t < n - 1; ++t )  
                {  
                    x = k >= i ? k + 1 : k ;  
                    y = t >= j ? t + 1 : t;  
                       
                    tmp[k][t] = src[x][y];  
                }  
            }  
               
            dst[j][i]  =  calculate_A( tmp, n - 1 );  
               
            if( ( i + j ) % 2 == 1 )  
            {  
                dst[j][i] = -1*dst[j][i];  
            }  
        }  
    }  
}  
   
void inverse( float src[4][4],float dst[4][4] )  
{  
	int n=4;
    float A = calculate_A( src, n );  
    float tmp[4][4];  
    int i, j;  
   
    calculate_A_adjoint( src, tmp, n );    
   
    for( i = 0; i < n; ++i )    
    {    
        for( j = 0; j < n; ++j )    
        {    
            dst[i][j] =  tmp[i][j] / A ;  
        }    
    }  
   
}  
void makeFrustum(float *matrix,float left, float right,float bottom,float top,float znear,float zfar)
{
    float X = 2*znear/(right-left);
    float Y = 2*znear/(top-bottom);
    float A = (right+left)/(right-left);
    float B = (top+bottom)/(top-bottom);
    float C = -(zfar+znear)/(zfar-znear);
    float D = -2*zfar*znear/(zfar-znear);
	float tmp[16]={
		X, 0, A, 0,
		0, Y, B, 0,
		0, 0, C, D,
        0, 0, -1, 0
	};
	memcpy(matrix,tmp,16*sizeof(float));
}

void makePerspective(float *matrix,float fovy, float aspect, float znear, float zfar)
{
    float ymax = znear * tan(fovy * 3.1415926 / 360.0);
    float ymin = -ymax;
    float xmin = ymin * aspect;
    float xmax = ymax * aspect;

    makeFrustum(matrix,xmin, xmax, ymin, ymax, znear, zfar);
}

void rotatev (float *matrix,float radians, float *v) 
{
	float r=sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    float x = v[0] / r, y = v[1] / r, z = v[2] / r;
    float s = sin(radians) , c = cos(radians) , t = 1 - c;
	float tmp[16]={
		t * x * x + c, t * x * y - s * z, t * x * z + s * y,0,
		t * x * y + s * z, t * y * y + c, t * y * z - s * x,0,
		t * x * z - s * y, t * y * z + s * x, t * z * z + c,0,
		0,0,0,1,
	};
	memcpy(matrix,tmp,16*sizeof(float));
}
void multMatrix(float *matrix,float *m)
{
	float sum=0;
	float tmp[16];
	for(int i=0;i<4;++i) 
		for(int j=0;j<4;++j) {
			sum=0;
			for(int k=0;k<4;++k) {
			   sum+=matrix[4*i+k]*m[4*k+j];
			}
			tmp[4*i+j]=sum;
		}
	memcpy(matrix,tmp,16*sizeof(float));
}
void translate(float *matrix,float *v)
{
	int i=0;
	for(int i=0;i<3;++i) {
		matrix[4*i+3] = v[i];
	}
}
void rotate(float *matrix,float angle, float *v) 
{
  float radians = angle * 3.1415926 / 180.0;
  float tmp[16];

  rotatev(tmp,radians, v);
  multMatrix(matrix,tmp);
}

static const char vShader_cube[] = 
	  "attribute highp vec3 vnormal;\n"
	  "attribute highp vec3 vpos;\n"
	  "attribute highp vec2 vtex;\n"
	  "uniform highp mat4 nmatrix;\n"
	  "uniform highp mat4 vmatrix;\n"
	  "uniform highp mat4 pmatrix;\n"
	  "varying highp vec2 vtexcoord;\n"
	  "varying highp vec3 vlight;\n"
	  "void main(void) {\n"
	  "  gl_Position = pmatrix * vmatrix * vec4(vpos, 1.0);\n"
	  "  vtexcoord = vtex;\n"
	  "  // Apply lighting effect\n"
	  "  highp vec3 ambientLight = vec3(0.6, 0.6, 0.6);\n"
	  "  highp vec3 directionalLightColor = vec3(0.5, 0.5, 0.75);\n"
	  "  highp vec3 directionalVector = vec3(0.85, 0.8, 0.75);\n"
	  "  highp vec4 transformedNormal = nmatrix * vec4(vnormal, 1.0);\n"
	  "  highp float directional = max(dot(transformedNormal.xyz, directionalVector), 0.0);\n"
	  "  vlight = ambientLight + (directionalLightColor * directional);\n"
	  "}\n";
static const char fShader_cube[] = 
	  "varying highp vec2 vtexcoord;\n"
      "varying highp vec3 vlight;\n"
	  "uniform sampler2D sampler;\n"
	  "void main(void) {\n"
      "  highp vec4 color = texture2D(sampler, vtexcoord);\n"
      "  gl_FragColor = vec4(color.rgb * vlight, color.a);\n"
	  "}\n";

static const char vShader_back[] = "attribute vec4 vPosition;\n"
    "attribute vec2 vTex;\n"
    "varying vec2 texCoords;\n"
    "uniform mat4 vtrans;\n"
    "uniform mat4 ttrans;\n"
    "void main() {\n"
    "  texCoords = vec2(ttrans * vec4(vTex, 0, 1.0)); \n" 
    "  gl_Position = vtrans*vPosition;\n"
    "}\n";

static const char fShader_back[] = 
    "precision mediump float;\n"
    "uniform sampler2D texsampler;\n"
    "varying vec2 texCoords;\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(texsampler, texCoords);\n"
    "}\n";
	  
GLuint loadShader(GLenum shaderType, const char* pSource) 
{
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    printf( "Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
            } else {
                printf("Guessing at GL_INFO_LOG_LENGTH size\n");
                char* buf = (char*) malloc(0x1000);
                if (buf) {
                    glGetShaderInfoLog(shader, 0x1000, NULL, buf);
                    printf("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
            }
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) 
{
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, pixelShader);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    printf("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}
void setupKHRTexture(vaglContext *ctx)
{
	int i;

	ctx->gl.khrimg=new EGLImageKHR[ctx->gl.maxclientBufffer];
	ctx->gl.khrtexid=new GLuint[ctx->gl.maxclientBufffer];

	glGenTextures(ctx->gl.maxclientBufffer, &ctx->gl.khrtexid[i]);
	glGenTextures(1,&ctx->gl.buffertexid);
	for(i=0;i<ctx->gl.maxclientBufffer;++i) {
		glBindTexture(GL_TEXTURE_2D, ctx->gl.khrtexid[i]);
		ctx->gl.khrimg[i] = eglCreateImageKHR(ctx->gl.dpy, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,ctx->gl.clientbuffer[i], 0);
		glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)ctx->gl.khrimg[i]);
	}
	
}

void putKHRTextureInfo(vaglContext *ctx,void *buffer,int num)
{
	ctx->gl.clientbuffer=(EGLClientBuffer*)buffer;
	ctx->gl.maxclientBufffer=num;
}

void** CreateVAGfxBuffer(vaglContext *ctx,int width,int height,int num,void **clientbuffer)
{
	GraphicBuffer** gfxBuffer_ptr;
	int stride = (width+0x3f) & (~0x3f);
	void** handle_ptr =new void*[num];
	EGLClientBuffer *clientBuffer_ptr=new EGLClientBuffer [num];
	gfxBuffer_ptr=new GraphicBuffer*[num];
	
	for(int i=0;i<num;++i) {
		gfxBuffer_ptr[i]=new GraphicBuffer(stride, height, FORMAT_NV12,GraphicBuffer::USAGE_HW_RENDER);
		clientBuffer_ptr[i] = (EGLClientBuffer)gfxBuffer_ptr[i]->getNativeBuffer();
		handle_ptr[i]=(void*)((gfxBuffer_ptr[i]->getNativeBuffer())->handle);
	}

	*clientbuffer=clientBuffer_ptr;
	ctx->gl.gfxBuffer_ptr=gfxBuffer_ptr;
	
	return handle_ptr;
}

bool setupGraphics(vaglContext *ctx,int w, int h) 
{
    ctx->gl.cube.program = createProgram(vShader_cube, fShader_cube);
    if (!ctx->gl.cube.program) {
        return false;
    }
    ctx->gl.cube.vpos = glGetAttribLocation(ctx->gl.cube.program, "vpos");
    ctx->gl.cube.vtex = glGetAttribLocation(ctx->gl.cube.program, "vtex");
	ctx->gl.cube.vnor = glGetAttribLocation(ctx->gl.cube.program, "vnormal");
    ctx->gl.cube.sampler = glGetUniformLocation(ctx->gl.cube.program, "sampler");
    ctx->gl.cube.vtrans = glGetUniformLocation(ctx->gl.cube.program, "vmatrix");
    ctx->gl.cube.ttrans = glGetUniformLocation(ctx->gl.cube.program, "pmatrix");
	ctx->gl.cube.ntrans = glGetUniformLocation(ctx->gl.cube.program, "nmatrix");
	
    ctx->gl.background.program = createProgram(vShader_back, fShader_back);
    if (!ctx->gl.background.program) {
        return false;
    }
    ctx->gl.background.vpos = glGetAttribLocation(ctx->gl.background.program, "vPosition");
    ctx->gl.background.vtex = glGetAttribLocation(ctx->gl.background.program, "vTex");
    ctx->gl.background.sampler = glGetUniformLocation(ctx->gl.background.program, "texsampler");
    ctx->gl.background.vtrans = glGetUniformLocation(ctx->gl.background.program, "vtrans");
    ctx->gl.background.ttrans = glGetUniformLocation(ctx->gl.background.program, "ttrans");
	
    glViewport(0, 0, w, h);
    return true;
}

static void getMaindispSize(EGLint& w, EGLint& h)
{
	status_t err;
	uint32_t width, height;
    // Get main display parameters.
    sp<IBinder> mainDpy = SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain);
    DisplayInfo mainDpyInfo;
    err = SurfaceComposerClient::getDisplayInfo(mainDpy, &mainDpyInfo);
    if (err != NO_ERROR) {
        printf("ERROR: unable to get display characteristics\n");
        return;
    }

    if (mainDpyInfo.orientation != DISPLAY_ORIENTATION_0 &&
            mainDpyInfo.orientation != DISPLAY_ORIENTATION_180) {
        // rotated
        width = mainDpyInfo.h;
        height = mainDpyInfo.w;
    } else {
        width = mainDpyInfo.w;
        height = mainDpyInfo.h;
    }
	w=width;
	h=height;
}

int gfxInit(vaglContext *ctx)
{
    EGLBoolean returnValue;
    EGLConfig myConfig = {0};
	EGLint majorVersion;
	EGLint minorVersion;
    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    EGLint s_configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE };
    EGLint w, h;


    ctx->gl.dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (ctx->gl.dpy == EGL_NO_DISPLAY) {
        printf("eglGetDisplay returned EGL_NO_DISPLAY.\n");
        return 0;
    }

    returnValue = eglInitialize(ctx->gl.dpy, &majorVersion, &minorVersion);
    if (returnValue != EGL_TRUE) {
        printf("eglInitialize failed\n");
        return 0;
    }

	getMaindispSize(w,h);
    ctx->gl.windowSurface=new WindowSurface;
    ctx->gl.window= ctx->gl.windowSurface->getSurface();
    returnValue = EGLUtils::selectConfigForNativeWindow(ctx->gl.dpy, s_configAttribs, ctx->gl.window, &myConfig);
    if (returnValue) {
        printf("EGLUtils::selectConfigForNativeWindow() returned %d\n", returnValue);
        return 1;
    }

    ctx->gl.surface = eglCreateWindowSurface(ctx->gl.dpy, myConfig, ctx->gl.window, NULL);
    if (ctx->gl.surface == EGL_NO_SURFACE) {
        printf("gelCreateWindowSurface failed.\n");
        return 1;
    }

    ctx->gl.context = eglCreateContext(ctx->gl.dpy, myConfig, EGL_NO_CONTEXT, context_attribs);
    if (ctx->gl.context == EGL_NO_CONTEXT) {
        printf("eglCreateContext failed\n");
        return 1;
    }
    returnValue = eglMakeCurrent(ctx->gl.dpy, ctx->gl.surface, ctx->gl.surface, ctx->gl.context);
    if (returnValue != EGL_TRUE) {
        return 1;
    }
    eglQuerySurface(ctx->gl.dpy, ctx->gl.surface, EGL_WIDTH, &w);
    eglQuerySurface(ctx->gl.dpy, ctx->gl.surface, EGL_HEIGHT, &h);
    GLint dim = w < h ? w : h;

    if(!setupGraphics(ctx,w, h)) {
        printf("Could not set up graphics.\n");
        return 1;
    }
	return 0;
}

void gfxUninit(vaglContext *ctx)
{
	delete ctx->gl.gfxBuffer_ptr;
    eglMakeCurrent(ctx->gl.dpy, EGL_NO_SURFACE , EGL_NO_SURFACE , EGL_NO_CONTEXT);
    eglDestroySurface(ctx->gl.dpy, ctx->gl.surface);
    eglDestroyContext(ctx->gl.dpy, ctx->gl.context);
    eglTerminate(ctx->gl.dpy);
	delete ctx->gl.windowSurface;
}
void SetTransMatrix(vaglContext *ctx)
{
	GLfloat nmatrix[16];
	GLfloat tmatrix[16];
    GLfloat vmatrix[16]={
		1.0,0,0,0,
		0,0.4,0,0,
		0,0,1,0,
		0,0,0,1,
		};
    GLfloat v[3]={1,0,1};
	static float angle=0;

	angle+=0.05;
	makePerspective(tmatrix,45, (float)ctx->gl.width/(float)ctx->gl.height, 0.1, 100.0);
	v[0]=-4.0; v[1]=-2.0; v[2]=-8.0;
	translate(vmatrix,v);
	v[0]=1.0; v[1]=0.0; v[2]=1.0;
	rotate(vmatrix,angle, v);

	inverse((float (*)[4])vmatrix,(float (*)[4])nmatrix);

	glUniformMatrix4fv(ctx->gl.cube.vtrans, 1, GL_TRUE, vmatrix);
	glUniformMatrix4fv(ctx->gl.cube.ttrans, 1, GL_TRUE, tmatrix);
	glUniformMatrix4fv(ctx->gl.cube.ntrans, 1, GL_FALSE, nmatrix);

}
void SetTransMatrix_background(vaglContext *ctx)
{
	float r=1.0f;
	float x=0.0f;
	float y=0.0f;
	
    GLfloat vmatrix[16]={
		0.0f,float(1.0/r),0.0f,0.0f,
		float(1.0/r),0.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,0.0f,
		x,y,0.0f,1.0f,
	};
    GLfloat tmatrix[16]={
		1.0f,0.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,0.0f,
		0.0f,0.0f,0.0f,1.0f,
	};

	glUniformMatrix4fv(ctx->gl.background.vtrans, 1, GL_FALSE, vmatrix);
	glUniformMatrix4fv(ctx->gl.background.ttrans, 1, GL_FALSE, tmatrix);
}

GLuint GetKHRid(vaglContext *ctx,VASurfaceID id)
{
    GLuint khrid;	
	int ptr=id;
	if(id==-1){
	   ptr=vaGetShowSurface(ctx);
	}
	khrid=ctx->gl.khrtexid[ptr];
	return khrid;
}
void renderFrame(vaglContext *ctx,VASurfaceID id) 
{
	static const GLfloat triVertices[]={
		// Front face
		-1.0, -1.0,  1.0,
		 1.0, -1.0,  1.0,
		 1.0,  1.0,  1.0,
		-1.0,  1.0,  1.0,

		// Back face
		-1.0, -1.0, -1.0,
		-1.0,  1.0, -1.0,
		 1.0,  1.0, -1.0,
		 1.0, -1.0, -1.0,

		// Top face
		-1.0,  1.0, -1.0,
		-1.0,  1.0,  1.0,
		 1.0,  1.0,  1.0,
		 1.0,  1.0, -1.0,

		// Bottom face
		-1.0, -1.0, -1.0,
		 1.0, -1.0, -1.0,
		 1.0, -1.0,  1.0,
		-1.0, -1.0,  1.0,

		// Right face
		 1.0, -1.0, -1.0,
		 1.0,  1.0, -1.0,
		 1.0,  1.0,  1.0,
		 1.0, -1.0,  1.0,

		// Left face
		-1.0, -1.0, -1.0,
		-1.0, -1.0,  1.0,
		-1.0,  1.0,  1.0,
		-1.0,  1.0, -1.0
	};
	static const GLfloat triTexs[]={
		// Front
		0.0,  0.0,
		1.0,  0.0,
		1.0,  1.0,
		0.0,  1.0,
		// Back
		0.0,  0.0,
		1.0,  0.0,
		1.0,  1.0,
		0.0,  1.0,
		// Top
		0.0,  0.0,
		1.0,  0.0,
		1.0,  1.0,
		0.0,  1.0,
		// Bottom
		0.0,  0.0,
		1.0,  0.0,
		1.0,  1.0,
		0.0,  1.0,
		// Right
		0.0,  0.0,
		1.0,  0.0,
		1.0,  1.0,
		0.0,  1.0,
		// Left
		0.0,  0.0,
		1.0,  0.0,
		1.0,  1.0,
		0.0,  1.0
	};
	static const GLfloat triNormals[] = {
	   // Front
		0.0,  0.0,  1.0,
		0.0,  0.0,  1.0,
		0.0,  0.0,  1.0,
		0.0,  0.0,  1.0,

	   // Back
		0.0,  0.0, -1.0,
		0.0,  0.0, -1.0,
		0.0,  0.0, -1.0,
		0.0,  0.0, -1.0,

	   // Top
		0.0,  1.0,  0.0,
		0.0,  1.0,  0.0,
		0.0,  1.0,  0.0,
		0.0,  1.0,  0.0,

	   // Bottom
		0.0, -1.0,  0.0,
		0.0, -1.0,  0.0,
		0.0, -1.0,  0.0,
		0.0, -1.0,  0.0,

	   // Right
		1.0,  0.0,  0.0,
		1.0,  0.0,  0.0,
		1.0,  0.0,  0.0,
		1.0,  0.0,  0.0,

	   // Left
	   -1.0,  0.0,  0.0,
	   -1.0,  0.0,  0.0,
	   -1.0,  0.0,  0.0,
	   -1.0,  0.0,  0.0
 	};

	static const GLushort triIndices[]= {
		0,  1,  2,      0,  2,  3,    // front
		4,  5,  6,      4,  6,  7,    // back
		8,  9,  10,     8,  10, 11,   // top
		12, 13, 14,     12, 14, 15,   // bottom
		16, 17, 18,     16, 18, 19,   // right
		20, 21, 22,     20, 22, 23    // left
	};
	
	static const GLfloat triVertices_back[]={
		-1.0f, -1.0f,
		-1.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, -1.0f,
		-1.0f, -1.0f,
	};
	static const GLfloat triTexs_back[]={
		0.0f,0.0f, 
		0.0f,1.0f, 
		1.0f,1.0f, 
		1.0f,1.0f, 
		1.0f,0.0f, 
		0.0f,0.0f, 
	};
	
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glUseProgram(ctx->gl.background.program);
    glVertexAttribPointer(ctx->gl.background.vpos, 2, GL_FLOAT, GL_FALSE, 0, triVertices_back);
    glEnableVertexAttribArray(ctx->gl.background.vpos);
    glVertexAttribPointer(ctx->gl.background.vtex, 2, GL_FLOAT, GL_FALSE, 0, triTexs_back);
    glEnableVertexAttribArray(ctx->gl.background.vtex);

    glUniform1i(ctx->gl.background.sampler, 0);
	SetTransMatrix_background(ctx);
	glBindTexture(GL_TEXTURE_2D, GetKHRid(ctx,id));
    glDrawArrays(GL_TRIANGLES, 0, 6);
	
    glUseProgram(ctx->gl.cube.program);
    glVertexAttribPointer(ctx->gl.cube.vpos, 3, GL_FLOAT, GL_FALSE, 0, triVertices);
    glEnableVertexAttribArray(ctx->gl.cube.vpos);
    glVertexAttribPointer(ctx->gl.cube.vtex, 2, GL_FLOAT, GL_FALSE, 0, triTexs);
    glEnableVertexAttribArray(ctx->gl.cube.vtex);
	glVertexAttribPointer(ctx->gl.cube.vnor, 3, GL_FLOAT, GL_FALSE, 0, triNormals);
	glEnableVertexAttribArray(ctx->gl.cube.vnor);

    glUniform1i(ctx->gl.cube.sampler, 0);
	SetTransMatrix(ctx);
	glBindTexture(GL_TEXTURE_2D, GetKHRid(ctx,id));
    glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_SHORT,triIndices);
	
	eglSwapBuffers(ctx->gl.dpy, ctx->gl.surface);
}
