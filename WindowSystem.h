/**********************************************************************

    程序名称：视窗系统实现
    程序版本：REV 0.1
    设计编写：rainhenry
    创建日期：20201116

    版本修订：
        REV 0.1  20201116  rainhenry   创建文档

    设计说明
        根据不同平台系统，实现不同的视窗系统

**********************************************************************/
//---------------------------------------------------------------------
//  重定义保护
#ifndef __WINDOWSYSTEM_H__
#define __WINDOWSYSTEM_H__

//---------------------------------------------------------------------
//  包含头文件
#include "EasyGL.h"

//---------------------------------------------------------------------
//  相关宏定义

//=============================================================实体定义
//---------------------------------------------------------------------
//  定义amd64 linux平台下的视窗系统接口，基于GLFW
class CEasyGL_AMD64_GLFW_WS: public CEasyGL_WindowSystemBase
{
//  公有成员
public:
    //  构造函数
    CEasyGL_AMD64_GLFW_WS();

    //  析构函数
    ~CEasyGL_AMD64_GLFW_WS();

    //  配置GLFW
    //  参数:width   创建窗口的宽度
    //  参数:height  创建窗口的高度
    //  参数:title   创建窗口的标题名称 
    virtual void Config(int width, int height, std::string title);

    //  视窗系统的初始化
    //  返回0表示初始化成功,其他值表示失败
    virtual int WindowSystemInit(void);
  
    //  释放视窗系统
    virtual void WindowSystemRelease(void);

    //  渲染开始
    virtual void RenderBegin(void);

    //  渲染结束
    virtual void RenderEnd(void);

    //  关闭窗口
    virtual void CloseWindows(void);

    //  当前视窗系统是否可用,用于当用户销毁视窗系统时进行判断退出
    virtual bool IsValid(void);

    //  当初始化完成后，视窗系统准备好
    virtual bool IsReady(void);

//  保护成员
protected:
    //  窗口高度
    int ws_width;
 
    //  窗口宽度
    int ws_height;

    //  窗口标题
    std::string ws_title;

    //  窗口句柄
    GLFWwindow* window_handle;

    //  视窗系统准备好标志
    bool is_ready_flag;
    pthread_mutex_t ready_mutex;

//  私有成员
private:
};


//---------------------------------------------------------------------
#endif  //  __WINDOWSYSTEM_H__


