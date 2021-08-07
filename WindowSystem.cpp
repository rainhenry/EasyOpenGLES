
//  程序版本  REV 0.1

//  包含头文件
#include "WindowSystem.h"

//---------------------------------------------------------------------
//  定义实体amd64 linux平台下的视窗系统接口，基于GLFW

//  构造函数
CEasyGL_AMD64_GLFW_WS::CEasyGL_AMD64_GLFW_WS()
{
    //  初始化成默认值
    this->ws_width = 1920;
    this->ws_height = 720;
    this->ws_title = "EasyGL Base OpenGLES";
    this->window_handle = 0;
    this->is_ready_flag = false;

    pthread_mutex_init(&this->ready_mutex, NULL);
}

//  析构函数
CEasyGL_AMD64_GLFW_WS::~CEasyGL_AMD64_GLFW_WS()
{
    pthread_mutex_destroy(&this->ready_mutex);
}

//  配置GLFW
//  参数:width   创建窗口的宽度
//  参数:height  创建窗口的高度
//  参数:title   创建窗口的标题名称 
void CEasyGL_AMD64_GLFW_WS::Config(int width, int height, std::string title)
{
    this->ws_width = width;
    this->ws_height = height;
    this->ws_title = title;
}

//  视窗系统的初始化
//  返回0表示初始化成功,其他值表示失败
int CEasyGL_AMD64_GLFW_WS::WindowSystemInit(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    this->window_handle = glfwCreateWindow(this->ws_width, 
                                           this->ws_height,
                                           this->ws_title.c_str(), 
                                           NULL, 
                                           NULL
                                          );
    glfwMakeContextCurrent(this->window_handle);

    //  设置视角
    if((this->ws_width) > (this->ws_height))
    {
        glViewport(0, -((this->ws_width-this->ws_height)/2), this->ws_width, this->ws_width);
    }
    else
    {
        glViewport(-((this->ws_height-this->ws_width)/2), 0, this->ws_height, this->ws_height);
    }

    //  初始化成功
    return 0;
}

//  释放视窗系统
void CEasyGL_AMD64_GLFW_WS::WindowSystemRelease(void)
{
    glfwTerminate();
}

//  渲染开始
void CEasyGL_AMD64_GLFW_WS::RenderBegin(void)
{
    glfwPollEvents();
    pthread_mutex_lock(&this->ready_mutex);
    this->is_ready_flag = true;
    pthread_mutex_unlock(&this->ready_mutex);
}

//  渲染结束
void CEasyGL_AMD64_GLFW_WS::RenderEnd(void)
{
    glfwSwapBuffers(this->window_handle);
}

//  关闭窗口
void CEasyGL_AMD64_GLFW_WS::CloseWindows(void)
{
    glfwTerminate();
}

//  当前视窗系统是否可用,用于当用户销毁视窗系统时进行判断退出
bool CEasyGL_AMD64_GLFW_WS::IsValid(void)
{
    if(!glfwWindowShouldClose(this->window_handle))  return true;
    else                                             return false;
}

//  当初始化完成后，视窗系统准备好
bool CEasyGL_AMD64_GLFW_WS::IsReady(void)
{
    bool re = false;
    pthread_mutex_lock(&this->ready_mutex);
    re = this->is_ready_flag;
    pthread_mutex_unlock(&this->ready_mutex);
    return re;
}


