
//  为了兼容各种平台的OpenGLES的头文件设置需求
//  这里采用了统一的包含头文件的方法


#ifndef __INCLUDEOPENGLES_H__
#define __INCLUDEOPENGLES_H__

//  OpenGLES的视窗系统
#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GLES3/gl31.h>

#endif  //  __INCLUDEOPENGLES_H__


