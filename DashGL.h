/**********************************************************************

    程序名称：基于EasyGL的表盘相关
    程序版本：REV 0.1
    设计编写：rainhenry
    创建日期：20201211

    版本修订：
        REV 0.1  20201211  rainhenry   创建文档

    设计说明
        基于EasyGL基本图像操作类,实现各种常见表盘动效的类和Shader的封装
    用户可以自行派生,编写各种效果的Shader,以实现不同的表盘效果.

**********************************************************************/
//---------------------------------------------------------------------
//  重定义保护
#ifndef __DASHGL_H__
#define __DASHGL_H__

//---------------------------------------------------------------------
//  包含头文件
#include "EasyGL.h"

//---------------------------------------------------------------------
//  相关宏定义

//=============================================================基本表盘
//---------------------------------------------------------------------
//  圆形表盘渐变拖尾着色器
class CDashGL_CircularGradientTrackShader:public CEasyGL_2DTextureElementShader
{
//  公有成员
public:
    //  构造函数
    CDashGL_CircularGradientTrackShader();

    //  析构函数
    ~CDashGL_CircularGradientTrackShader();

    //  着色器渲染,由操作对象调用，不可用由框架直接调用
    virtual void ShaderRender(void);

    //  设置轨迹拖尾
    virtual void SetTrack(float start, float track);

    //  当前类的名字
    virtual std::string ClassName(void);

//  保护成员
protected:
    //  获取顶点着色器源码
    virtual const GLchar* GetVertexShaderSrc(void);

    //  片段着色器源码
    virtual const GLchar* GetFragmentShaderSrc(void);

    //  轨迹拖尾
    float start_angle_deg;
    float track_angle_deg;
    pthread_mutex_t track_mutex;

//  私有成员
private:
};

//---------------------------------------------------------------------
//  圆形表盘实心拖尾着色器
class CDashGL_CircularSolidTrackShader:public CDashGL_CircularGradientTrackShader
{
//  公有成员
public:
    //  当前类的名字
    virtual std::string ClassName(void);

//  保护成员
protected:
    //  片段着色器源码
    virtual const GLchar* GetFragmentShaderSrc(void);

//  私有成员
private:
};

//---------------------------------------------------------------------
//  圆形表盘渐变拖尾对象
class CDashGL_CircularGradientTrackObject:public CEasyGL_2DImageObject
{
//  公有成员
public:
    //  构造函数
    CDashGL_CircularGradientTrackObject();

    //  设置轨迹拖尾
    virtual void SetTrack(float start, float track);

    //  刷新(纹理、坐标数据等)
    virtual void Refresh(void);

//  保护成员
protected:
    //  自己的Shader
    static CDashGL_CircularGradientTrackShader gradient_shader;

    //  轨迹拖尾
    float start_angle_deg;
    float track_angle_deg;

//  私有成员
private:

};

//---------------------------------------------------------------------
//  圆形表盘实心拖尾对象
class CDashGL_CircularSolidTrackObject:public CDashGL_CircularGradientTrackObject
{
//  公有成员
public:
    //  构造函数
    CDashGL_CircularSolidTrackObject();

    //  刷新(纹理、坐标数据等)
    virtual void Refresh(void);

//  保护成员
protected:
    //  自己的Shader
    static CDashGL_CircularSolidTrackShader solid_shader;

//  私有成员
private:

};



//---------------------------------------------------------------------
#endif  //  __DASHGL_H__


