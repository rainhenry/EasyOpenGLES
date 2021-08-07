/**********************************************************************

    程序名称：简单的OpenGL/ES应用类
    程序版本：REV 0.9
    设计编写：rainhenry
    创建日期：20201102

    版本修订：
        REV 0.1  20201102  rainhenry   创建文档
        REV 0.2  20201113  rainhenry   增加载入纹理图片数据时，对纹理数据外框增加指定像素的透明边缘
                                       增加载入纯RAW纹理数据(用于投射屏幕等)
                                       增加FFmpeg视频解码
                                       增加对每个物体对象的显示/不显示操作
                                       增加FreeType2字体引擎支持
                                       增加2D相关操作的保持像素和保持尺寸的功能
                                       增加2D传统仅xy坐标操作功能
                                       分离视窗系统的实现部分，与OpenGLES功能分离
        REV 0.3  20201120  rainhenry   增加不同重置ffmpeg的方法,用于循环播放
                                       增加检查视窗系统是否初始化成功
        REV 0.4  20201126  rainhenry   修复线程异常退出后导致的控制标志无法清除问题
        REV 0.5  20201205  rainhenry   增加对OpenGLES 2.0和3.0的Shader纹理兼容开关
        REV 0.6  20201206  rainhenry   增加调试图层,用于标记2D图片的边框和旋转中心
                                       并且增加手动动态调节坐标功能
                                       增加自定义背景色
                                       将Shader版本定义统一放到H文件中的宏定义
                                       修复3D的正面剔除,改为人眼视角的背面剔除
                                       为了让2D和3D混合到同一个立体坐标系中,关闭了2D的面剔除
                                       修复文字显示会有阴影问题
        REV 0.7  20201210  rainhenry   修复当使用调试功能时,被调试对象发生重新绘制的时候,
                                       引起的崩溃问题,原因是由于终端调试类中的辅助线对象
                                       为保护成员,即使指针被传送出去,其他类也是无法访问的,
                                       所以将其改为共有成员
                                       增加调试命令行对物体透明度的修改
        REV 0.8  20201214  rainhenry   将Shader程序与本程序独立开,使用Shader2C工具进行合成
                                       到ShaderSrc.c文件中进行加载
                                       并且为每个由shader基类派生的子类,都增加自己类名字的
                                       定义,方便排查shader错误问题的定位
                                       取消Shader版本字符串定义
                                       取消Shader纹理函数兼容定义
                                       增加Shader的二进制格式的保存和载入功能
                                       这里生成的sdb文件为Shader的二进制程序文件
                                       然后生成的sdf文件为格式号码的ASCII文件,用于识别格式
                                       修复极低概率反复切换图片纹理资源造成的崩溃问题
                                       解决方法为合并刷新和渲染操作为一个互斥锁中完成
        REV 0.9  20201215  rainhenry   尝试增加多重采样相关实现抗锯齿


    设计说明
        采用基于OpenGL/ES的简单2D和3D绘图的运行框架，整体由框架类、
    视窗类、着色器类、操作对象类等构成。其中操作对象类可以派生出 3D
    骨骼绘图、3D纹理绘图、2D纹理绘图等。
        在程序启动后，会自动开启一个线程用于图形渲染操作，同时提供
    给用户一些控制该绘图线程的API，比如查询当前fps等。可以停止渲染
    等。并且对用户接口函数均支持多线程的数据保护措施，比如互斥锁等。
        多线程数据保护的操作均在  着色器类  中定义和使用，因为它是
    在渲染过程中主要对外的接口。当然框架类中有一部分控制渲染过程的
    也需要进行数据保护操作。然后着色器类和视窗系统类都是直接独立访
    问于线程中的。它们仅仅在渲染未开始之前由用户初始化。
        这里需要特别说明一下着色器类中的纹理数据，因为它的使用是在
    渲染线程中调用的，是申请内存、填充数据 和 释放内存都是在渲染的
    进行过程中，由用户进行调用的。虽然内存实体存在于操作对象类中，
    但是为了让代码更加统一，所以它的互斥也放到了着色器类中。

    关于FFmpeg的支持
        本代码在ffmpeg V3.4进行测试通过。并且要求编译成共享库的形式
    即在编译ffmpeg的时候，需要执行 ./configure --enable-shared 

    关于FreeType2的支持
        本代码在freetype2 v2.10.4版本进行测试通过。

    关于控制台的坐标实时调试
        当启动调试功能以后,它可以根据用户的输入按键来实时动态调节坐标
    位置等信息,并且在控制台打印出每次调整后的数值.相关按键为:
        w a s d     为平移位置的方向控制键
        q r         为左右旋转的控制键
        z x         为放大缩小的控制键
        i j k l     为移动物体旋转中心的控制键
        < >         (不按shift的时候也是),为粗调细调的控制
        0 o         键为全部参数归零重置
        p           键为打印当前全部位置坐标几何信息
        t           键用于显示隐藏辅助线
        r           建用于控制被调试对象本身的显示和隐藏
        [ ]         用于改变被调试对象的透明度

**********************************************************************/
//---------------------------------------------------------------------
//  重定义保护
#ifndef __EASYGL_H__
#define __EASYGL_H__

//---------------------------------------------------------------------
//  包含头文件
//  标准系统头文件
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <termios.h>

//  图片解码
#include "stb_image.h"

//  OpenGLES的视窗系统
#include "IncludeOpenGLES.h"

//---------------------------------------------------------------------
//  相关宏定义
#define EASYGL_INFOLOG_LEN                 2048            //  Shader编译信息存储空间大小
#define EASYGL_GEOMETRY_LINE_TYPE          GL_LINE_LOOP    //  骨骼填充的线的类型

//  Shader二进制文件所在路径
#define EASYGL_SHADER_BINARY_PATH          ""

//  调试平面 和 实际显示屏幕 的最小Z轴间距
#define EASYGL_DEBUG_Z_MIN                 (0.001f)

//  背景色
#define EASYGL_BACKGOUND_COLOR_R           (0.0f)
#define EASYGL_BACKGOUND_COLOR_G           (0.0f)
#define EASYGL_BACKGOUND_COLOR_B           (0.0f)
#define EASYGL_BACKGOUND_COLOR_A           (1.0f)

//  是否开启FFmpeg视频解码
#define EN_FFMPEG                          1

//  是否开启FreeType2字体引擎
#define EN_FREETYPE2                       1

//  FFmpeg视频解码
#if EN_FFMPEG
#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
}
#endif  //  __cplusplus
#endif  //  EN_FFMPEG

//  FreeType2字体引擎
#if EN_FREETYPE2
#include <ft2build.h>
#include FT_FREETYPE_H
#include <wchar.h>
#endif  //  EN_FREETYPE2

//=============================================================接口定义
//---------------------------------------------------------------------
//  定义视窗系统类的接口类，实际根据不同的硬件平台进行移植
class CEasyGL_WindowSystemBase
{
//  公有成员
public:
    //  视窗系统的初始化
    //  返回0表示初始化成功,其他值表示失败
    virtual int WindowSystemInit(void) = 0;
  
    //  释放视窗系统
    virtual void WindowSystemRelease(void) = 0;

    //  渲染开始
    virtual void RenderBegin(void) = 0;

    //  渲染结束
    virtual void RenderEnd(void) = 0;

    //  关闭窗口
    virtual void CloseWindows(void) = 0;

    //  当前视窗系统是否可用,用于当用户销毁视窗系统时进行判断退出
    virtual bool IsValid(void) = 0;

    //  当初始化完成后，视窗系统准备好
    virtual bool IsReady(void) = 0;
};


//---------------------------------------------------------------------
//  着色器接口类
class CEasyGL_ShaderBase
{
//  公有成员
public:
    //  定义纹理数据对象
    typedef struct
    {
        GLint format;    //  纹理图片格式
        int width;       //  宽度
        int height;      //  高度
        void* pdata;     //  纹理数据首地址
    }STextureInfo;

    //  初始化着色器，含载入、编译、链接等
    //  成功后返回着色器程序句柄，失败返回0
    virtual GLint ShaderInit(void) = 0;

    //  着色器是否可用
    virtual bool IsValid(void) = 0;

    //  着色器渲染,由操作对象调用，不可用由框架直接调用
    virtual void ShaderRender(void) = 0;

    //  设置顶点原始数据
    virtual void SetVertex(GLfloat* pv,      //  顶点数据首地址
                           int len           //  总长度，单位为float的个数
                          ) = 0;

    //  可选的设置顶点的平面元素索引数据
    virtual void SetVertexElements(GLuint* pdata,        //  索引数据首地址
                                   int len               //  索引数据长度，以数值个数为单位
                                  ) = 0;

    //  平移
    virtual void Move(float x, float y, float z) = 0;

    //  移动物体的中心点
    virtual void MoveCenter(float x, float y, float z) = 0;

    //  以角度旋转
    virtual void RotateDeg(float x, float y, float z) = 0;

    //  以弧度旋转
    virtual void RotateRad(float x, float y, float z) = 0;

    //  缩放
    virtual void Scale(float x, float y, float z) = 0;

    //  设置混合颜色
    virtual void SetBlendColor(float r, float g, float b, float a) = 0;

    //  指定纹理
    virtual void SetTexture2D(GLint imageformat,           //  纹理图片格式
                              int width,                   //  宽度
                              int height,                  //  高度
                              void* pdata                  //  纹理数据首地址
                             ) = 0;

    //  当前类的名字
    virtual std::string ClassName(void) = 0;
};

//---------------------------------------------------------------------
//  操作对象接口类
class CEasyGL_OperationObjectBase
{
//  公有成员
public:
    //  可选基本初始化
    //  成功返回0，失败返回非0
    virtual int Init(CEasyGL_ShaderBase* ps) = 0;

    //  编译着色器
    virtual int CompileShader(void) = 0; 

    //  刷新(纹理、坐标数据)
    virtual void Refresh(void) = 0;

    //  渲染操作
    virtual void Render(void) = 0;

    //  设置显示状态
    virtual void Show(bool status) = 0;

    //  获取当前显示状态
    virtual bool GetShowStatus(void) = 0;

    //  平移
    virtual void Move(float x, float y, float z) = 0;

    //  移动物体的中心点
    virtual void MoveCenter(float x, float y, float z) = 0;

    //  以角度旋转
    virtual void RotateDeg(float x, float y, float z) = 0;

    //  以弧度旋转
    virtual void RotateRad(float x, float y, float z) = 0;

    //  缩放
    virtual void Scale(float x, float y, float z) = 0;

    //  获取当前对象的Z轴位置(调试用途)
    virtual float GetZ(void) = 0;
};

//---------------------------------------------------------------------
//  框架接口类
class CEasyGL_PlatformBase
{
//  公有成员
public:
    //  添加操作对象
    virtual int AddOperationObject(CEasyGL_OperationObjectBase* poo) = 0;
    
    //  框架初始化，初始化时，需要指定视窗系统
    //  成功返回0，失败返回非0
    virtual int PlatformInit(CEasyGL_WindowSystemBase* pws) = 0;

    //  开始渲染线程
    virtual int StartRenderThread(void) = 0;

    //  结束渲染线程
    virtual int StopRenderThread(void) = 0;

    //  阻塞等待渲染结束,用于退出时等待，并释放资源前的判断
    virtual void WaitForRenderEnd(void) = 0;

    //  获取线程运行状态
    virtual bool IsRun(void) = 0;

    //  获取当前渲染帧率
    virtual float GetFPS(void) = 0;

    //  线程主循环
    virtual void* thread_run(void* arg) = 0;
};


//=============================================================实体定义
//---------------------------------------------------------------------
//  纯几何骨架填充着色器
//  绘制数据为纯x y z的点信息
//  提供平移、三维旋转、缩放、骨骼颜色API
class CEasyGL_OnlyGeometryShader:public CEasyGL_ShaderBase
{
//  公有成员
public:
    //  构造函数
    CEasyGL_OnlyGeometryShader();

    //  析构函数
    ~CEasyGL_OnlyGeometryShader();

    //  初始化着色器，含载入、编译、链接等
    //  成功后返回着色器程序句柄，失败返回0
    virtual GLint ShaderInit(void);

    //  着色器是否可用
    virtual bool IsValid(void);

    //  着色器渲染,由操作对象调用，不可用由框架直接调用
    virtual void ShaderRender(void);

    //  设置顶点原始数据
    virtual void SetVertex(GLfloat* pv,      //  顶点数据首地址
                           int len           //  总长度，单位为float的个数
                          );

    //  可选的设置顶点的平面元素索引数据
    virtual void SetVertexElements(GLuint* pdata,        //  索引数据首地址
                                   int len               //  索引数据长度，以数值个数为单位
                                  );

    //  平移
    virtual void Move(float x, float y, float z);

    //  移动物体的中心点
    virtual void MoveCenter(float x, float y, float z);

    //  以角度旋转
    virtual void RotateDeg(float x, float y, float z);

    //  以弧度旋转
    virtual void RotateRad(float x, float y, float z);

    //  缩放
    virtual void Scale(float x, float y, float z);

    //  设置混合颜色
    virtual void SetBlendColor(float r, float g, float b, float a);

    //  指定纹理
    virtual void SetTexture2D(GLint imageformat,           //  纹理图片格式
                              int width,                   //  宽度
                              int height,                  //  高度
                              void* pdata                  //  纹理数据首地址
                             );

    //  当前类的名字
    virtual std::string ClassName(void);

//  保护成员
protected:
    //  获取顶点着色器源码
    virtual const GLchar* GetVertexShaderSrc(void);

    //  片段着色器源码
    virtual const GLchar* GetFragmentShaderSrc(void);

    //  着色器句柄
    GLuint ShaderHandle;

    //  着色器是否可用
    bool shader_valid;

    //  顶点数据
    GLfloat* pvertex;
    int vertex_len;
    pthread_mutex_t vertex_mutex;

    //  平移参数
    float move_x;
    float move_y;
    float move_z;
    pthread_mutex_t move_mutex;

    //  移动中心参数
    float move_center_x;
    float move_center_y;
    float move_center_z;
    pthread_mutex_t move_center_mutex;

    //  旋转参数，弧度单位
    float rotate_rad_x;
    float rotate_rad_y;
    float rotate_rad_z;
    pthread_mutex_t rotate_mutex;

    //  缩放参数
    float scale_x;
    float scale_y;
    float scale_z;
    pthread_mutex_t scale_mutex;

    //  混合颜色
    float blendcolor_r;
    float blendcolor_g;
    float blendcolor_b;
    float blendcolor_a;
    pthread_mutex_t blendcolor_mutex;

//  私有成员
private:
};


//---------------------------------------------------------------------
//  纯几何骨架填充着色器,以元素进行绘图
//  绘制数据为纯x y z的点信息
//  提供平移、三维旋转、缩放、骨骼颜色API
class CEasyGL_OnlyGeometryElementShader:public CEasyGL_OnlyGeometryShader
{
//  公有成员
public:
    //  构造函数
    CEasyGL_OnlyGeometryElementShader();

    //  析构函数
    ~CEasyGL_OnlyGeometryElementShader();

    //  着色器渲染,由操作对象调用，不可用由框架直接调用
    virtual void ShaderRender(void);

    //  可选的设置顶点的平面元素索引数据
    virtual void SetVertexElements(GLuint* pdata,        //  索引数据首地址
                                   int len               //  索引数据长度，以数值个数为单位
                                  );

    //  当前类的名字
    virtual std::string ClassName(void);

//  保护成员
protected:
    //  平面元素索引数据
    GLuint* p_vertex_element;
    int vertex_element_len;

//  私有成员
private:
};

//---------------------------------------------------------------------
//  无光照效果的3D纹理填充着色器
//  绘制数据为x y z的点信息 和 u v纹理坐标信息
//  提供平移、三维旋转、缩放、整体混合纯色的API
class CEasyGL_3DTextureShader:public CEasyGL_OnlyGeometryShader
{
//  公有成员
public:
    //  构造函数
    CEasyGL_3DTextureShader();

    //  析构函数
    ~CEasyGL_3DTextureShader();

    //  着色器渲染,由操作对象调用，不可用由框架直接调用
    virtual void ShaderRender(void);

    //  指定纹理
    virtual void SetTexture2D(GLint imageformat,           //  纹理图片格式
                              int width,                   //  宽度
                              int height,                  //  高度
                              void* pdata                  //  纹理数据首地址
                             );

    //  当前类的名字
    virtual std::string ClassName(void);

//  保护成员
protected:
    //  获取顶点着色器源码
    virtual const GLchar* GetVertexShaderSrc(void);

    //  片段着色器源码
    virtual const GLchar* GetFragmentShaderSrc(void);

    //  纹理缓冲区
    STextureInfo TextureBuff;

    //  纹理操作互斥量
    pthread_mutex_t texture_mutex;

    //  纹理句柄
    GLuint texture_handle;

//  私有成员
private:
};

//---------------------------------------------------------------------
//  无光照效果的2D纹理填充着色器
//  绘制数据为x y z的点信息 和 u v纹理坐标信息
//  提供平移、三维旋转、缩放、整体混合纯色的API
class CEasyGL_2DTextureShader:public CEasyGL_3DTextureShader
{
//  公有成员
public:
    //  着色器渲染,由操作对象调用，不可用由框架直接调用
    virtual void ShaderRender(void);

    //  当前类的名字
    virtual std::string ClassName(void);
};

//---------------------------------------------------------------------
//  无光照效果的3D纹理填充着色器(压缩)
//  绘制数据为x y z的点信息 和 u v纹理坐标信息
//  提供平移、三维旋转、缩放、整体混合纯色的API
class CEasyGL_3DTextureElementShader:public CEasyGL_3DTextureShader
{
//  公有成员
public:
    //  构造函数
    CEasyGL_3DTextureElementShader();

    //  析构函数
    ~CEasyGL_3DTextureElementShader();

    //  着色器渲染,由操作对象调用，不可用由框架直接调用
    virtual void ShaderRender(void);

    //  可选的设置顶点的平面元素索引数据
    virtual void SetVertexElements(GLuint* pdata,        //  索引数据首地址
                                   int len               //  索引数据长度，以数值个数为单位
                                  );

    //  当前类的名字
    virtual std::string ClassName(void);

//  保护成员
protected:
    //  平面元素索引数据
    GLuint* p_vertex_element;
    int vertex_element_len;

//  私有成员
private:
};

//---------------------------------------------------------------------
//  无光照效果的3D纹理填充着色器(压缩)
//  绘制数据为x y z的点信息 和 u v纹理坐标信息
//  提供平移、三维旋转、缩放、整体混合纯色的API
class CEasyGL_2DTextureElementShader:public CEasyGL_3DTextureElementShader
{
//  公有成员
public:
    //  着色器渲染,由操作对象调用，不可用由框架直接调用
    virtual void ShaderRender(void);

    //  当前类的名字
    virtual std::string ClassName(void);
};

//---------------------------------------------------------------------
//  无光照效果的3D纹理填充多重采样着色器(压缩)
class CEasyGL_2DTextureMultiSampleElementShader:public CEasyGL_2DTextureElementShader
{
//  公有成员
public:
    //  构造函数
    CEasyGL_2DTextureMultiSampleElementShader();

    //  析构函数
    ~CEasyGL_2DTextureMultiSampleElementShader();

    //  着色器渲染,由操作对象调用，不可用由框架直接调用
    virtual void ShaderRender(void);

    //  当前类的名字
    virtual std::string ClassName(void);

//  保护成员
protected:
    //  片段着色器源码
    virtual const GLchar* GetFragmentShaderSrc(void);
};

//---------------------------------------------------------------------
//  纯顶点骨架的非压缩类，即采用Draw方法绘图
class CEasyGL_3DGeometryObject:public CEasyGL_OperationObjectBase
{
//  公有成员
public:
    //  构造函数
    CEasyGL_3DGeometryObject();

    //  析构函数
    ~CEasyGL_3DGeometryObject();

    //  可选基本初始化
    //  成功返回0，失败返回非0
    virtual int Init(CEasyGL_ShaderBase* ps);

    //  编译着色器
    virtual int CompileShader(void); 

    //  配置顶点信息
    //  成功返回0，失败返回非0
    virtual int ConfigVertex(GLfloat* pv,      //  顶点数据首地址
                             int len           //  总长度，单位为float的个数
                            );

    //  渲染操作
    virtual void Render(void);

    //  可选基本释放
    virtual void Release(void);

    //  平移
    virtual void Move(float x, float y, float z);

    //  移动物体的中心点
    virtual void MoveCenter(float x, float y, float z);

    //  以角度旋转
    virtual void RotateDeg(float x, float y, float z);

    //  以弧度旋转
    virtual void RotateRad(float x, float y, float z);

    //  缩放
    virtual void Scale(float x, float y, float z);

    //  设置骨骼颜色
    virtual void SetColor(float r, float g, float b, float a);

    //  刷新(纹理、坐标数据等)
    virtual void Refresh(void);

    //  设置显示状态
    virtual void Show(bool status);

    //  获取当前显示状态
    virtual bool GetShowStatus(void);

    //  获取当前对象的Z轴位置(调试用途)
    virtual float GetZ(void);

//  保护成员
protected:
    //  着色器对象
    CEasyGL_ShaderBase* pshader;

    //  顶点信息
    GLfloat* pvertex;
    int vertex_len;

    //  平移数据
    float move_x;
    float move_y;
    float move_z;

    //  平移中心点数据
    float move_center_x;
    float move_center_y;
    float move_center_z;

    //  弧度旋转数据
    float rotate_rad_x;
    float rotate_rad_y;
    float rotate_rad_z;

    //  缩放数据
    float scale_x;
    float scale_y;
    float scale_z;

    //  骨骼颜色数据
    float color_r;
    float color_g;
    float color_b;
    float color_a;

    //  数据操作互斥量
    pthread_mutex_t data_mutex;

    //  显示状态
    bool show_flag;
    pthread_mutex_t show_mutex;

//  私有成员
private:
};

//---------------------------------------------------------------------
//  纯顶点骨架的压缩类，即采用Elements方法绘图
class CEasyGL_3DGeometryElementObject:public CEasyGL_3DGeometryObject
{
//  公有成员
public:
    //  构造函数
    CEasyGL_3DGeometryElementObject();

    //  析构函数
    ~CEasyGL_3DGeometryElementObject();

    //  可选的设置顶点的平面元素索引数据
    virtual void ConfigVertexElements(GLuint* pdata,        //  索引数据首地址
                                      int len               //  索引数据长度，以数值个数为单位
                                     );

    //  刷新(纹理、坐标数据等)
    virtual void Refresh(void);

//  保护成员
protected:
    //  元素信息
    GLuint* pvertex_element;
    int vertex_element_len;

//  私有成员
private:
};


//---------------------------------------------------------------------
//  3D纹理的非压缩类，即采用Draw方法绘图
class CEasyGL_3DTextureObject:public CEasyGL_3DGeometryObject
{
//  公有成员
public:
    //  构造函数
    CEasyGL_3DTextureObject();

    //  析构函数
    ~CEasyGL_3DTextureObject();

    //  从图片文件载入纹理数据,支持渲染过程中的动态载入
    //  成功返回0  失败返回非0
    virtual int LoadTextureFromImageFile(std::string filename);

    //  刷新(纹理、坐标数据等)
    virtual void Refresh(void);

//  保护成员
protected:
    //  当前纹理数据对象
    CEasyGL_ShaderBase::STextureInfo texture_info;

//  私有成员
private:

};

//---------------------------------------------------------------------
//  2D纹理贴图的非压缩类，即采用Draw方法绘图
class CEasyGL_2DImageObject:public CEasyGL_3DTextureObject
{
//  公有成员
public:
    //  尺寸计算模式
    typedef enum
    {
        ESizeCalcMode_KeepSize = 0,           //  保持单位尺寸比例
        ESizeCalcMode_KeepPixel,              //  保持像素比例
    }ESizeCalcMode;

    //  构造函数
    CEasyGL_2DImageObject();

    //  析构函数
    ~CEasyGL_2DImageObject();

    //  载入显示图片,根据图片自动计算显示比例
    //  成功返回0  失败返回非0
    virtual int LoadImage(std::string filename, 
                          ESizeCalcMode mode,
                          float pixel             //  当像素比例时，每个单位的像素值
                         );

    //  2D平移
    virtual void Move2D(float x, float y);

    //  2D移动物体的中心点
    virtual void MoveCenter2D(float x, float y);

    //  配置调试
    virtual void ConfigDebug(CEasyGL_2DImageObject* pobj);

    //  平移
    virtual void Move(float x, float y, float z);

    //  移动物体的中心点
    virtual void MoveCenter(float x, float y, float z);

    //  以角度旋转
    virtual void RotateDeg(float x, float y, float z);

    //  以弧度旋转
    virtual void RotateRad(float x, float y, float z);

    //  缩放
    virtual void Scale(float x, float y, float z);

    //  设置长宽尺寸(调试用的浮点值)
    virtual void SetWidthHighFloat(float x, float y);

//  保护成员
protected:
    //  矩形顶点UV信息
    GLfloat rect_v[20];

    //  矩形元素信息
    GLuint rect_v_index[6];

    //  自己的Shader
    static CEasyGL_2DTextureElementShader my_shader;

    //  刷新(纹理、坐标数据等)
    virtual void Refresh(void);

    //  调试对象指针
    CEasyGL_2DImageObject* pDebugObj;

//  私有成员
private:
};

//---------------------------------------------------------------------
//  2D纹理贴图的非压缩类，增加Alpha边缘扩充，即采用Draw方法绘图
class CEasyGL_2DImageAlphaEdgeObject:public CEasyGL_2DImageObject
{
//  公有成员
public:
    //  构造函数
    CEasyGL_2DImageAlphaEdgeObject();

    //  析构函数
    ~CEasyGL_2DImageAlphaEdgeObject();

    //  载入显示图片,根据图片自动计算显示比例
    //  参数a_size 表示增加外框边缘的alpha的像素个数
    //  成功返回0  失败返回非0
    virtual int LoadImageAlphaEdge(std::string filename, 
                                   int a_size, 
                                   ESizeCalcMode mode,
                                   float pixel             //  当像素比例时，每个单位的像素值
                                  );

//  保护成员
protected:
    //  当前纹理alpha边沿的数据对象
    CEasyGL_ShaderBase::STextureInfo texture_alpha_edge_info;

    //  刷新(纹理、坐标数据等)
    virtual void Refresh(void);

//  私有成员
private:
};

//---------------------------------------------------------------------
//  2D纹理贴图的非压缩类，增加Alpha边缘裁剪，即采用Draw方法绘图
class CEasyGL_2DImageAlphaCutObject:public CEasyGL_2DImageObject
{
//  公有成员
public:
    //  构造函数
    CEasyGL_2DImageAlphaCutObject();

    //  析构函数
    ~CEasyGL_2DImageAlphaCutObject();

    //  载入显示图片,根据图片自动计算显示比例(仅仅支持4通道带有alpha的图片)
    //  参数a_size 表示外框边缘裁剪的alpha的像素个数
    //  成功返回0  失败返回非0
    virtual int LoadImageAlphaCut(std::string filename, 
                                  int a_size, 
                                  ESizeCalcMode mode,
                                  float pixel             //  当像素比例时，每个单位的像素值
                                 );

//  保护成员
protected:

//  私有成员
private:
};

//---------------------------------------------------------------------
//  2D高速高效率纹理贴图，用于RAW的RGB视频播放，即采用Draw方法绘图
class CEasyGL_2DEfficientImageObject:public CEasyGL_2DImageObject
{
//  公有成员
public:
    //  构造函数
    CEasyGL_2DEfficientImageObject();

    //  析构函数
    ~CEasyGL_2DEfficientImageObject();

    //  创建缓冲区
    //  x和y为视频尺寸
    //  成功返回0  失败返回非0
    virtual int CreateBuffer(int x, int y,
                             ESizeCalcMode mode,
                             float pixel             //  当像素比例时，每个单位的像素值
                            );

    //  更新画面
    //  成功返回0  失败返回非0
    virtual int UpdateImage(void* pdat);

//  保护成员
protected:
    //  显示缓冲0和1
    CEasyGL_ShaderBase::STextureInfo texture_efficient_info0;
    CEasyGL_ShaderBase::STextureInfo texture_efficient_info1;

    //  当前使用的缓冲
    int current_buffer_index;

    //  刷新(纹理、坐标数据等)
    virtual void Refresh(void);

//  私有成员
private:
};

//---------------------------------------------------------------------
//  2D纹理贴图的多重采样非压缩类，即采用Draw方法绘图
class CEasyGL_2DImageMultiSampleObject:public CEasyGL_2DImageObject
{
//  公有成员
public:
    //  构造函数
    CEasyGL_2DImageMultiSampleObject();

//  保护成员
protected:
    //  多重采样的Shader
    static CEasyGL_2DTextureMultiSampleElementShader multisample_shader;

    //  刷新(纹理、坐标数据等)
    virtual void Refresh(void);

//  私有成员
private:
};

//---------------------------------------------------------------------
//  简单的框架类
class CEasyGL_SimplePlatform:public CEasyGL_PlatformBase
{
//  公有成员
public:
    //  构造函数
    CEasyGL_SimplePlatform();

    //  析构函数
    ~CEasyGL_SimplePlatform();

    //  添加操作对象
    //  成功返回0，失败返回非0
    virtual int AddOperationObject(CEasyGL_OperationObjectBase* poo);
    
    //  框架初始化，初始化时，需要指定视窗系统
    //  成功返回0，失败返回非0
    virtual int PlatformInit(CEasyGL_WindowSystemBase* pws);

    //  开始渲染线程
    //  成功返回0，失败返回非0
    virtual int StartRenderThread(void);

    //  结束渲染线程
    //  成功返回0，失败返回非0
    virtual int StopRenderThread(void);

    //  阻塞等待渲染结束,用于退出时等待，并释放资源前的判断
    virtual void WaitForRenderEnd(void);

    //  获取当前渲染帧率
    virtual float GetFPS(void);

    //  获取线程运行状态
    virtual bool IsRun(void);

    //  线程主循环
    virtual void* thread_run(void* arg);

//  保护成员
protected:
    //  操作对象容器
    std::vector<CEasyGL_OperationObjectBase*> OptObjVec;

    //  线程对象
    pthread_t thread_handle;

    //  线程控制运行标志
    bool thread_ctrl_flag;

    //  线程运行标志
    bool thread_run_flag;

    //  相关线程控制标志的互斥量
    pthread_mutex_t thread_flag_mutex;

    //  当前FPS值
    float fps;
    pthread_mutex_t fps_mutex;

    //  视窗系统
    CEasyGL_WindowSystemBase* pWinSys;

//  私有成员
private:

};


//=========================================================视频解码相关
#if EN_FFMPEG

//---------------------------------------------------------------------
//  视频解码框架类
class CEasyGL_FFmpaePlatform
{
//  公有成员
public:
    //  播放模式
    typedef enum
    {
        EPlayMode_Once = 0,          //  单次播放，播放完成后停止
        EPlayMode_Cycle,             //  循环连续播放
    }EPlayMode;

    //  FFmpeg上下文数据结构
    typedef struct
    {
        //  相关控制信息
        AVFormatContext*    p_fmt_ctx;
        AVCodecContext*     p_codec_ctx; 
        AVCodecParameters*  p_codec_par;
        AVCodec*            p_codec;
        AVFrame*            p_frm_raw;
        AVPacket*           p_packet;
        int                 buf_size;
        unsigned char*      buffer;
        int                 v_idx;

        //  视频信息
        float               FrameRate;         //  帧率
        int                 Width;             //  宽度
        int                 Height;            //  高度
        unsigned long       TotalFrame;        //  该视频总共帧数量
        EPlayMode           play_mode;         //  播放模式
    }SFFmpegContext;

    //  播放状态
    typedef enum
    {
        EPlayerStatus_NoConf = 0,              //  未配置
        EPlayerStatus_Stop,                    //  停止播放
        EPlayerStatus_Playing,                 //  正在播放
    }EPlayerStatus;

    //  构造函数
    CEasyGL_FFmpaePlatform();

    //  析构函数
    ~CEasyGL_FFmpaePlatform();

    //  配置播放
    //  成功返回0，失败返回非0
    virtual int Config(std::string filename,                   //  播放视频文件
                       CEasyGL_2DEfficientImageObject* pobj,   //  高效贴图对象
                       CEasyGL_2DImageObject::ESizeCalcMode mode, //  显示比例模式
                       float pixel                             //  当像素比例时，每个单位的像素值
                      );

    //  关闭播放
    //  成功返回0，失败返回非0
    virtual int Close(void);

    //  开始播放线程
    //  参数 mode 为播放模式，默认循环播放
    //  成功返回0，失败返回非0
    virtual int StartPlayerThread(EPlayMode mode = EPlayMode_Cycle);

    //  结束播放线程
    //  成功返回0，失败返回非0
    virtual int StopPlayerThread(void);

    //  获取线程运行状态
    virtual bool IsRun(void);

    //  线程主循环
    virtual void* thread_run(void* arg);

    //  获取播放状态
    virtual EPlayerStatus GetPlayStatus(void);

//  保护成员
protected:
    //  线程对象
    pthread_t thread_handle;

    //  线程控制运行标志
    bool thread_ctrl_flag;

    //  线程运行标志
    bool thread_run_flag;

    //  相关线程控制标志的互斥量
    pthread_mutex_t thread_flag_mutex;

    //  贴图对象
    CEasyGL_2DEfficientImageObject* pObject;

    //  FFmpeg上下文
    SFFmpegContext ffmpeg_context;

    //  解码器可用
    bool DecodeIsValid;

    //  图像转换
    void YUV2RGB(int in_width,
                 int in_height,
                 unsigned char** in_pdat_yuv,
                 int* in_pline_size,
                 unsigned char* out_pdat_rgb
                );

    //  播放状态
    EPlayerStatus play_status;
    pthread_mutex_t play_status_mutex;

    //  设置播放状态
    virtual void SetPlayStatus(EPlayerStatus st);

    //  显示比例相关
    CEasyGL_2DImageObject::ESizeCalcMode scale_mode;
    float scale_pixel;

    //  重置ffmpeg,用于循环播放
    virtual void ResetFFmpeg(void);

    //  视频文件名
    std::string video_filename;

//  私有成员
private:
};

#endif  //  EN_FFMPEG


//================================================FreeType2字体引擎相关
#if EN_FREETYPE2

//---------------------------------------------------------------------
//  2D文字类，即采用Draw方法绘图
class CEasyGL_2DTextObject:public CEasyGL_2DImageObject
{
//  公有成员
public:
    //  文字信息上下文数据类型
    typedef struct
    {
        //  字体引擎相关
        FT_Library        library;
        FT_Face           face;
        FT_GlyphSlot      slot;
        FT_Matrix         matrix;
        FT_Vector         pen;

        //  绘制相关
        double            angle_deg;       //  单个字符的旋转角度
        int               font_width;      //  单个字符的宽度
        int               font_height;     //  单个字符的高度
        int               frame_width;     //  整个字符串外框的留边宽度
        int               span;            //  每个文字的间距空隙
    }STextContext;

    //  构造函数
    CEasyGL_2DTextObject();

    //  析构函数
    ~CEasyGL_2DTextObject();

    //  配置文字信息
    //  成功返回0  失败返回非0
    virtual int Config(std::string filename,            //  TTF字体文件
                       int font_width,                  //  单个字符的宽度
                       int font_height,                 //  单个字符的高度
                       int frame_width,                 //  整个字符串外框的留边宽度
                       int span,                        //  每个文字的间距空隙
                       double angle_deg                 //  单个字符的旋转角度
                      );

    //  绘制文字
    //  成功返回0  失败返回非0
    virtual int DrawText(const wchar_t* pstr,           //  基于UNICODE编码的文字
                         int r, int g, int b, int a,    //  绘制文字时的颜色
                         ESizeCalcMode mode,            //  显示比例模式
                         float pixel                    //  当像素比例时，每个单位的像素值
                        );

//  保护成员
protected:
    //  当前文字纹理的绘制缓冲区的数据对象
    CEasyGL_ShaderBase::STextureInfo texture_text_info;

    //  当前文字上下文信息
    STextContext text_context;

    //  文字可用
    bool text_valid;

    //  刷新(纹理、坐标数据等)
    virtual void Refresh(void);

//  私有成员
private:
};

#endif  //  EN_FREETYPE2

//=========================================================调试图层相关
//---------------------------------------------------------------------
//  定义调试图层类
class EasyGL_2DDebugObj:public CEasyGL_2DImageObject
{
//  公有成员
public:
    //  构造函数
    EasyGL_2DDebugObj();

    //  析构函数
    ~EasyGL_2DDebugObj();

    //  刷新(纹理、坐标数据等)
    virtual void Refresh(void);

    //  设置长宽尺寸(调试用的浮点值)
    virtual void SetWidthHighFloat(float x, float y);

//  保护成员
protected:
    //  矩形顶点UV信息
    GLfloat rect_v_debug[4*3];

    //  矩形元素信息
    GLuint rect_v_index_debug[3*3];

    //  自己的Shader
    static CEasyGL_OnlyGeometryElementShader my_geometry_shader;

//  私有成员
private:
};

//---------------------------------------------------------------------
//  控制台位置调试类
class EasyGL_ConsolePositionDebug:public CEasyGL_OperationObjectBase
{
//  公有成员
public:
    //  定义精度种类
    typedef enum
    {
        EPrecisionType_0_1 = 0,         //  0.1
        EPrecisionType_0_01,            //  0.01
        EPrecisionType_0_001,           //  0.001
        EPrecisionType_0_0001,          //  0.0001
    }EPrecisionType;

    //  构造函数
    EasyGL_ConsolePositionDebug();

    //  析构函数
    ~EasyGL_ConsolePositionDebug();

    //  配置被调试的物体
    virtual void ConfigDebug(CEasyGL_2DImageObject* pobj,
                             float aux_line_color_r,
                             float aux_line_color_g,
                             float aux_line_color_b,
                             float aux_line_color_a,
                             bool aux_line_show = true
                            );

    //  获取辅助线对象
    virtual CEasyGL_2DImageObject* GetAuxLineObject(void);

    //  开始线程
    //  成功返回0，失败返回非0
    virtual int StartThread(void);

    //  结束线程
    //  成功返回0，失败返回非0
    virtual int StopThread(void);

    //  阻塞等待渲染结束,用于退出时等待，并释放资源前的判断
    virtual void WaitForThreadEnd(void);

    //  获取线程运行状态
    virtual bool IsRun(void);

    //  线程主循环
    virtual void* thread_run(void* arg);

    //  为了实现直接对框架追加对象进行赋值
    //  采用基于操作对象基类实现
    //  可选基本初始化
    //  成功返回0，失败返回非0
    virtual int Init(CEasyGL_ShaderBase* ps);

    //  编译着色器
    virtual int CompileShader(void);

    //  刷新(纹理、坐标数据)
    virtual void Refresh(void);

    //  渲染操作
    virtual void Render(void);

    //  设置显示状态
    virtual void Show(bool status);

    //  获取当前显示状态
    virtual bool GetShowStatus(void);

    //  平移
    virtual void Move(float x, float y, float z);

    //  移动物体的中心点
    virtual void MoveCenter(float x, float y, float z);

    //  以角度旋转
    virtual void RotateDeg(float x, float y, float z);

    //  以弧度旋转
    virtual void RotateRad(float x, float y, float z);

    //  缩放
    virtual void Scale(float x, float y, float z);

    //  获取当前对象的Z轴位置(调试用途)
    virtual float GetZ(void);

    //  辅助线对象
    EasyGL_2DDebugObj AuxLineObject;

//  保护成员
protected:
    //  被调试物体的指针
    CEasyGL_2DImageObject* pObjDebug;

    //  线程对象
    pthread_t thread_handle;

    //  线程控制运行标志
    bool thread_ctrl_flag;

    //  线程运行标志
    bool thread_run_flag;

    //  相关线程控制标志的互斥量
    pthread_mutex_t thread_flag_mutex;

    //  初始坐标和角度相关
    float pos_x;
    float pos_y;
    float center_x;
    float center_y;
    float scale_x;
    float scale_y;
    float scale_z;
    float rotate_deg_z;
    float transparent;

    //  精度
    EPrecisionType precision;

    //  辅助线显示控制变量
    bool AuxLineShow;

    //  调试对象本身的显示控制变量
    bool DebugObjectShow;

    //  根据精度做自增
    virtual float add(float in);

    //  根据精度做自减
    virtual float sub(float in);

    //  更新平面部分
    virtual void Update2D(void);

//  私有成员
private:
};

//---------------------------------------------------------------------
#endif  //  __EASYGL_H__


