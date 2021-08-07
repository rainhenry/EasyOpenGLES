
##  视频解码器设置
LIB_FFMPEG=-lavutil -lavformat -lavcodec -lswscale -lswresample -lavdevice -lavfilter -lz

##  FreeType字体支持
INCLUDE_FREETYPE=-I/usr/include/freetype2
LIB_FREETYPE=-lfreetype

##  视窗系统
LIB_EGL_WS=-lglfw

##  OpenGLES
LIB_OPENGLES=-lGLESv2

##  基本系统
LIB_BASE=-pthread

##  C语言工具链
CC=gcc

##  C++工具链
CXX=g++

##  Shader文件列表
SHADER_FILE_LIST= CEasyGL_OnlyGeometryShader_FragmentShader.c \
                  CEasyGL_OnlyGeometryShader_VertexShader.c \
                  CEasyGL_3DTextureShader_VertexShader.c \
                  CEasyGL_3DTextureShader_FragmentShader.c \
                  CDashGL_CircularGradientTrackShader_VertexShader.c \
                  CDashGL_CircularGradientTrackShader_FragmentShader.c \
                  CDashGL_CircularSolidTrackShader_FragmentShader.c \
                  CEasyGL_2DTextureMultiSampleElementShader_FragmentShader.c

all:run

run:cube.o teapot.o monkey.o stb_image.o WindowSystem.o EasyGL.o main.o DashGL.o ShaderSrc.o
	@echo "    [CXX]   run"
	@${CXX} -o run cube.o teapot.o monkey.o stb_image.o WindowSystem.o ShaderSrc.o EasyGL.o DashGL.o main.o ${LIB_FFMPEG} ${INCLUDE_FREETYPE} ${LIB_FREETYPE} ${LIB_EGL_WS} ${LIB_OPENGLES} ${LIB_BASE}
	@echo "All Build Finish!!"

main.o:main.cpp cube.h WindowSystem.h DashGL.h EasyGL.h monkey.h stb_image.h teapot.h IncludeOpenGLES.h
	@echo "    [CXX]   main.o"
	@${CXX} -c -o main.o main.cpp ${LIB_FFMPEG} ${INCLUDE_FREETYPE} ${LIB_FREETYPE} ${LIB_EGL_WS} ${LIB_OPENGLES} ${LIB_BASE}

DashGL.o:DashGL.cpp DashGL.h EasyGL.h ShaderSrc.c ShaderSrc.h
	@echo "    [CXX]   DashGL.o"
	@${CXX} -c -o DashGL.o DashGL.cpp ${LIB_FFMPEG} ${INCLUDE_FREETYPE} ${LIB_FREETYPE} ${LIB_EGL_WS} ${LIB_OPENGLES} ${LIB_BASE} ${LIB_PATH}

EasyGL.o:EasyGL.cpp EasyGL.h IncludeOpenGLES.h
	@echo "    [CXX]   EasyGL.o"
	@${CXX} -c -o EasyGL.o EasyGL.cpp ${LIB_FFMPEG} ${INCLUDE_FREETYPE} ${LIB_FREETYPE} ${LIB_EGL_WS} ${LIB_OPENGLES} ${LIB_BASE}

ShaderSrc.o:ShaderSrc.c ShaderSrc.h IncludeOpenGLES.h
	@echo "    [CC]    ShaderSrc.o"
	@${CC} -c -o ShaderSrc.o ShaderSrc.c ${LIB_EGL_WS} ${LIB_OPENGLES} ${LIB_BASE} ${LIB_PATH}

WindowSystem.o:WindowSystem.cpp WindowSystem.h IncludeOpenGLES.h
	@echo "    [CXX]   WindowSystem.o"
	@${CXX} -c -o WindowSystem.o WindowSystem.cpp ${LIB_FFMPEG} ${INCLUDE_FREETYPE} ${LIB_FREETYPE} ${LIB_EGL_WS} ${LIB_OPENGLES} ${LIB_BASE}

stb_image.o:stb_image.c stb_image.h IncludeOpenGLES.h
	@echo "    [CC]    stb_image.o"
	@${CC} -c -o stb_image.o stb_image.c

monkey.o:monkey.cpp monkey.h IncludeOpenGLES.h
	@echo "    [CXX]   monkey.o"
	@${CXX} -c -o monkey.o monkey.cpp

teapot.o:teapot.c teapot.h IncludeOpenGLES.h
	@echo "    [CC]    teapot.o"
	@${CC} -c -o teapot.o teapot.c

cube.o:cube.c cube.h IncludeOpenGLES.h
	@echo "    [CC]    cube.o"
	@${CC} -c -o cube.o cube.c


ShaderSrc.h ShaderSrc.c:Shader2C ${SHADER_FILE_LIST}
	@echo "    [SDR]   ShaderSrc.c"
	@-rm -rf *.sdf
	@-rm -rf *.sdb
	@./Shader2C -o ShaderSrc.c ${SHADER_FILE_LIST}


Shader2C:Shader2C.cpp IncludeOpenGLES.h
	@echo "    [CXX]   Shader2C"
	@${CXX} -o Shader2C Shader2C.cpp ${LIB_PATH}


clean:
	@-rm -rf *.o
	@-rm -rf run
	@-rm -rf *.sdf
	@-rm -rf *.sdb
	@-rm -rf Shader2C
	@echo "Clean all Finish!!"


