
//  包含头文件
#include "EasyGL.h"
#include "WindowSystem.h"
#include "cube.h"
#include "teapot.h"
#include "monkey.h"

//  定义黑白窗口切换缓冲区
unsigned char b_buff[800*600*3];
unsigned char w_buff[800*600*3];

//  延时us
void linux_delay_us(long us)
{
    struct timeval last_time;           //  上一次渲染时间
    struct timeval now_time;            //  现在时间
    long timespan = 0L;

    //  获取当前时间
    gettimeofday(&last_time, NULL);

    do
    {
        //  释放CPU
        usleep(1);

        //  计算渲染时间
        gettimeofday(&now_time, NULL);
        if(now_time.tv_sec != last_time.tv_sec)
        {
            timespan = (now_time.tv_sec - last_time.tv_sec - 1L) * 1000000L;
            timespan += 1000000L - last_time.tv_usec;
            timespan += now_time.tv_usec;
        }
        else
        {
            timespan = now_time.tv_usec - last_time.tv_usec;
        }
    }
    while(timespan < us);
}


//  主函数
int main(int argc, char** argv)
{
    //  初始化黑白窗口的颜色
    memset(b_buff, 0x00, sizeof(b_buff));
    memset(w_buff, 0xFF, sizeof(w_buff));


    //  定义视窗系统类
    CEasyGL_AMD64_GLFW_WS glfw_ws;
    glfw_ws.Config(1920, 720, "EasyGL Base OpenGLES");

    //  定义着色器类
    CEasyGL_3DTextureShader texture3d_shader;    //  3D立体纹理着色器
    CEasyGL_OnlyGeometryShader geo3d_shader;     //  3D骨架着色器

    //  定义对象物体类
    //  带有纹理的立方体  大
    CEasyGL_3DTextureObject obj3d1;
    obj3d1.Init(&texture3d_shader);
    obj3d1.ConfigVertex((GLfloat*)cube_3d_vt_data, 180);
    obj3d1.LoadTextureFromImageFile("cube.png");
  
    //  骨骼显示的茶壶
    CEasyGL_3DGeometryObject obj3d2;
    obj3d2.Init(&geo3d_shader);
    obj3d2.ConfigVertex((GLfloat*)pteapot_v, 2256*3*3);

    //  2D图片 黑色笑脸
    CEasyGL_2DImageObject image1;
    image1.LoadImage("image1.jpg", CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);

    //  2D图片 粉色小姐姐头像
    CEasyGL_2DImageObject image2;
    image2.LoadImage("image2.jpg", CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);
    //  2D图片 皇后背景
    CEasyGL_2DImageObject image4;
    //image4.LoadImage("123.png", CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);
    image4.LoadImage("hh.jpg", CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);


    //  2D图片 黑色小姐姐头像
    //CEasyGL_2DImageObject image3;
    //image3.LoadImage("image3.jpg");
    //  2D图片 极品飞车
    CEasyGL_2DImageAlphaEdgeObject image3;
    image3.LoadImageAlphaEdge("car1.png", 20, CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);         //  追加
    //CEasyGL_2DImageAlphaCutObject image3;
    //image3.LoadImageAlphaCut("car1.png", 40);        //  裁剪


    //  3D骨骼显示的猴子头
    CEasyGL_3DGeometryObject obj3d3;
    obj3d3.Init(&geo3d_shader);
    obj3d3.ConfigVertex((GLfloat*)monkey_3d_v_data, sizeof(monkey_3d_v_data)/sizeof(GLfloat));

    //  2D图片 指针
    CEasyGL_2DImageObject image5;
    //CEasyGL_2DImageMultiSampleObject image5;
    image5.LoadImage("image4.png", CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);

    //  指针的调试
    EasyGL_2DDebugObj image5_debug;
    image5_debug.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    image5.ConfigDebug(&image5_debug);        //  绑定调试

    //  带有纹理的立方体  小
    CEasyGL_3DTextureObject obj3d4;
    obj3d4.Init(&texture3d_shader);
    obj3d4.ConfigVertex((GLfloat*)cube_3d_vt_data, 180);
    obj3d4.LoadTextureFromImageFile("cube.png");

    //  2D快速切换
    CEasyGL_2DEfficientImageObject efficient_image1;
    efficient_image1.CreateBuffer(800, 600, CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);

    //  2D视频画布
    CEasyGL_2DEfficientImageObject video1;
    CEasyGL_FFmpaePlatform ffmpeg1;
    ffmpeg1.Config("video1.mp4", &video1, CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);       //  微电影
    //ffmpeg1.Config("video2.mp4", &video1, CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);       //  超短视频，测试循环播放
    //ffmpeg1.Config("video3.avi", &video1, CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);       //  长电影
    //ffmpeg1.Config("video4.mp4", &video1, CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);       //  宣传短视频

    //  2D视频画布
    //CEasyGL_2DEfficientImageObject video2;
    //CEasyGL_FFmpaePlatform ffmpeg2;
    //ffmpeg2.Config("video1.mp4", &video2, CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);       //  微电影
    //ffmpeg2.Config("video2.mp4", &video2, CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);       //  超短视频，测试循环播放
    //ffmpeg2.Config("video3.avi", &video2, CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);       //  长电影
    //ffmpeg2.Config("video4.mp4", &video2, CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);       //  宣传短视频


    //  悬浮文字
    CEasyGL_2DTextObject text1;
    text1.Config("TTF/simhei.ttf", 200, 200, 30, 15, 0.0); 
    text1.DrawText(L"中文测试ABC./?#123愛してる", 255, 0, 0, 255, CEasyGL_2DImageObject::ESizeCalcMode_KeepPixel, 5000.0f);

    //  韩语测试
    CEasyGL_2DTextObject text2;
    text2.Config("TTF/batang.ttc", 200, 200, 30, 15, 0.0);
    text2.DrawText(L"한국어 시험 사랑해요", 0, 255, 0, 255, CEasyGL_2DImageObject::ESizeCalcMode_KeepPixel, 5000.0f);

    //  帧率显示
    CEasyGL_2DTextObject text_fps;
    text_fps.Config("TTF/FZZZHONGHJW.TTF", 64, 64, 30, 40, 0.0);
    text_fps.DrawText(L"FPS:0.0", 255, 255, 255, 255, CEasyGL_2DImageObject::ESizeCalcMode_KeepPixel, 1000.0f);



    //  定义调试类
    EasyGL_ConsolePositionDebug console_debug;
    console_debug.ConfigDebug(&image4, 1.0f, 0.0f, 0.0f, 1.0f);
    console_debug.StartThread();


    //  定义运行框架类
    CEasyGL_SimplePlatform platform;

    //  添加物体对象
    platform.AddOperationObject(&obj3d1);      //  带有纹理的立方体  大
    platform.AddOperationObject(&image1);      //  2D图片 黑色笑脸
    platform.AddOperationObject(&image4);      //  2D图片 皇后背景
    platform.AddOperationObject(&image2);      //  2D图片 粉色小姐姐头像
    //platform.AddOperationObject(&obj3d3);      //  3D骨骼显示的猴子头
    platform.AddOperationObject(&obj3d2);      //  骨骼显示的茶壶
    platform.AddOperationObject(&obj3d4);      //  带有纹理的立方体  小
    platform.AddOperationObject(&efficient_image1);  //  2D快速切换
    platform.AddOperationObject(&image3);   //  中间黑色小姐姐头像   2D图片 极品飞车
    platform.AddOperationObject(&image5);   //  旋转的指针
    platform.AddOperationObject(&image5_debug);   //  旋转的指针 的 调试
    platform.AddOperationObject(&video1);   //  2D视频画布
    //platform.AddOperationObject(&video2);   //  2D视频画布
    platform.AddOperationObject(&text1);    //  悬浮文字
    platform.AddOperationObject(&text2);    //  韩语测试
    platform.AddOperationObject(&text_fps); //  帧率显示

    platform.AddOperationObject(&console_debug);      //  2D图片 的 调试

    //  配置物体
    //  带有纹理的立方体  大
    obj3d1.Scale(0.2f, 0.2f, 0.2f);
    obj3d1.Move(0.5f, 0.0f, 0.0f);

    //  骨骼显示的茶壶
    obj3d2.Scale(0.1f, 0.1f, 0.1f);
    obj3d2.SetColor(0.0f, 1.0f, 0.0f, 1.0f);
    obj3d2.Move(-0.5f, 0.0f, 0.0f);

    //  2D图片 黑色笑脸
    image1.Scale(0.1f, 0.1f, 0.1f);
    image1.Move(-0.1f, 0.0f, 0.0f);

    //  2D图片 粉色小姐姐头像
    image2.Scale(0.1f, 0.1f, 0.1f);
    image2.Move(0.1f, 0.0f, 0.0f);

    //  中间黑色小姐姐头像   2D图片 极品飞车
    image3.Scale(0.5f, 0.5f, 0.5f);
    image3.Move(0.0f, 0.1f, -0.5f);

    //  2D图片 皇后背景
    //image4.Scale(0.5f, 0.5f, 0.5f);
    //image4.Move(-0.2f, 0.05f, 0.81f);

    //  3D骨骼显示的猴子头
    obj3d3.Scale(0.13f, 0.13f, 0.13f);
    obj3d3.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    obj3d3.Move(0.0f, -0.2f, 0.0f);

    //  旋转的指针
    image5.MoveCenter(-0.765151515f, -0.5f, 0.0f);
    image5.Scale(0.3f, 0.3f, 0.3f);
    image5.RotateDeg(0.0f, 0.0f, 0.0f);
    image5.Move(0.2f, 0.1f, -0.82f);

    //  带有纹理的立方体  小
    obj3d4.Scale(0.1f, 0.1f, 0.1f);
    obj3d4.Move(-0.7f, -0.2f, -0.83f);

    //  2D快速切换
    efficient_image1.Scale(0.2f, 0.2f, 0.2f);
    efficient_image1.Move(-0.1f, 0.15f, -0.51f);
    efficient_image1.Show(false);      //  关闭显示

    //  2D视频画布
    video1.Scale(0.5f, 0.5f, 0.5f);
    video1.Move(-0.98f, 0.08f, -0.9f);

    //  2D视频画布
    //video2.Scale(0.5f, 0.5f, 0.5f);
    //video2.Move(0.45f, 0.06f, -0.9f);

    //  悬浮文字
    text1.Scale(1.0f, 1.0f, 1.0f);
    text1.Move(0.15f, -0.1f, -0.89f);

    //  韩语测试
    text2.Scale(1.0f, 1.0f, 1.0f);
    text2.Move(0.2f, -0.15f, -0.89f);

    //  帧率显示
    text_fps.Scale(0.35f, 0.35f, 0.35f);
    text_fps.Move(-0.35f, -0.35f, -0.9f);


    //  准备运行
    platform.PlatformInit(&glfw_ws);
    platform.StartRenderThread();

    //  开始播放视频
    ffmpeg1.StartPlayerThread(CEasyGL_FFmpaePlatform::EPlayMode_Cycle);
    //ffmpeg2.StartPlayerThread(CEasyGL_FFmpaePlatform::EPlayMode_Cycle);

    //  等待渲染线程启动
    while(!glfw_ws.IsReady())
    {
        usleep(1);
    }

    //  等待用户按退出
    int i=0;
    int k=0;
    float deg1 = 0.0f;
    float deg2 = 0.0f;
    float deg3 = 0.0f;
    float deg4 = 0.0f;

    float deg5 = 0.0f;
    float deg6 = 0.0f;
    float deg7 = 0.0f;

    int kk = 0;
    while(glfw_ws.IsValid())
    {
        //  1次循环延时大约1ms
        linux_delay_us(1000L);
        //usleep(1000);
        i++;
        

        //  快速2D切换
        if(kk>=1000)
        {
            kk = 0;
        }
        else
        {
            kk++;
        }
        if(kk>=500)
        {
            efficient_image1.UpdateImage(b_buff);
        }
        else
        {
            efficient_image1.UpdateImage(w_buff);
        }
        

        //  指针旋转
        //deg1 = sin(i / 600.0f) * 120.0f - 90.0f;
        double d_i = i;
        deg1 = (i / (-18.0f));
        image5.RotateDeg(0.0f, 0.0f, deg1);


        //  两个立方体的旋转
        deg2 = i / 10.0f;
        deg3 = i / 50.0f;
        deg4 = i / 100.0f;
        obj3d1.RotateDeg(deg4, deg2, deg3);
        obj3d4.RotateDeg(deg4/10.0f, deg2/10.0f, deg3/10.0f);

        //  猴子头和茶壶的旋转
        deg5 = i / 60.0f;
        deg6 = i / 120.0f;
        deg7 = i / 600.0f;
        obj3d2.RotateDeg(deg7, deg5, deg6);
        obj3d3.RotateDeg(deg6, deg5, deg7);

        //  FPS的刷新显示
        if(k>=10)
        {
            float fps = platform.GetFPS();
            int i_fps = fps * 100.0;
            k=0;
            wchar_t tmp_str[30];
            memset(tmp_str, 0, sizeof(tmp_str));
            swprintf(tmp_str, 30, L"FPS:%d.%02d", i_fps / 100, i_fps%100);
            text_fps.DrawText(tmp_str, 255, 255, 255, 255, CEasyGL_2DImageObject::ESizeCalcMode_KeepSize, 0.0f);
            //printf("FPS = %f\r\n", fps);
        }
        else
        {
            k++;
        }

    }

    //  停止播放
    ffmpeg1.Close();
    //ffmpeg2.Close();

    //  退出
    platform.StopRenderThread();

    return 0;
}


