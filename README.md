基于OpenGLES开发的简单使用代码  
支持3D纯几何线条绘图  
支持3D填充纹理绘图  
支持2D填充纹理绘图  
支持视频播放  
支持字体

依赖freetype2  
依赖ffmpeg4.3  


大家可以免费使用，可以用于任何用途，但是记得注明出处  
倡导开源，因为开源才能让我们进步更快，走的更远

QQ:1444197961  
WeChat:+86-13104051251  
E-mail:rainhenry314@gmail.com  
Github:https://github.com/rainhenry  

这里简单说下环境的安装和搭建  
1、安装相关依赖包  
sudo apt-get install gcc g++ make vim git  
sudo apt-get install libgles2-mesa-dev  
sudo apt-get install libglfw3-dev  
sudo apt-get install libsdl2-dev  
sudo apt-get install libx264-dev  
sudo apt-get install libfreetype6-dev  

2、下载ffmpeg的4.3b版本的源码  
git clone https://git.ffmpeg.org/ffmpeg.git -b release/4.3

3、编译ffmpeg  
cd ffmpeg  
./configure --enable-shared --enable-ffplay --enable-sdl2 --enable-libx264 --enable-gpl  
make -j3  
sudo make install  
sudo ldconfig  

4、编译EasyOpenGLES  
cd EasyOpenGLES  
make clean  
make  

5、运行  
./run



