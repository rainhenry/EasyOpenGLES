
//  程序版本  REV 0.9

//  包含头文件
#include "EasyGL.h"

//  Shader
#include "ShaderSrc.h"


//  角度转弧度
float EasyGL_DegToRad(float in_deg)
{
    float deg = in_deg;
    float re = (deg * 2.0f * M_PI) / 360.0f;
    return (float)re;
}

//---------------------------------------------------------------------
//  纯几何骨架填充着色器
//  绘制数据为纯x y z的点信息
//  提供平移、三维旋转、缩放、骨骼颜色API

//  获取顶点着色器源码
const GLchar* CEasyGL_OnlyGeometryShader::GetVertexShaderSrc(void)
{
    return CEasyGL_OnlyGeometryShader_VertexShader_c;
}

//  片段着色器源码
const GLchar* CEasyGL_OnlyGeometryShader::GetFragmentShaderSrc(void)
{
    return CEasyGL_OnlyGeometryShader_FragmentShader_c;
}

//  构造函数
CEasyGL_OnlyGeometryShader::CEasyGL_OnlyGeometryShader()
{
    //  初始化
    this->ShaderHandle = 0;
    this->pvertex = 0;
    this->vertex_len = 0;
    this->move_x = 0.0f;
    this->move_y = 0.0f;
    this->move_z = 0.0f;
    this->move_center_x = 0.0f;
    this->move_center_y = 0.0f;
    this->move_center_z = 0.0f;
    this->rotate_rad_x = 0.0f;
    this->rotate_rad_y = 0.0f;
    this->rotate_rad_z = 0.0f;
    this->scale_x = 1.0f;
    this->scale_y = 1.0f;
    this->scale_z = 1.0f;
    this->blendcolor_r = 1.0f;
    this->blendcolor_g = 1.0f;
    this->blendcolor_b = 1.0f;
    this->blendcolor_a = 1.0f;
    this->shader_valid = false;

    pthread_mutex_init(&this->vertex_mutex, NULL);
    pthread_mutex_init(&this->move_mutex, NULL);
    pthread_mutex_init(&this->move_center_mutex, NULL);
    pthread_mutex_init(&this->rotate_mutex, NULL);
    pthread_mutex_init(&this->scale_mutex, NULL);
    pthread_mutex_init(&this->blendcolor_mutex, NULL);
}

//  析构函数
CEasyGL_OnlyGeometryShader::~CEasyGL_OnlyGeometryShader()
{
    if(this->ShaderHandle != 0)
    {
        glDeleteShader(this->ShaderHandle);
        this->ShaderHandle = 0;
    }

    pthread_mutex_destroy(&this->vertex_mutex);
    pthread_mutex_destroy(&this->move_mutex);
    pthread_mutex_destroy(&this->move_center_mutex);
    pthread_mutex_destroy(&this->rotate_mutex);
    pthread_mutex_destroy(&this->scale_mutex);
    pthread_mutex_destroy(&this->blendcolor_mutex);
}

//  初始化着色器，含载入、编译、链接等
//  返回着色器程序句柄
GLint CEasyGL_OnlyGeometryShader::ShaderInit(void)
{
    //  定义相关变量
    enum Consts {INFOLOG_LEN = EASYGL_INFOLOG_LEN};
    GLint re = 0;
    GLchar infoLog[INFOLOG_LEN];
    GLint fragment_shader;
    GLint shader_program = 0;
    GLint success;
    GLint vertex_shader;
    GLchar* psrc_v[1];
    GLchar* psrc_f[1];
    std::string filename;
    GLenum format = 0;


    //  当着色器已经初始化过
    if(this->shader_valid)
    {
        //glDeleteShader(this->ShaderHandle);
        //this->ShaderHandle = 0;
        //  返回着色器初始化失败，因为已经初始化过了
        return 0;
    }

    //  检查是否支持二进制的Shader
    GLint formats = 0;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);

    //  当不支持二进制格式
    if(formats < 1)
    {
        printf("Driver does not support any binary formats!!\r\n");
    }
    //  支持二进制格式
    else
    {
        //  载入格式
        filename = EASYGL_SHADER_BINARY_PATH + this->ClassName() + ".sdf";
        FILE* pfile = fopen(filename.c_str(), "rt");

        //  当不存在文件
        if(pfile == 0)
        {
            //  此时程序向下执行,然后创建Shader相关位二进制文件
        }
        //  当存在文件
        else
        {
            //  识别格式
            fscanf(pfile, "%08X", &format);
            fclose(pfile);

            //  创建程序
            shader_program = glCreateProgram();

            //  载入二进制文件
            filename = EASYGL_SHADER_BINARY_PATH + this->ClassName() + ".sdb";
            std::ifstream inputStream(filename.c_str(),
                                      std::ios::binary
                                     );
            std::istreambuf_iterator<char> startIt(inputStream), endIt;
            std::vector<char> buffer(startIt, endIt);
            inputStream.close();

            //  安装
            glProgramBinary(shader_program, format, buffer.data(), buffer.size());

            //  检查是否载入成功
            GLint status;
            glGetProgramiv(shader_program, GL_LINK_STATUS, &status);

            //  当载入失败
            if(status == GL_FALSE)
            {
                //  释放程序
                glDeleteShader(shader_program);

                //  提示失败
                printf("Load binary Shader Error!!\r\n");
            }
            //  当载入成功
            else
            {
                //  调试信息
                printf("Load %s format = %08X, Length = %ld\r\n", this->ClassName().c_str(), format, buffer.size());

                //  更新内部变量
                this->ShaderHandle = shader_program;

                //  着色器可用
                this->shader_valid = true;      

                //  返回程序
                return shader_program;
            }
        }
    }

    /* Vertex shader */
    printf("Compile:%s_VertexShader.c\r\n", this->ClassName().c_str());
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    psrc_v[0] = (GLchar*)(this->GetVertexShaderSrc());
    glShaderSource(vertex_shader, 1, psrc_v, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) 
    {
        glGetShaderInfoLog(vertex_shader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        re = 0;
    }
    //  当顶点着色器编译成功
    else
    {
        /* Fragment shader */
        printf("Compile:%s_FragmentShader.c\r\n", this->ClassName().c_str());
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        psrc_f[0] = (GLchar*)(this->GetFragmentShaderSrc());
        glShaderSource(fragment_shader, 1, psrc_f, NULL);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success) 
        {
            glGetShaderInfoLog(fragment_shader, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
            re = 0;
        }
        //  当片段着色器编译成功
        else
        {
            /* Link shaders */
            shader_program = glCreateProgram();
            glAttachShader(shader_program, vertex_shader);
            glAttachShader(shader_program, fragment_shader);
            glLinkProgram(shader_program);
            glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
            if (!success) 
            {
                glGetProgramInfoLog(shader_program, INFOLOG_LEN, NULL, infoLog);
                printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
                re = 0;
            }
            //  当连接成功
            else
            {
                //  检查shader是否合法,当合法
                if(shader_program != 0)
                {
                    //  当支持二进制格式
                    if(!(formats < 1))
                    {
                        //  将结果保存成二进制格式
                        //  获取二进制长度
                        GLint length = 0;
                        glGetProgramiv(shader_program, GL_PROGRAM_BINARY_LENGTH, &length);

                        //  接收二进制文件数据
                        std::vector<GLubyte> buffer(length);
                        GLenum format = 0;
                        glGetProgramBinary(shader_program, length, NULL, &format, buffer.data());

                        //  写入数据到文件
                        filename = EASYGL_SHADER_BINARY_PATH + this->ClassName() + ".sdb";
                        std::ofstream out(filename.c_str(), std::ios::binary);
                        out.write(reinterpret_cast<char*>(buffer.data()), length);
                        out.close();

                        //  写入格式到文件
                        filename = EASYGL_SHADER_BINARY_PATH + this->ClassName() + ".sdf";
                        FILE* pfile = fopen(filename.c_str(), "wt");
                        fprintf(pfile, "%08X", format);
                        fclose(pfile);

                        //  调试信息
                        printf("Save %s format = %08X, Length = %d\r\n", this->ClassName().c_str(), format, length);
                    }

                    //  设置返回值
                    re = shader_program;

                    //  更新内部变量
                    this->ShaderHandle = shader_program;

                    //  着色器可用
                    this->shader_valid = true;      
                }
                //  当非法
                else
                {
                    re = 0;
                }
            }
        }
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return re;
}

//  着色器是否可用
bool CEasyGL_OnlyGeometryShader::IsValid(void)
{
    return this->shader_valid;
}

//  着色器渲染,由操作对象调用，不可用由框架直接调用
void CEasyGL_OnlyGeometryShader::ShaderRender(void)
{
    //  检查着色器
    if(!this->shader_valid) return;

    //  应用着色器
    glUseProgram(this->ShaderHandle);  

    //  设置平移参数
    int id_vec3_move = glGetUniformLocation(this->ShaderHandle, "vec3_move");
    pthread_mutex_lock(&this->move_mutex);
    glUniform3f(id_vec3_move, this->move_x, this->move_y, this->move_z);
    pthread_mutex_unlock(&this->move_mutex);

    //  设置移动中心参数
    int id_vec3_move_center = glGetUniformLocation(this->ShaderHandle, "vec3_move_center");
    pthread_mutex_lock(&this->move_center_mutex);
    glUniform3f(id_vec3_move_center, this->move_center_x, this->move_center_y, this->move_center_z);
    pthread_mutex_unlock(&this->move_center_mutex);

    //  设置旋转参数
    pthread_mutex_lock(&this->rotate_mutex);
    GLfloat mat3_rotate_x[3*3] = 
    {   
       1.0,   0.0f,                     0.0f,
       0.0f,  cos(this->rotate_rad_x),  -sin(this->rotate_rad_x),
       0.0f,  sin(this->rotate_rad_x),  cos(this->rotate_rad_x)
    };
    GLfloat mat3_rotate_y[3*3] = 
    {   
       cos(this->rotate_rad_y),  0.0f,  sin(this->rotate_rad_y),
       0.0f,                     1.0f,  0.0f,
       -sin(this->rotate_rad_y), 0.0f,  cos(this->rotate_rad_y)
    };
    GLfloat mat3_rotate_z[3*3] = 
    {   
       cos(this->rotate_rad_z),  -sin(this->rotate_rad_z),  0.0f,
       sin(this->rotate_rad_z),  cos(this->rotate_rad_z),   0.0f,
       0.0f,                     0.0f,                      1.0f
    };
    pthread_mutex_unlock(&this->rotate_mutex);
    int id_mat3_rotate_x = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_x");
    int id_mat3_rotate_y = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_y");
    int id_mat3_rotate_z = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_z");
    glUniformMatrix3fv(id_mat3_rotate_x, 1, GL_FALSE, mat3_rotate_x);
    glUniformMatrix3fv(id_mat3_rotate_y, 1, GL_FALSE, mat3_rotate_y);
    glUniformMatrix3fv(id_mat3_rotate_z, 1, GL_FALSE, mat3_rotate_z);

    //  设置缩放参数
    pthread_mutex_lock(&this->scale_mutex);
    GLfloat mat3_scale[3*3] = 
    {   
       this->scale_x, 0.0f, 0.0f,
       0.0f, this->scale_y, 0.0f,
       0.0f, 0.0f, this->scale_z
    };
    pthread_mutex_unlock(&this->scale_mutex);
    int id_mat3_scale = glGetUniformLocation(this->ShaderHandle, "mat3_scale");
    glUniformMatrix3fv(id_mat3_scale, 1, GL_FALSE, mat3_scale);

    //  设置混合颜色
    int id_color = glGetUniformLocation(this->ShaderHandle, "in_blend_color");
    pthread_mutex_lock(&this->blendcolor_mutex);
    glUniform4f(id_color, 
                this->blendcolor_r, 
                this->blendcolor_g, 
                this->blendcolor_b, 
                this->blendcolor_a
               );
    pthread_mutex_unlock(&this->blendcolor_mutex);

    //  渲染操作
    GLint id_vertex = glGetAttribLocation(this->ShaderHandle, "in_pos");
    //  检查顶点信息可用
    pthread_mutex_lock(&this->vertex_mutex);
    if((this->pvertex != 0) &&
       (this->vertex_len > 0)
      )
    {
        //  设置顶点数据
        glVertexAttribPointer(id_vertex, 
                              3, 
                              GL_FLOAT, 
                              GL_FALSE, 
                              3*sizeof(GLfloat), 
                              this->pvertex
                              );

        //  使能当前顶点数据
        glEnableVertexAttribArray(id_vertex);

        //  执行绘图
        glDrawArrays(EASYGL_GEOMETRY_LINE_TYPE,
                     0,
                     this->vertex_len/3    //  这里的单位是点的数量
                                           //  除以3表示3个float数据为1个点信息
                    );
    }
    pthread_mutex_unlock(&this->vertex_mutex);

}

//  设置顶点原始数据
void CEasyGL_OnlyGeometryShader::SetVertex(GLfloat* pv,      //  顶点数据首地址
                                           int len           //  总长度，单位为float的个数
                                          )
{
    pthread_mutex_lock(&this->vertex_mutex);
    this->pvertex = pv;
    this->vertex_len = len;
    pthread_mutex_unlock(&this->vertex_mutex);
}

//  可选的设置顶点的平面元素索引数据
void CEasyGL_OnlyGeometryShader::SetVertexElements(
         GLuint* pdata,        //  索引数据首地址
         int len               //  索引数据长度，以数值个数为单位
        )
{
    //  本类无该功能
}

//  指定纹理
void CEasyGL_OnlyGeometryShader::SetTexture2D(
         GLint imageformat,           //  纹理图片格式
         int width,                   //  宽度
         int height,                  //  高度
         void* pdata                  //  纹理数据首地址
        )
{
    //  本类无该功能
}

//  平移
void CEasyGL_OnlyGeometryShader::Move(float x, float y, float z)
{
    //  更新参数
    pthread_mutex_lock(&this->move_mutex);
    this->move_x = x;
    this->move_y = y;
    this->move_z = z;
    pthread_mutex_unlock(&this->move_mutex);
}

//  移动物体的中心点
void CEasyGL_OnlyGeometryShader::MoveCenter(float x, float y, float z)
{
    //  更新参数
    pthread_mutex_lock(&this->move_center_mutex);
    this->move_center_x = x;
    this->move_center_y = y;
    this->move_center_z = z;
    pthread_mutex_unlock(&this->move_center_mutex);
}

//  以角度旋转
void CEasyGL_OnlyGeometryShader::RotateDeg(float x, float y, float z)
{
    //  更新参数
    pthread_mutex_lock(&this->rotate_mutex);
    this->rotate_rad_x = EasyGL_DegToRad(x);
    this->rotate_rad_y = EasyGL_DegToRad(y);
    this->rotate_rad_z = EasyGL_DegToRad(z);
    pthread_mutex_unlock(&this->rotate_mutex);
}

//  以弧度旋转
void CEasyGL_OnlyGeometryShader::RotateRad(float x, float y, float z)
{
    //  更新参数
    pthread_mutex_lock(&this->rotate_mutex);
    this->rotate_rad_x = x;
    this->rotate_rad_y = y;
    this->rotate_rad_z = z;
    pthread_mutex_unlock(&this->rotate_mutex);
}

//  缩放
void CEasyGL_OnlyGeometryShader::Scale(float x, float y, float z)
{
    pthread_mutex_lock(&this->scale_mutex);
    this->scale_x = x;
    this->scale_y = y;
    this->scale_z = z;
    pthread_mutex_unlock(&this->scale_mutex);
}

//  设置混合颜色
void CEasyGL_OnlyGeometryShader::SetBlendColor(float r, float g, float b, float a)
{
    pthread_mutex_lock(&this->blendcolor_mutex);
    this->blendcolor_r = r;
    this->blendcolor_g = g;
    this->blendcolor_b = b;
    this->blendcolor_a = a;
    pthread_mutex_unlock(&this->blendcolor_mutex);
}

//  当前类的名字
std::string CEasyGL_OnlyGeometryShader::ClassName(void)
{
    std::string re_str = "CEasyGL_OnlyGeometryShader";
    return re_str;
}

//---------------------------------------------------------------------
//  纯几何骨架填充着色器,以元素进行绘图
//  绘制数据为纯x y z的点信息
//  提供平移、三维旋转、缩放、骨骼颜色API

//  构造函数
CEasyGL_OnlyGeometryElementShader::CEasyGL_OnlyGeometryElementShader()
{
    this->p_vertex_element = 0;
    this->vertex_element_len = 0;
}

//  析构函数
CEasyGL_OnlyGeometryElementShader::~CEasyGL_OnlyGeometryElementShader()
{
}

//  着色器渲染,由操作对象调用，不可用由框架直接调用
void CEasyGL_OnlyGeometryElementShader::ShaderRender(void)
{
    //  检查着色器
    if(!this->shader_valid) return;

    //  应用着色器
    glUseProgram(this->ShaderHandle);  

    //  设置平移参数
    int id_vec3_move = glGetUniformLocation(this->ShaderHandle, "vec3_move");
    pthread_mutex_lock(&this->move_mutex);
    glUniform3f(id_vec3_move, this->move_x, this->move_y, this->move_z);
    pthread_mutex_unlock(&this->move_mutex);

    //  设置移动中心参数
    int id_vec3_move_center = glGetUniformLocation(this->ShaderHandle, "vec3_move_center");
    pthread_mutex_lock(&this->move_center_mutex);
    glUniform3f(id_vec3_move_center, this->move_center_x, this->move_center_y, this->move_center_z);
    pthread_mutex_unlock(&this->move_center_mutex);

    //  设置旋转参数
    pthread_mutex_lock(&this->rotate_mutex);
    GLfloat mat3_rotate_x[3*3] = 
    {   
       1.0,   0.0f,                     0.0f,
       0.0f,  cos(this->rotate_rad_x),  -sin(this->rotate_rad_x),
       0.0f,  sin(this->rotate_rad_x),  cos(this->rotate_rad_x)
    };
    GLfloat mat3_rotate_y[3*3] = 
    {   
       cos(this->rotate_rad_y),  0.0f,  sin(this->rotate_rad_y),
       0.0f,                     1.0f,  0.0f,
       -sin(this->rotate_rad_y), 0.0f,  cos(this->rotate_rad_y)
    };
    GLfloat mat3_rotate_z[3*3] = 
    {   
       cos(this->rotate_rad_z),  -sin(this->rotate_rad_z),  0.0f,
       sin(this->rotate_rad_z),  cos(this->rotate_rad_z),   0.0f,
       0.0f,                     0.0f,                      1.0f
    };
    pthread_mutex_unlock(&this->rotate_mutex);
    int id_mat3_rotate_x = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_x");
    int id_mat3_rotate_y = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_y");
    int id_mat3_rotate_z = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_z");
    glUniformMatrix3fv(id_mat3_rotate_x, 1, GL_FALSE, mat3_rotate_x);
    glUniformMatrix3fv(id_mat3_rotate_y, 1, GL_FALSE, mat3_rotate_y);
    glUniformMatrix3fv(id_mat3_rotate_z, 1, GL_FALSE, mat3_rotate_z);

    //  设置缩放参数
    pthread_mutex_lock(&this->scale_mutex);
    GLfloat mat3_scale[3*3] = 
    {   
       this->scale_x, 0.0f, 0.0f,
       0.0f, this->scale_y, 0.0f,
       0.0f, 0.0f, this->scale_z
    };
    pthread_mutex_unlock(&this->scale_mutex);
    int id_mat3_scale = glGetUniformLocation(this->ShaderHandle, "mat3_scale");
    glUniformMatrix3fv(id_mat3_scale, 1, GL_FALSE, mat3_scale);

    //  设置混合颜色
    int id_color = glGetUniformLocation(this->ShaderHandle, "in_blend_color");
    pthread_mutex_lock(&this->blendcolor_mutex);
    glUniform4f(id_color, 
                this->blendcolor_r, 
                this->blendcolor_g, 
                this->blendcolor_b, 
                this->blendcolor_a
               );
    pthread_mutex_unlock(&this->blendcolor_mutex);

    //  渲染操作
    GLint id_vertex = glGetAttribLocation(this->ShaderHandle, "in_pos");
    //  检查顶点信息可用
    pthread_mutex_lock(&this->vertex_mutex);
    if((this->pvertex != 0) &&
       (this->vertex_len > 0)
      )
    {
        //  设置顶点数据
        glVertexAttribPointer(id_vertex, 
                              3, 
                              GL_FLOAT, 
                              GL_FALSE, 
                              3*sizeof(GLfloat), 
                              this->pvertex
                              );

        //  使能顶点数据
        glEnableVertexAttribArray(id_vertex);

        //  执行绘图
        glDrawElements(EASYGL_GEOMETRY_LINE_TYPE,
                       this->vertex_element_len,
                       GL_UNSIGNED_INT,
                       this->p_vertex_element
                      );
    }
    pthread_mutex_unlock(&this->vertex_mutex);
}

//  可选的设置顶点的平面元素索引数据
void CEasyGL_OnlyGeometryElementShader::SetVertexElements(
         GLuint* pdata,  //  索引数据首地址
         int len         //  索引数据长度，以数值个数为单位
         )
{
    pthread_mutex_lock(&this->vertex_mutex);
    this->p_vertex_element = pdata;
    this->vertex_element_len = len;
    pthread_mutex_unlock(&this->vertex_mutex);
}

//  当前类的名字
std::string CEasyGL_OnlyGeometryElementShader::ClassName(void)
{
    std::string re_str = "CEasyGL_OnlyGeometryElementShader";
    return re_str;
}

//---------------------------------------------------------------------
//  无光照效果的3D纹理填充着色器
//  绘制数据为x y z的点信息 和 u v纹理坐标信息
//  提供平移、三维旋转、缩放、整体混合纯色的API

//  获取顶点着色器源码
const GLchar* CEasyGL_3DTextureShader::GetVertexShaderSrc(void)
{
    return CEasyGL_3DTextureShader_VertexShader_c;
}

//  片段着色器源码
const GLchar* CEasyGL_3DTextureShader::GetFragmentShaderSrc(void)
{
    return CEasyGL_3DTextureShader_FragmentShader_c;
}

//  构造函数
CEasyGL_3DTextureShader::CEasyGL_3DTextureShader()
{
    this->TextureBuff.format = GL_RGBA;
    this->TextureBuff.width = 0;
    this->TextureBuff.height = 0;
    this->TextureBuff.pdata = 0;

    pthread_mutex_init(&this->texture_mutex, NULL);

    //  配置纹理参数
    this->texture_handle = 0;
    glGenTextures(1, &this->texture_handle);
    glBindTexture(GL_TEXTURE_2D, this->texture_handle);
}

//  析构函数
CEasyGL_3DTextureShader::~CEasyGL_3DTextureShader()
{
    pthread_mutex_destroy(&this->texture_mutex);
}

//  着色器渲染,由操作对象调用，不可用由框架直接调用
void CEasyGL_3DTextureShader::ShaderRender(void)
{
    //  检查着色器
    if(!this->shader_valid) return;

    //  应用着色器
    glUseProgram(this->ShaderHandle);   

    //  设置平移参数
    int id_vec3_move = glGetUniformLocation(this->ShaderHandle, "vec3_move");
    pthread_mutex_lock(&this->move_mutex);
    glUniform3f(id_vec3_move, this->move_x, this->move_y, this->move_z);
    pthread_mutex_unlock(&this->move_mutex);

    //  设置移动中心参数
    int id_vec3_move_center = glGetUniformLocation(this->ShaderHandle, "vec3_move_center");
    pthread_mutex_lock(&this->move_center_mutex);
    glUniform3f(id_vec3_move_center, this->move_center_x, this->move_center_y, this->move_center_z);
    pthread_mutex_unlock(&this->move_center_mutex);

    //  设置旋转参数
    pthread_mutex_lock(&this->rotate_mutex);
    GLfloat mat3_rotate_x[3*3] = 
    {   
       1.0,   0.0f,                     0.0f,
       0.0f,  cos(this->rotate_rad_x),  -sin(this->rotate_rad_x),
       0.0f,  sin(this->rotate_rad_x),  cos(this->rotate_rad_x)
    };
    GLfloat mat3_rotate_y[3*3] = 
    {   
       cos(this->rotate_rad_y),  0.0f,  sin(this->rotate_rad_y),
       0.0f,                     1.0f,  0.0f,
       -sin(this->rotate_rad_y), 0.0f,  cos(this->rotate_rad_y)
    };
    GLfloat mat3_rotate_z[3*3] = 
    {   
       cos(this->rotate_rad_z),  -sin(this->rotate_rad_z),  0.0f,
       sin(this->rotate_rad_z),  cos(this->rotate_rad_z),   0.0f,
       0.0f,                     0.0f,                      1.0f
    };
    pthread_mutex_unlock(&this->rotate_mutex);
    int id_mat3_rotate_x = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_x");
    int id_mat3_rotate_y = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_y");
    int id_mat3_rotate_z = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_z");
    glUniformMatrix3fv(id_mat3_rotate_x, 1, GL_FALSE, mat3_rotate_x);
    glUniformMatrix3fv(id_mat3_rotate_y, 1, GL_FALSE, mat3_rotate_y);
    glUniformMatrix3fv(id_mat3_rotate_z, 1, GL_FALSE, mat3_rotate_z);

    //  设置缩放参数
    pthread_mutex_lock(&this->scale_mutex);
    GLfloat mat3_scale[3*3] = 
    {   
       this->scale_x, 0.0f, 0.0f,
       0.0f, this->scale_y, 0.0f,
       0.0f, 0.0f, this->scale_z
    };
    pthread_mutex_unlock(&this->scale_mutex);
    int id_mat3_scale = glGetUniformLocation(this->ShaderHandle, "mat3_scale");
    glUniformMatrix3fv(id_mat3_scale, 1, GL_FALSE, mat3_scale);

    //  设置混合颜色
    int id_color = glGetUniformLocation(this->ShaderHandle, "in_blend_color");
    pthread_mutex_lock(&this->blendcolor_mutex);
    glUniform4f(id_color, 
                this->blendcolor_r, 
                this->blendcolor_g, 
                this->blendcolor_b, 
                this->blendcolor_a
               );
    pthread_mutex_unlock(&this->blendcolor_mutex);

    //  渲染纹理
    int texture_localtion = glGetUniformLocation(this->ShaderHandle, "texture1");
    glEnable(GL_TEXTURE_2D);
    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);    //  采用正面剔除法
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    pthread_mutex_lock(&this->texture_mutex);
    //  当纹理图片信息合法
    if((this->TextureBuff.width > 0)  &&
       (this->TextureBuff.height > 0) &&
       (this->TextureBuff.pdata != 0)
      )
    {
        glTexImage2D(GL_TEXTURE_2D, 
                     0, 
                     this->TextureBuff.format,
                     this->TextureBuff.width,
                     this->TextureBuff.height,
                     0, 
                     this->TextureBuff.format,
                     GL_UNSIGNED_BYTE, 
                     this->TextureBuff.pdata
                     );
    }
    pthread_mutex_unlock(&this->texture_mutex);
    glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture_handle);
    glUniform1i(texture_localtion, 0);

    //  渲染操作
    GLint id_vertex = glGetAttribLocation(this->ShaderHandle, "in_pos");
    GLint id_texture_color = glGetAttribLocation(this->ShaderHandle, "in_color");
    //  检查顶点信息可用
    pthread_mutex_lock(&this->vertex_mutex);
    if((this->pvertex != 0) &&
       (this->vertex_len > 0)
      )
    {
        //  设置顶点数据
        glVertexAttribPointer(id_vertex, 
                              3, 
                              GL_FLOAT, 
                              GL_FALSE, 
                              5*sizeof(GLfloat), 
                              this->pvertex
                              );

        //  使能当前顶点数据
        glEnableVertexAttribArray(id_vertex);

        //  设置纹理数据
        glVertexAttribPointer(id_texture_color, 
                              2, 
                              GL_FLOAT, 
                              GL_FALSE, 
                              5*sizeof(GLfloat), 
                              this->pvertex + 3
                              );

        //  使能当前纹理坐标数据
        glEnableVertexAttribArray(id_texture_color);
 

        //  执行绘图
        glDrawArrays(GL_TRIANGLES,
                     0,
                     this->vertex_len/5    //  这里的单位是点的数量
                                           //  除以5表示5个float数据为1个点信息
                    );
    }
    pthread_mutex_unlock(&this->vertex_mutex);
}

//  指定纹理
void CEasyGL_3DTextureShader::SetTexture2D(GLint imageformat,           //  纹理图片格式
                                           int width,                   //  宽度
                                           int height,                  //  高度
                                           void* pdata                  //  纹理数据首地址
                                          )
{
    pthread_mutex_lock(&this->texture_mutex);
    this->TextureBuff.format = imageformat;
    this->TextureBuff.width = width;
    this->TextureBuff.height = height;
    this->TextureBuff.pdata = pdata;
    pthread_mutex_unlock(&this->texture_mutex);
}

//  当前类的名字
std::string CEasyGL_3DTextureShader::ClassName(void)
{
    std::string re_str = "CEasyGL_3DTextureShader";
    return re_str;
}

//---------------------------------------------------------------------
//  无光照效果的2D纹理填充着色器
//  绘制数据为x y z的点信息 和 u v纹理坐标信息
//  提供平移、三维旋转、缩放、整体混合纯色的API

//  着色器渲染,由操作对象调用，不可用由框架直接调用
void CEasyGL_2DTextureShader::ShaderRender(void)
{
    //  检查着色器
    if(!this->shader_valid) return;

    //  应用着色器
    glUseProgram(this->ShaderHandle);   

    //  设置平移参数
    int id_vec3_move = glGetUniformLocation(this->ShaderHandle, "vec3_move");
    pthread_mutex_lock(&this->move_mutex);
    glUniform3f(id_vec3_move, this->move_x, this->move_y, this->move_z);
    pthread_mutex_unlock(&this->move_mutex);

    //  设置移动中心参数
    int id_vec3_move_center = glGetUniformLocation(this->ShaderHandle, "vec3_move_center");
    pthread_mutex_lock(&this->move_center_mutex);
    glUniform3f(id_vec3_move_center, this->move_center_x, this->move_center_y, this->move_center_z);
    pthread_mutex_unlock(&this->move_center_mutex);

    //  设置旋转参数
    pthread_mutex_lock(&this->rotate_mutex);
    GLfloat mat3_rotate_x[3*3] = 
    {   
       1.0,   0.0f,                     0.0f,
       0.0f,  cos(this->rotate_rad_x),  -sin(this->rotate_rad_x),
       0.0f,  sin(this->rotate_rad_x),  cos(this->rotate_rad_x)
    };
    GLfloat mat3_rotate_y[3*3] = 
    {   
       cos(this->rotate_rad_y),  0.0f,  sin(this->rotate_rad_y),
       0.0f,                     1.0f,  0.0f,
       -sin(this->rotate_rad_y), 0.0f,  cos(this->rotate_rad_y)
    };
    GLfloat mat3_rotate_z[3*3] = 
    {   
       cos(this->rotate_rad_z),  -sin(this->rotate_rad_z),  0.0f,
       sin(this->rotate_rad_z),  cos(this->rotate_rad_z),   0.0f,
       0.0f,                     0.0f,                      1.0f
    };
    pthread_mutex_unlock(&this->rotate_mutex);
    int id_mat3_rotate_x = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_x");
    int id_mat3_rotate_y = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_y");
    int id_mat3_rotate_z = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_z");
    glUniformMatrix3fv(id_mat3_rotate_x, 1, GL_FALSE, mat3_rotate_x);
    glUniformMatrix3fv(id_mat3_rotate_y, 1, GL_FALSE, mat3_rotate_y);
    glUniformMatrix3fv(id_mat3_rotate_z, 1, GL_FALSE, mat3_rotate_z);

    //  设置缩放参数
    pthread_mutex_lock(&this->scale_mutex);
    GLfloat mat3_scale[3*3] = 
    {   
       this->scale_x, 0.0f, 0.0f,
       0.0f, this->scale_y, 0.0f,
       0.0f, 0.0f, this->scale_z
    };
    pthread_mutex_unlock(&this->scale_mutex);
    int id_mat3_scale = glGetUniformLocation(this->ShaderHandle, "mat3_scale");
    glUniformMatrix3fv(id_mat3_scale, 1, GL_FALSE, mat3_scale);

    //  设置混合颜色
    int id_color = glGetUniformLocation(this->ShaderHandle, "in_blend_color");
    pthread_mutex_lock(&this->blendcolor_mutex);
    glUniform4f(id_color, 
                this->blendcolor_r, 
                this->blendcolor_g, 
                this->blendcolor_b, 
                this->blendcolor_a
               );
    pthread_mutex_unlock(&this->blendcolor_mutex);

    //  渲染纹理
    int texture_localtion = glGetUniformLocation(this->ShaderHandle, "texture1");
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);    //  关闭面剔除法
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    pthread_mutex_lock(&this->texture_mutex);
    //  当纹理图片信息合法
    if((this->TextureBuff.width > 0)  &&
       (this->TextureBuff.height > 0) &&
       (this->TextureBuff.pdata != 0)
      )
    {
        glTexImage2D(GL_TEXTURE_2D, 
                     0, 
                     this->TextureBuff.format,
                     this->TextureBuff.width,
                     this->TextureBuff.height,
                     0, 
                     this->TextureBuff.format,
                     GL_UNSIGNED_BYTE, 
                     this->TextureBuff.pdata
                     );
    }
    pthread_mutex_unlock(&this->texture_mutex);
    glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture_handle);
    glUniform1i(texture_localtion, 0);

    //  渲染操作
    GLint id_vertex = glGetAttribLocation(this->ShaderHandle, "in_pos");
    GLint id_texture_color = glGetAttribLocation(this->ShaderHandle, "in_color");
    //  检查顶点信息可用
    pthread_mutex_lock(&this->vertex_mutex);
    if((this->pvertex != 0) &&
       (this->vertex_len > 0)
      )
    {
        //  设置顶点数据
        glVertexAttribPointer(id_vertex, 
                              3, 
                              GL_FLOAT, 
                              GL_FALSE, 
                              5*sizeof(GLfloat), 
                              this->pvertex
                              );

        //  使能当前顶点数据
        glEnableVertexAttribArray(id_vertex);

        //  设置纹理数据
        glVertexAttribPointer(id_texture_color, 
                              2, 
                              GL_FLOAT, 
                              GL_FALSE, 
                              5*sizeof(GLfloat), 
                              this->pvertex + 3
                              );

        //  使能当前纹理坐标数据
        glEnableVertexAttribArray(id_texture_color);
 

        //  执行绘图
        glDrawArrays(GL_TRIANGLES,
                     0,
                     this->vertex_len/5    //  这里的单位是点的数量
                                           //  除以5表示5个float数据为1个点信息
                    );
    }
    pthread_mutex_unlock(&this->vertex_mutex);
}

//  当前类的名字
std::string CEasyGL_2DTextureShader::ClassName(void)
{
    std::string re_str = "CEasyGL_2DTextureShader";
    return re_str;
}

//---------------------------------------------------------------------
//  无光照效果的3D纹理填充着色器(压缩)
//  绘制数据为x y z的点信息 和 u v纹理坐标信息
//  提供平移、三维旋转、缩放、整体混合纯色的API

//  构造函数
CEasyGL_3DTextureElementShader::CEasyGL_3DTextureElementShader()
{
    this->p_vertex_element = 0;
    this->vertex_element_len = 0;
}

//  析构函数
CEasyGL_3DTextureElementShader::~CEasyGL_3DTextureElementShader()
{
}

//  着色器渲染,由操作对象调用，不可用由框架直接调用
void CEasyGL_3DTextureElementShader::ShaderRender(void)
{
    //  检查着色器
    if(!this->shader_valid) return;

    //  应用着色器
    glUseProgram(this->ShaderHandle);   

    //  设置平移参数
    int id_vec3_move = glGetUniformLocation(this->ShaderHandle, "vec3_move");
    pthread_mutex_lock(&this->move_mutex);
    glUniform3f(id_vec3_move, this->move_x, this->move_y, this->move_z);
    pthread_mutex_unlock(&this->move_mutex);

    //  设置移动中心参数
    int id_vec3_move_center = glGetUniformLocation(this->ShaderHandle, "vec3_move_center");
    pthread_mutex_lock(&this->move_center_mutex);
    glUniform3f(id_vec3_move_center, this->move_center_x, this->move_center_y, this->move_center_z);
    pthread_mutex_unlock(&this->move_center_mutex);

    //  设置旋转参数
    pthread_mutex_lock(&this->rotate_mutex);
    GLfloat mat3_rotate_x[3*3] = 
    {   
       1.0,   0.0f,                     0.0f,
       0.0f,  cos(this->rotate_rad_x),  -sin(this->rotate_rad_x),
       0.0f,  sin(this->rotate_rad_x),  cos(this->rotate_rad_x)
    };
    GLfloat mat3_rotate_y[3*3] = 
    {   
       cos(this->rotate_rad_y),  0.0f,  sin(this->rotate_rad_y),
       0.0f,                     1.0f,  0.0f,
       -sin(this->rotate_rad_y), 0.0f,  cos(this->rotate_rad_y)
    };
    GLfloat mat3_rotate_z[3*3] = 
    {   
       cos(this->rotate_rad_z),  -sin(this->rotate_rad_z),  0.0f,
       sin(this->rotate_rad_z),  cos(this->rotate_rad_z),   0.0f,
       0.0f,                     0.0f,                      1.0f
    };
    pthread_mutex_unlock(&this->rotate_mutex);
    int id_mat3_rotate_x = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_x");
    int id_mat3_rotate_y = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_y");
    int id_mat3_rotate_z = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_z");
    glUniformMatrix3fv(id_mat3_rotate_x, 1, GL_FALSE, mat3_rotate_x);
    glUniformMatrix3fv(id_mat3_rotate_y, 1, GL_FALSE, mat3_rotate_y);
    glUniformMatrix3fv(id_mat3_rotate_z, 1, GL_FALSE, mat3_rotate_z);

    //  设置缩放参数
    pthread_mutex_lock(&this->scale_mutex);
    GLfloat mat3_scale[3*3] = 
    {   
       this->scale_x, 0.0f, 0.0f,
       0.0f, this->scale_y, 0.0f,
       0.0f, 0.0f, this->scale_z
    };
    pthread_mutex_unlock(&this->scale_mutex);
    int id_mat3_scale = glGetUniformLocation(this->ShaderHandle, "mat3_scale");
    glUniformMatrix3fv(id_mat3_scale, 1, GL_FALSE, mat3_scale);

    //  设置混合颜色
    int id_color = glGetUniformLocation(this->ShaderHandle, "in_blend_color");
    pthread_mutex_lock(&this->blendcolor_mutex);
    glUniform4f(id_color, 
                this->blendcolor_r, 
                this->blendcolor_g, 
                this->blendcolor_b, 
                this->blendcolor_a
               );
    pthread_mutex_unlock(&this->blendcolor_mutex);

    //  渲染纹理
    int texture_localtion = glGetUniformLocation(this->ShaderHandle, "texture1");
    glEnable(GL_TEXTURE_2D);
    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);    //  采用正面剔除法
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    pthread_mutex_lock(&this->texture_mutex);
    //  当纹理图片信息合法
    if((this->TextureBuff.width > 0)  &&
       (this->TextureBuff.height > 0) &&
       (this->TextureBuff.pdata != 0)
      )
    {
        glTexImage2D(GL_TEXTURE_2D, 
                     0, 
                     this->TextureBuff.format,
                     this->TextureBuff.width,
                     this->TextureBuff.height,
                     0, 
                     this->TextureBuff.format,
                     GL_UNSIGNED_BYTE, 
                     this->TextureBuff.pdata
                     );
    }
    pthread_mutex_unlock(&this->texture_mutex);
    glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture_handle);
    glUniform1i(texture_localtion, 0);

    //  渲染操作
    GLint id_vertex = glGetAttribLocation(this->ShaderHandle, "in_pos");
    GLint id_texture_color = glGetAttribLocation(this->ShaderHandle, "in_color");
    //  检查顶点信息可用
    pthread_mutex_lock(&this->vertex_mutex);
    if((this->pvertex != 0) &&
       (this->vertex_len > 0)
      )
    {
        //  设置顶点数据
        glVertexAttribPointer(id_vertex, 
                              3, 
                              GL_FLOAT, 
                              GL_FALSE, 
                              5*sizeof(GLfloat), 
                              this->pvertex
                              );

        //  使能当前顶点数据
        glEnableVertexAttribArray(id_vertex);

        //  设置纹理数据
        glVertexAttribPointer(id_texture_color, 
                              2, 
                              GL_FLOAT, 
                              GL_FALSE, 
                              5*sizeof(GLfloat), 
                              this->pvertex + 3
                              );

        //  使能当前纹理坐标数据
        glEnableVertexAttribArray(id_texture_color);
 
        //  执行绘图
        glDrawElements(GL_TRIANGLES,
                       this->vertex_element_len,
                       GL_UNSIGNED_INT,
                       this->p_vertex_element
                      );
    }
    pthread_mutex_unlock(&this->vertex_mutex);
}

//  可选的设置顶点的平面元素索引数据
void CEasyGL_3DTextureElementShader::SetVertexElements(
         GLuint* pdata,        //  索引数据首地址
         int len               //  索引数据长度，以数值个数为单位
        )
{
    pthread_mutex_lock(&this->vertex_mutex);
    this->p_vertex_element = pdata;
    this->vertex_element_len = len;
    pthread_mutex_unlock(&this->vertex_mutex);
}

//  当前类的名字
std::string CEasyGL_3DTextureElementShader::ClassName(void)
{
    std::string re_str = "CEasyGL_3DTextureElementShader";
    return re_str;
}

//---------------------------------------------------------------------
//  无光照效果的3D纹理填充着色器(压缩)
//  绘制数据为x y z的点信息 和 u v纹理坐标信息
//  提供平移、三维旋转、缩放、整体混合纯色的API

//  着色器渲染,由操作对象调用，不可用由框架直接调用
void CEasyGL_2DTextureElementShader::ShaderRender(void)
{
    //  检查着色器
    if(!this->shader_valid) return;

    //  应用着色器
    glUseProgram(this->ShaderHandle);   

    //  设置平移参数
    int id_vec3_move = glGetUniformLocation(this->ShaderHandle, "vec3_move");
    pthread_mutex_lock(&this->move_mutex);
    glUniform3f(id_vec3_move, this->move_x, this->move_y, this->move_z);
    pthread_mutex_unlock(&this->move_mutex);

    //  设置移动中心参数
    int id_vec3_move_center = glGetUniformLocation(this->ShaderHandle, "vec3_move_center");
    pthread_mutex_lock(&this->move_center_mutex);
    glUniform3f(id_vec3_move_center, this->move_center_x, this->move_center_y, this->move_center_z);
    pthread_mutex_unlock(&this->move_center_mutex);

    //  设置旋转参数
    pthread_mutex_lock(&this->rotate_mutex);
    GLfloat mat3_rotate_x[3*3] = 
    {   
       1.0,   0.0f,                     0.0f,
       0.0f,  cos(this->rotate_rad_x),  -sin(this->rotate_rad_x),
       0.0f,  sin(this->rotate_rad_x),  cos(this->rotate_rad_x)
    };
    GLfloat mat3_rotate_y[3*3] = 
    {   
       cos(this->rotate_rad_y),  0.0f,  sin(this->rotate_rad_y),
       0.0f,                     1.0f,  0.0f,
       -sin(this->rotate_rad_y), 0.0f,  cos(this->rotate_rad_y)
    };
    GLfloat mat3_rotate_z[3*3] = 
    {   
       cos(this->rotate_rad_z),  -sin(this->rotate_rad_z),  0.0f,
       sin(this->rotate_rad_z),  cos(this->rotate_rad_z),   0.0f,
       0.0f,                     0.0f,                      1.0f
    };
    pthread_mutex_unlock(&this->rotate_mutex);
    int id_mat3_rotate_x = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_x");
    int id_mat3_rotate_y = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_y");
    int id_mat3_rotate_z = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_z");
    glUniformMatrix3fv(id_mat3_rotate_x, 1, GL_FALSE, mat3_rotate_x);
    glUniformMatrix3fv(id_mat3_rotate_y, 1, GL_FALSE, mat3_rotate_y);
    glUniformMatrix3fv(id_mat3_rotate_z, 1, GL_FALSE, mat3_rotate_z);

    //  设置缩放参数
    pthread_mutex_lock(&this->scale_mutex);
    GLfloat mat3_scale[3*3] = 
    {   
       this->scale_x, 0.0f, 0.0f,
       0.0f, this->scale_y, 0.0f,
       0.0f, 0.0f, this->scale_z
    };
    pthread_mutex_unlock(&this->scale_mutex);
    int id_mat3_scale = glGetUniformLocation(this->ShaderHandle, "mat3_scale");
    glUniformMatrix3fv(id_mat3_scale, 1, GL_FALSE, mat3_scale);

    //  设置混合颜色
    int id_color = glGetUniformLocation(this->ShaderHandle, "in_blend_color");
    pthread_mutex_lock(&this->blendcolor_mutex);
    glUniform4f(id_color, 
                this->blendcolor_r, 
                this->blendcolor_g, 
                this->blendcolor_b, 
                this->blendcolor_a
               );
    pthread_mutex_unlock(&this->blendcolor_mutex);

    //  渲染纹理
    int texture_localtion = glGetUniformLocation(this->ShaderHandle, "texture1");
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);    //  关闭面剔除法
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    pthread_mutex_lock(&this->texture_mutex);
    //  当纹理图片信息合法
    if((this->TextureBuff.width > 0)  &&
       (this->TextureBuff.height > 0) &&
       (this->TextureBuff.pdata != 0)
      )
    {
        glTexImage2D(GL_TEXTURE_2D, 
                     0, 
                     this->TextureBuff.format,
                     this->TextureBuff.width,
                     this->TextureBuff.height,
                     0, 
                     this->TextureBuff.format,
                     GL_UNSIGNED_BYTE, 
                     this->TextureBuff.pdata
                     );
    }
    pthread_mutex_unlock(&this->texture_mutex);
    glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture_handle);
    glUniform1i(texture_localtion, 0);

    //  渲染操作
    GLint id_vertex = glGetAttribLocation(this->ShaderHandle, "in_pos");
    GLint id_texture_color = glGetAttribLocation(this->ShaderHandle, "in_color");
    //  检查顶点信息可用
    pthread_mutex_lock(&this->vertex_mutex);
    if((this->pvertex != 0) &&
       (this->vertex_len > 0)
      )
    {
        //  设置顶点数据
        glVertexAttribPointer(id_vertex, 
                              3, 
                              GL_FLOAT, 
                              GL_FALSE, 
                              5*sizeof(GLfloat), 
                              this->pvertex
                              );

        //  使能当前顶点数据
        glEnableVertexAttribArray(id_vertex);

        //  设置纹理数据
        glVertexAttribPointer(id_texture_color, 
                              2, 
                              GL_FLOAT, 
                              GL_FALSE, 
                              5*sizeof(GLfloat), 
                              this->pvertex + 3
                              );

        //  使能当前纹理坐标数据
        glEnableVertexAttribArray(id_texture_color);
 
        //  执行绘图
        glDrawElements(GL_TRIANGLES,
                       this->vertex_element_len,
                       GL_UNSIGNED_INT,
                       this->p_vertex_element
                      );
    }
    pthread_mutex_unlock(&this->vertex_mutex);
}

//  当前类的名字
std::string CEasyGL_2DTextureElementShader::ClassName(void)
{
    std::string re_str = "CEasyGL_2DTextureElementShader";
    return re_str;
}

//---------------------------------------------------------------------
//  无光照效果的3D纹理填充多重采样着色器(压缩)

//  构造函数
CEasyGL_2DTextureMultiSampleElementShader::CEasyGL_2DTextureMultiSampleElementShader()
{
}

//  析构函数
CEasyGL_2DTextureMultiSampleElementShader::~CEasyGL_2DTextureMultiSampleElementShader()
{
}

//  片段着色器源码
const GLchar* CEasyGL_2DTextureMultiSampleElementShader::GetFragmentShaderSrc(void)
{
    return CEasyGL_2DTextureMultiSampleElementShader_FragmentShader_c;
}

//  着色器渲染,由操作对象调用，不可用由框架直接调用
void CEasyGL_2DTextureMultiSampleElementShader::ShaderRender(void)
{
    //  检查着色器
    if(!this->shader_valid) return;

    //  应用着色器
    glUseProgram(this->ShaderHandle);   

    //  设置平移参数
    int id_vec3_move = glGetUniformLocation(this->ShaderHandle, "vec3_move");
    pthread_mutex_lock(&this->move_mutex);
    glUniform3f(id_vec3_move, this->move_x, this->move_y, this->move_z);
    pthread_mutex_unlock(&this->move_mutex);

    //  设置移动中心参数
    int id_vec3_move_center = glGetUniformLocation(this->ShaderHandle, "vec3_move_center");
    pthread_mutex_lock(&this->move_center_mutex);
    glUniform3f(id_vec3_move_center, this->move_center_x, this->move_center_y, this->move_center_z);
    pthread_mutex_unlock(&this->move_center_mutex);

    //  设置旋转参数
    pthread_mutex_lock(&this->rotate_mutex);
    GLfloat mat3_rotate_x[3*3] = 
    {   
       1.0,   0.0f,                     0.0f,
       0.0f,  cos(this->rotate_rad_x),  -sin(this->rotate_rad_x),
       0.0f,  sin(this->rotate_rad_x),  cos(this->rotate_rad_x)
    };
    GLfloat mat3_rotate_y[3*3] = 
    {   
       cos(this->rotate_rad_y),  0.0f,  sin(this->rotate_rad_y),
       0.0f,                     1.0f,  0.0f,
       -sin(this->rotate_rad_y), 0.0f,  cos(this->rotate_rad_y)
    };
    GLfloat mat3_rotate_z[3*3] = 
    {   
       cos(this->rotate_rad_z),  -sin(this->rotate_rad_z),  0.0f,
       sin(this->rotate_rad_z),  cos(this->rotate_rad_z),   0.0f,
       0.0f,                     0.0f,                      1.0f
    };
    pthread_mutex_unlock(&this->rotate_mutex);
    int id_mat3_rotate_x = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_x");
    int id_mat3_rotate_y = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_y");
    int id_mat3_rotate_z = glGetUniformLocation(this->ShaderHandle, "mat3_rotate_z");
    glUniformMatrix3fv(id_mat3_rotate_x, 1, GL_FALSE, mat3_rotate_x);
    glUniformMatrix3fv(id_mat3_rotate_y, 1, GL_FALSE, mat3_rotate_y);
    glUniformMatrix3fv(id_mat3_rotate_z, 1, GL_FALSE, mat3_rotate_z);

    //  设置缩放参数
    pthread_mutex_lock(&this->scale_mutex);
    GLfloat mat3_scale[3*3] = 
    {   
       this->scale_x, 0.0f, 0.0f,
       0.0f, this->scale_y, 0.0f,
       0.0f, 0.0f, this->scale_z
    };
    pthread_mutex_unlock(&this->scale_mutex);
    int id_mat3_scale = glGetUniformLocation(this->ShaderHandle, "mat3_scale");
    glUniformMatrix3fv(id_mat3_scale, 1, GL_FALSE, mat3_scale);

    //  设置混合颜色
    int id_color = glGetUniformLocation(this->ShaderHandle, "in_blend_color");
    pthread_mutex_lock(&this->blendcolor_mutex);
    glUniform4f(id_color, 
                this->blendcolor_r, 
                this->blendcolor_g, 
                this->blendcolor_b, 
                this->blendcolor_a
               );
    pthread_mutex_unlock(&this->blendcolor_mutex);

    //  渲染纹理
    int texture_localtion = glGetUniformLocation(this->ShaderHandle, "texture1");
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);    //  关闭面剔除法
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    pthread_mutex_lock(&this->texture_mutex);
    //  当纹理图片信息合法
    if((this->TextureBuff.width > 0)  &&
       (this->TextureBuff.height > 0) &&
       (this->TextureBuff.pdata != 0)
      )
    {
        glTexImage2D(GL_TEXTURE_2D_MULTISAMPLE, 
                     0, 
                     this->TextureBuff.format,
                     this->TextureBuff.width,
                     this->TextureBuff.height,
                     0, 
                     this->TextureBuff.format,
                     GL_UNSIGNED_BYTE, 
                     this->TextureBuff.pdata
                     );
    }
    pthread_mutex_unlock(&this->texture_mutex);
    glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture_handle);
    glUniform1i(texture_localtion, 0);

    //  渲染操作
    GLint id_vertex = glGetAttribLocation(this->ShaderHandle, "in_pos");
    GLint id_texture_color = glGetAttribLocation(this->ShaderHandle, "in_color");
    //  检查顶点信息可用
    pthread_mutex_lock(&this->vertex_mutex);
    if((this->pvertex != 0) &&
       (this->vertex_len > 0)
      )
    {
        //  设置顶点数据
        glVertexAttribPointer(id_vertex, 
                              3, 
                              GL_FLOAT, 
                              GL_FALSE, 
                              5*sizeof(GLfloat), 
                              this->pvertex
                              );

        //  使能当前顶点数据
        glEnableVertexAttribArray(id_vertex);

        //  设置纹理数据
        glVertexAttribPointer(id_texture_color, 
                              2, 
                              GL_FLOAT, 
                              GL_FALSE, 
                              5*sizeof(GLfloat), 
                              this->pvertex + 3
                              );

        //  使能当前纹理坐标数据
        glEnableVertexAttribArray(id_texture_color);
 
        //  执行绘图
        glDrawElements(GL_TRIANGLES,
                       this->vertex_element_len,
                       GL_UNSIGNED_INT,
                       this->p_vertex_element
                      );
    }
    pthread_mutex_unlock(&this->vertex_mutex);
}

//  当前类的名字
std::string CEasyGL_2DTextureMultiSampleElementShader::ClassName(void)
{
    std::string re_str = "CEasyGL_2DTextureMultiSampleElementShader";
    return re_str;
}


//---------------------------------------------------------------------
//  纯顶点骨架的非压缩类，即采用Draw方法绘图

//  构造函数
CEasyGL_3DGeometryObject::CEasyGL_3DGeometryObject()
{
    this->pshader = 0;
    this->pvertex = 0;
    this->vertex_len = 0;
    this->move_x = 0.0f;
    this->move_y = 0.0f;
    this->move_z = 0.0f;
    this->move_center_x = 0.0f;
    this->move_center_y = 0.0f;
    this->move_center_z = 0.0f;
    this->rotate_rad_x = 0.0f;
    this->rotate_rad_y = 0.0f;
    this->rotate_rad_z = 0.0f;
    this->scale_x = 1.0f;
    this->scale_y = 1.0f;
    this->scale_z = 1.0f;
    this->color_r = 1.0f;
    this->color_g = 1.0f;
    this->color_b = 1.0f;
    this->color_a = 1.0f;

    this->show_flag = true;    //  根据常见编程习惯，默认显示

    pthread_mutex_init(&this->data_mutex, NULL);
    pthread_mutex_init(&this->show_mutex, NULL);
}

//  析构函数
CEasyGL_3DGeometryObject::~CEasyGL_3DGeometryObject()
{
    pthread_mutex_destroy(&this->data_mutex);
    pthread_mutex_destroy(&this->show_mutex);
}

//  可选基本初始化
//  成功返回0，失败返回非0
int CEasyGL_3DGeometryObject::Init(CEasyGL_ShaderBase* ps)
{
    //  检查输入参数
    if(ps == 0) return -1;

    //  配置
    this->pshader = ps;

    //  返回成功
    return 0;
}

//  编译着色器
int CEasyGL_3DGeometryObject::CompileShader(void)
{
    if(this->pshader != 0)
    {
        this->pshader->ShaderInit();
    }
} 

//  配置顶点信息
//  成功返回0，失败返回非0
int CEasyGL_3DGeometryObject::ConfigVertex(GLfloat* pv,      //  顶点数据首地址
                                           int len           //  总长度，单位为float的个数
                                          )
{
    //  检查参数是否合法,当合法
    if((pv != 0) && (len > 0) && (this->pshader != 0))
    {
        pthread_mutex_lock(&this->data_mutex);
        this->pvertex = pv;
        this->vertex_len = len;
        pthread_mutex_unlock(&this->data_mutex);
        return 0;
    }
    //  非法
    else
    {
        return -1;
    }
}


//  渲染操作
void CEasyGL_3DGeometryObject::Render(void)
{
    //  检查参数是否合法,当合法
    if(this->pshader != 0)
    {
        this->pshader->ShaderRender();
    }
}

//  可选基本释放
void CEasyGL_3DGeometryObject::Release(void)
{
}

//  平移
void CEasyGL_3DGeometryObject::Move(float x, float y, float z)
{
    pthread_mutex_lock(&this->data_mutex);
    this->move_x = x;
    this->move_y = y;
    this->move_z = z;
    pthread_mutex_unlock(&this->data_mutex);
}

//  平移物体中心点
void CEasyGL_3DGeometryObject::MoveCenter(float x, float y, float z)
{
    pthread_mutex_lock(&this->data_mutex);
    this->move_center_x = x;
    this->move_center_y = y;
    this->move_center_z = z;
    pthread_mutex_unlock(&this->data_mutex);
}

//  以角度旋转
void CEasyGL_3DGeometryObject::RotateDeg(float x, float y, float z)
{
    pthread_mutex_lock(&this->data_mutex);
    this->rotate_rad_x = EasyGL_DegToRad(x);
    this->rotate_rad_y = EasyGL_DegToRad(y);
    this->rotate_rad_z = EasyGL_DegToRad(z);
    pthread_mutex_unlock(&this->data_mutex);
}

//  以弧度旋转
void CEasyGL_3DGeometryObject::RotateRad(float x, float y, float z)
{
    pthread_mutex_lock(&this->data_mutex);
    this->rotate_rad_x = x;
    this->rotate_rad_y = y;
    this->rotate_rad_z = z;
    pthread_mutex_unlock(&this->data_mutex);
}

//  缩放
void CEasyGL_3DGeometryObject::Scale(float x, float y, float z)
{
    pthread_mutex_lock(&this->data_mutex);
    this->scale_x = x;
    this->scale_y = y;
    this->scale_z = z;
    pthread_mutex_unlock(&this->data_mutex);
}

//  设置骨骼颜色
void CEasyGL_3DGeometryObject::SetColor(float r, float g, float b, float a)
{
    pthread_mutex_lock(&this->data_mutex);
    this->color_r = r;
    this->color_g = g;
    this->color_b = b;
    this->color_a = a;
    pthread_mutex_unlock(&this->data_mutex);
}

//  刷新(纹理、坐标数据等)
void CEasyGL_3DGeometryObject::Refresh(void)
{
    //  检查参数是否合法,当合法
    if(this->pshader != 0)
    {
        pthread_mutex_lock(&this->data_mutex);
        this->pshader->SetVertex(this->pvertex, this->vertex_len);
        this->pshader->Move(move_x, move_y, move_z);
        this->pshader->MoveCenter(move_center_x, move_center_y, move_center_z);
        this->pshader->RotateRad(rotate_rad_x, rotate_rad_y, rotate_rad_z);
        this->pshader->Scale(scale_x, scale_y, scale_z);
        this->pshader->SetBlendColor(color_r, color_g, color_b, color_a);
        this->pshader->ShaderRender();
        pthread_mutex_unlock(&this->data_mutex);
    }
}

//  设置显示状态
void CEasyGL_3DGeometryObject::Show(bool status)
{
    pthread_mutex_lock(&this->show_mutex);
    this->show_flag = status;
    pthread_mutex_unlock(&this->show_mutex);
}

//  获取当前显示状态
bool CEasyGL_3DGeometryObject::GetShowStatus(void)
{
    bool re = false;
    pthread_mutex_lock(&this->show_mutex);
    re = this->show_flag;
    pthread_mutex_unlock(&this->show_mutex);
    return re;
}

//  获取当前对象的Z轴位置(调试用途)
float CEasyGL_3DGeometryObject::GetZ(void)
{
    float re = 0.0f;
    pthread_mutex_lock(&this->data_mutex);
    re = this->move_z;
    pthread_mutex_unlock(&this->data_mutex);
    return re;
}

//---------------------------------------------------------------------
//  纯顶点骨架的压缩类，即采用Elements方法绘图

//  构造函数
CEasyGL_3DGeometryElementObject::CEasyGL_3DGeometryElementObject()
{
}

//  析构函数
CEasyGL_3DGeometryElementObject::~CEasyGL_3DGeometryElementObject()
{
}

//  可选的设置顶点的平面元素索引数据
void CEasyGL_3DGeometryElementObject::ConfigVertexElements(
         GLuint* pdata,        //  索引数据首地址
         int len               //  索引数据长度，以数值个数为单位
        )
{
    //  检查参数是否合法,当合法
    if(this->pshader != 0)
    {
        pthread_mutex_lock(&this->data_mutex);
        this->pvertex_element = pdata;
        this->vertex_element_len = len;
        pthread_mutex_unlock(&this->data_mutex);
    }
}

//  刷新(纹理、坐标数据等)
void CEasyGL_3DGeometryElementObject::Refresh(void)
{
    //  检查参数是否合法,当合法
    if(this->pshader != 0)
    {
        pthread_mutex_lock(&this->data_mutex);
        this->pshader->SetVertex(this->pvertex, this->vertex_len);
        this->pshader->SetVertexElements(this->pvertex_element, this->vertex_element_len);
        this->pshader->Move(move_x, move_y, move_z);
        this->pshader->MoveCenter(move_center_x, move_center_y, move_center_z);
        this->pshader->RotateRad(rotate_rad_x, rotate_rad_y, rotate_rad_z);
        this->pshader->Scale(scale_x, scale_y, scale_z);
        this->pshader->SetBlendColor(color_r, color_g, color_b, color_a);
        this->pshader->ShaderRender();
        pthread_mutex_unlock(&this->data_mutex);
    }
}


//---------------------------------------------------------------------
//  3D纹理的非压缩类，即采用Draw方法绘图

//  构造函数
CEasyGL_3DTextureObject::CEasyGL_3DTextureObject()
{
    this->texture_info.format = GL_RGBA;
    this->texture_info.width = 0;
    this->texture_info.height = 0;
    this->texture_info.pdata = 0;
}

//  析构函数
CEasyGL_3DTextureObject::~CEasyGL_3DTextureObject()
{
    //  当存在图像未释放
    if(this->texture_info.pdata != 0)
    {
        //  释放
        stbi_image_free(this->texture_info.pdata);
        this->texture_info.format = GL_RGBA;
        this->texture_info.width = 0;
        this->texture_info.height = 0;
        this->texture_info.pdata = 0;
    }
}

//  从图片文件载入纹理数据,支持渲染过程中的动态载入
//  成功返回0  失败返回非0
int CEasyGL_3DTextureObject::LoadTextureFromImageFile(std::string filename)
{
    //  定义临时纹理资源结构对象
    CEasyGL_ShaderBase::STextureInfo tmp_info;
    tmp_info.format = GL_RGBA;
    tmp_info.width = 0;
    tmp_info.height = 0;
    tmp_info.pdata = 0;
    int channels = 0;
 
    //  检查着色器是否合法，当非法
    if(this->pshader == 0) return -3;

    //  尝试载入纹理图片
    tmp_info.pdata = stbi_load(filename.c_str(),
                               &tmp_info.width,
                               &tmp_info.height,
                               &channels,
                               0
                              );

    //  检测释放载入成功,当载入失败
    if(tmp_info.pdata == 0) return -1;

    //  当为4通道图像
    if(channels == 4)
    {
        tmp_info.format = GL_RGBA;
    }
    //  当为3通道图像
    else if(channels == 3)
    {
        tmp_info.format = GL_RGB;
    }
    //  格式错误
    else
    {
        //  该格式虽然程序中并未考虑，但是需要释放资源
        stbi_image_free(tmp_info.pdata);
        return -2;
    }

    //  备份上一次的纹理资源
    pthread_mutex_lock(&this->data_mutex);
    CEasyGL_ShaderBase::STextureInfo last_texture = this->texture_info;

    //  更新当前纹理数据
    this->texture_info = tmp_info;
    pthread_mutex_unlock(&this->data_mutex);

    //  切换完成后，释放之前的资源
    if(last_texture.pdata != 0)
    {
        stbi_image_free(last_texture.pdata);
    }

    //  操作成功
    return 0;
}

//  刷新(纹理、坐标数据等)
void CEasyGL_3DTextureObject::Refresh(void)
{
    //  检查参数是否合法,当合法
    if(this->pshader != 0)
    {
        pthread_mutex_lock(&this->data_mutex);
        this->pshader->SetVertex(this->pvertex, this->vertex_len);
        this->pshader->Move(move_x, move_y, move_z);
        this->pshader->MoveCenter(move_center_x, move_center_y, move_center_z);
        this->pshader->RotateRad(rotate_rad_x, rotate_rad_y, rotate_rad_z);
        this->pshader->Scale(scale_x, scale_y, scale_z);
        this->pshader->SetBlendColor(color_r, color_g, color_b, color_a);
        this->pshader->SetTexture2D(this->texture_info.format,          //  纹理图片格式
                                    this->texture_info.width,           //  宽度
                                    this->texture_info.height,          //  高度
                                    this->texture_info.pdata            //  纹理数据首地址
                                   );
        this->pshader->ShaderRender();
        pthread_mutex_unlock(&this->data_mutex);
    }
}

//---------------------------------------------------------------------
//  2D纹理贴图的非压缩类，即采用Draw方法绘图

//  自己的Shader
CEasyGL_2DTextureElementShader CEasyGL_2DImageObject::my_shader;

//  构造函数
CEasyGL_2DImageObject::CEasyGL_2DImageObject()
{
    //  配置顶点
    rect_v[0]= 0.0f;rect_v[1]= 0.0f;rect_v[2]= 0.0f; rect_v[3]= 0.0f;rect_v[4]= 1.0f;
    rect_v[5]= 0.0f;rect_v[6]= 1.0f;rect_v[7]= 0.0f; rect_v[8]= 0.0f;rect_v[9]= 0.0f;
    rect_v[10]=1.0f;rect_v[11]=0.0f;rect_v[12]=0.0f; rect_v[13]=1.0f;rect_v[14]=1.0f;
    rect_v[15]=1.0f;rect_v[16]=1.0f;rect_v[17]=0.0f; rect_v[18]=1.0f;rect_v[19]=0.0f;

    //  配置元素
    rect_v_index[0]=0;rect_v_index[1]=3;rect_v_index[2]=1;
    rect_v_index[3]=0;rect_v_index[4]=2;rect_v_index[5]=3;

    //  配置着色器
    this->Init(&this->my_shader);

    //  载入顶点信息
    this->ConfigVertex((GLfloat*)this->rect_v, 20);

    //  配置调试对象
    this->pDebugObj = 0;
}

//  析构函数
CEasyGL_2DImageObject::~CEasyGL_2DImageObject()
{
}

//  载入显示图片,根据图片自动计算显示比例
//  成功返回0  失败返回非0
int CEasyGL_2DImageObject::LoadImage(std::string filename, 
                                     CEasyGL_2DImageObject::ESizeCalcMode mode,
                                     float pixel             //  当像素比例时，每个单位的像素值
                                    )
{
    //  定义返回值
    int re = -1;

    //  定义临时纹理资源结构对象
    CEasyGL_ShaderBase::STextureInfo image_info;
    image_info.format = GL_RGBA;
    image_info.width = 0;
    image_info.height = 0;
    image_info.pdata = 0;
    int channels = 0;
 
    //  检查着色器是否合法，当非法
    if(this->pshader == 0) return -3;

    //  尝试载入纹理图片
    image_info.pdata = stbi_load(filename.c_str(),
                               &image_info.width,
                               &image_info.height,
                               &channels,
                               0
                              );

    //  检测释放载入成功,当载入失败
    if(image_info.pdata == 0) return -1;

    //  当为4通道图像
    if(channels == 4)
    {
        image_info.format = GL_RGBA;
    }
    //  当为3通道图像
    else if(channels == 3)
    {
        image_info.format = GL_RGB;
    }
    //  格式错误
    else
    {
        //  该格式虽然程序中并未考虑，但是需要释放资源
        stbi_image_free(image_info.pdata);
        return -2;
    }

    //  操作成功
    re = 0;


    //  备份上一次的纹理资源
    pthread_mutex_lock(&this->data_mutex);
    CEasyGL_ShaderBase::STextureInfo last_texture = this->texture_info;

    //  更新当前纹理数据
    this->texture_info = image_info;

    //  当为像素比例
    if(mode == ESizeCalcMode_KeepPixel)
    {
        //  定义图像信息的临时变量
        CEasyGL_ShaderBase::STextureInfo tmp_info;

        //  获取图形信息
        tmp_info = this->texture_info;

        //  根据像素信息计算比例
        float x = (tmp_info.width * 1.0f) / pixel;
        float y = (tmp_info.height * 1.0f) / pixel;

        //  赋值设置
        rect_v[10]=x;     //  x
        rect_v[15]=x;     //  x
        rect_v[6]= y;     //  y
        rect_v[16]=y;     //  y

        //  调试对象
        if(this->pDebugObj != 0)
        {
            this->pDebugObj->SetWidthHighFloat(x, y);
        }
    }
    //  当为尺寸比例
    else
    {
        //  定义图像信息的临时变量
        CEasyGL_ShaderBase::STextureInfo tmp_info;

        //  获取图形信息
        tmp_info = this->texture_info;

        //  当图像信息可用
        if((tmp_info.width > 0) && (tmp_info.height > 0))
        {
            //  当宽度大于高度
            if(tmp_info.width > tmp_info.height)
            {
                float tmp_height = tmp_info.height;
                float tmp_width = tmp_info.width;
                rect_v[10]=1.0f;                       //  x
                rect_v[15]=1.0f;                       //  x
                rect_v[6]= tmp_height / tmp_width;     //  y
                rect_v[16]=tmp_height / tmp_width;     //  y

                //  调试对象
                if(this->pDebugObj != 0)
                {
                    this->pDebugObj->SetWidthHighFloat(1.0f, tmp_height / tmp_width);
                }
            }
            //  当宽度小于高度
            else if(tmp_info.width < tmp_info.height)
            {
                float tmp_height = tmp_info.height;
                float tmp_width = tmp_info.width;
                rect_v[10]=tmp_width / tmp_height;     //  x
                rect_v[15]=tmp_width / tmp_height;     //  x
                rect_v[6]= 1.0f;                       //  y
                rect_v[16]=1.0f;                       //  y

                //  调试对象
                if(this->pDebugObj != 0)
                {
                    this->pDebugObj->SetWidthHighFloat(tmp_width / tmp_height, 1.0f);
                }
            }
            //  否则相等
            else
            {
                rect_v[10]=1.0f;     //  x
                rect_v[15]=1.0f;     //  x
                rect_v[6]= 1.0f;     //  y
                rect_v[16]=1.0f;     //  y

                //  调试对象
                if(this->pDebugObj != 0)
                {
                    this->pDebugObj->SetWidthHighFloat(1.0f, 1.0f);
                }
            }
        }
    }
    pthread_mutex_unlock(&this->data_mutex);

    //  切换完成后，释放之前的资源
    if(last_texture.pdata != 0)
    {
        stbi_image_free(last_texture.pdata);
    }

    //  操作完成
    return re;
}

//  设置长宽尺寸(调试用的浮点值)
void CEasyGL_2DImageObject::SetWidthHighFloat(float x, float y)
{
    pthread_mutex_lock(&this->data_mutex);
    rect_v[10]=x;     //  x
    rect_v[15]=x;     //  x
    rect_v[6]= y;     //  y
    rect_v[16]=y;     //  y
    pthread_mutex_unlock(&this->data_mutex);
}

//  2D平移
void CEasyGL_2DImageObject::Move2D(float x, float y)
{
    pthread_mutex_lock(&this->data_mutex);
    this->move_x = x;
    this->move_y = y;
    pthread_mutex_unlock(&this->data_mutex);

    //  调试对象
    if(this->pDebugObj != 0)
    {
        this->pDebugObj->Move2D(x, y);
    }
}

//  2D移动物体的中心点
void CEasyGL_2DImageObject::MoveCenter2D(float x, float y)
{
    pthread_mutex_lock(&this->data_mutex);
    this->move_center_x = x;
    this->move_center_y = y;
    pthread_mutex_unlock(&this->data_mutex);

    //  取消调试对象的中心点移动,这样可以看到旋转中心的位置
    ////  调试对象
    //if(this->pDebugObj != 0)
    //{
    //    this->pDebugObj->MoveCenter2D(x, y);
    //}
}

//  配置调试
void CEasyGL_2DImageObject::ConfigDebug(CEasyGL_2DImageObject* pobj)
{
    this->pDebugObj = pobj;

    //  当调试对象可用
    if(this->pDebugObj != 0)
    {
        this->pDebugObj->SetWidthHighFloat(this->rect_v[10], this->rect_v[6]);
    }
}


//  刷新(纹理、坐标数据等)
void CEasyGL_2DImageObject::Refresh(void)
{
    //  检查参数是否合法,当合法
    if(this->pshader != 0)
    {
        pthread_mutex_lock(&this->data_mutex);
        this->pshader->SetVertex(this->pvertex, this->vertex_len);
        this->pshader->SetVertexElements(this->rect_v_index, 6);
        this->pshader->Move(move_x, move_y, move_z);
        this->pshader->MoveCenter(move_center_x, move_center_y, move_center_z);
        this->pshader->RotateRad(rotate_rad_x, rotate_rad_y, rotate_rad_z);
        this->pshader->Scale(scale_x, scale_y, scale_z);
        this->pshader->SetBlendColor(color_r, color_g, color_b, color_a);
        this->pshader->SetTexture2D(this->texture_info.format,          //  纹理图片格式
                                    this->texture_info.width,           //  宽度
                                    this->texture_info.height,          //  高度
                                    this->texture_info.pdata            //  纹理数据首地址
                                   );
        this->pshader->ShaderRender();
        pthread_mutex_unlock(&this->data_mutex);
    }
}

//  平移
void CEasyGL_2DImageObject::Move(float x, float y, float z)
{
    //  调用基类方法
    CEasyGL_3DTextureObject::Move(x, y, z);

    //  调试对象
    if(this->pDebugObj != 0)
    {
        this->pDebugObj->Move(x, y, z - EASYGL_DEBUG_Z_MIN);
    }
}

//  移动物体的中心点
void CEasyGL_2DImageObject::MoveCenter(float x, float y, float z)
{
    //  调用基类方法
    CEasyGL_3DTextureObject::MoveCenter(x, y, z);

    //  取消调试对象的中心点移动,这样可以看到旋转中心的位置
    ////  调试对象
    //if(this->pDebugObj != 0)
    //{
    //    this->pDebugObj->MoveCenter(x, y, z);
    //}
}

//  以角度旋转
void CEasyGL_2DImageObject::RotateDeg(float x, float y, float z)
{
    //  调用基类方法
    CEasyGL_3DTextureObject::RotateDeg(x, y, z);

    //  调试对象
    if(this->pDebugObj != 0)
    {
        this->pDebugObj->RotateDeg(x, y, z);
    }
}

//  以弧度旋转
void CEasyGL_2DImageObject::RotateRad(float x, float y, float z)
{
    //  调用基类方法
    CEasyGL_3DTextureObject::RotateRad(x, y, z);

    //  调试对象
    if(this->pDebugObj != 0)
    {
        this->pDebugObj->RotateRad(x, y, z);
    }
}

//  缩放
void CEasyGL_2DImageObject::Scale(float x, float y, float z)
{
    //  调用基类方法
    CEasyGL_3DTextureObject::Scale(x, y, z);

    //  调试对象
    if(this->pDebugObj != 0)
    {
        this->pDebugObj->Scale(x, y, z);
    }
}


//---------------------------------------------------------------------
//  2D纹理贴图的非压缩类，增加Alpha边缘扩充，即采用Draw方法绘图

//  构造函数
CEasyGL_2DImageAlphaEdgeObject::CEasyGL_2DImageAlphaEdgeObject()
{
    this->texture_alpha_edge_info.format = GL_RGBA;
    this->texture_alpha_edge_info.width = 0;
    this->texture_alpha_edge_info.height = 0;
    this->texture_alpha_edge_info.pdata = 0;
}

//  析构函数
CEasyGL_2DImageAlphaEdgeObject::~CEasyGL_2DImageAlphaEdgeObject()
{
    if(this->texture_alpha_edge_info.pdata != 0)
    {
        delete[] (unsigned char*)this->texture_alpha_edge_info.pdata;
        this->texture_alpha_edge_info.pdata = 0;
    }
}

//  载入显示图片,根据图片自动计算显示比例
//  参数a_size 表示增加外框边缘的alpha的像素个数
//  成功返回0  失败返回非0
int CEasyGL_2DImageAlphaEdgeObject::LoadImageAlphaEdge(
        std::string filename, 
        int a_size, 
        CEasyGL_2DImageObject::ESizeCalcMode mode,
        float pixel             //  当像素比例时，每个单位的像素值
       )
{
    //  定义返回值
    int re = -1;

    //  定义临时纹理资源结构对象
    CEasyGL_ShaderBase::STextureInfo image_info;
    image_info.format = GL_RGBA;
    image_info.width = 0;
    image_info.height = 0;
    image_info.pdata = 0;
    int channels = 0;
    unsigned char* image_buff = 0;

    //  检查alpha像素个数
    if(a_size < 0) return -4;
 
    //  检查着色器是否合法，当非法
    if(this->pshader == 0) return -3;

    //  尝试载入纹理图片
    image_info.pdata = stbi_load(filename.c_str(),
                                 &image_info.width,
                                 &image_info.height,
                                 &channels,
                                 0
                                );

    //  检测释放载入成功,当载入失败
    if(image_info.pdata == 0) return -1;

    //  当为4通道图像
    if(channels == 4)
    {
        image_info.format = GL_RGBA;
        image_buff = 
            new unsigned char[(image_info.width+a_size*2) * (image_info.height+a_size*2) * 4];
        memset(image_buff, 0x00, (image_info.width+a_size*2) * (image_info.height+a_size*2) * 4);
        int x=0;
        int y=0;
        for(y=0;y<image_info.height;y++)
        {
            for(x=0;x<image_info.width;x++)
            {
                memcpy(
                    image_buff + (y+a_size)*(image_info.width+a_size*2)*4 + (x+a_size)*4, 
                    (unsigned char*)(image_info.pdata) + y*image_info.width*4 + x*4,
                    4
                    );
            }
        }
        stbi_image_free(image_info.pdata);
    }
    //  当为3通道图像
    else if(channels == 3)
    {
        image_info.format = GL_RGBA;
        image_buff = 
            new unsigned char[(image_info.width+a_size*2) * (image_info.height+a_size*2) * 4];
        memset(image_buff, 0x00, (image_info.width+a_size*2) * (image_info.height+a_size*2) * 4);
        int x=0;
        int y=0;
        for(y=0;y<image_info.height;y++)
        {
            for(x=0;x<image_info.width;x++)
            {
                memcpy(
                    image_buff + (y+a_size)*(image_info.width+a_size*2)*4 + (x+a_size)*4, 
                    (unsigned char*)image_info.pdata + y*image_info.width*3 + x*3,
                    3
                    );
                image_buff[(y+a_size)*(image_info.width+a_size*2)*4 + (x+a_size)*4 + 3] = 0xFF;
            }
        }
        stbi_image_free(image_info.pdata);
    }
    //  格式错误
    else
    {
        //  该格式虽然程序中并未考虑，但是需要释放资源
        stbi_image_free(image_info.pdata);
        return -2;
    }

    //  设置缓冲区,并整理格式
    image_info.pdata = image_buff;
    image_info.format = GL_RGBA;
    image_info.width = image_info.width + a_size*2;
    image_info.height = image_info.height + a_size*2;

    //  操作成功
    re = 0;

    //  备份上一次的纹理资源
    pthread_mutex_lock(&this->data_mutex);
    CEasyGL_ShaderBase::STextureInfo last_texture = this->texture_alpha_edge_info;

    //  更新当前纹理数据
    this->texture_alpha_edge_info = image_info;


    //  当为像素比例
    if(mode == ESizeCalcMode_KeepPixel)
    {
        //  定义图像信息的临时变量
        CEasyGL_ShaderBase::STextureInfo tmp_info;

        //  获取图形信息
        tmp_info = this->texture_info;

        //  根据像素信息计算比例
        float x = (tmp_info.width * 1.0f) / pixel;
        float y = (tmp_info.height * 1.0f) / pixel;

        //  赋值设置
        rect_v[10]=x;     //  x
        rect_v[15]=x;     //  x
        rect_v[6]= y;     //  y
        rect_v[16]=y;     //  y

        //  调试对象
        if(this->pDebugObj != 0)
        {
            this->pDebugObj->SetWidthHighFloat(x, y);
        }
    }
    //  当为尺寸比例
    else
    {
        //  定义图像信息的临时变量
        CEasyGL_ShaderBase::STextureInfo tmp_info;

        //  获取图形信息
        tmp_info = this->texture_alpha_edge_info;

        //  当图像信息可用
        if((tmp_info.width > 0) && (tmp_info.height > 0))
        {
            //  当宽度大于高度
            if(tmp_info.width > tmp_info.height)
            {
                float tmp_height = tmp_info.height;
                float tmp_width = tmp_info.width;
                rect_v[10]=1.0f;                       //  x
                rect_v[15]=1.0f;                       //  x
                rect_v[6]= tmp_height / tmp_width;     //  y
                rect_v[16]=tmp_height / tmp_width;     //  y

                //  调试对象
                if(this->pDebugObj != 0)
                {
                    this->pDebugObj->SetWidthHighFloat(1.0f, tmp_height / tmp_width);
                }
            }
            //  当宽度小于高度
            else if(tmp_info.width < tmp_info.height)
            {
                float tmp_height = tmp_info.height;
                float tmp_width = tmp_info.width;
                rect_v[10]=tmp_width / tmp_height;     //  x
                rect_v[15]=tmp_width / tmp_height;     //  x
                rect_v[6]= 1.0f;                       //  y
                rect_v[16]=1.0f;                       //  y

                //  调试对象
                if(this->pDebugObj != 0)
                {
                    this->pDebugObj->SetWidthHighFloat(tmp_width / tmp_height, 1.0f);
                }
            }
            //  否则相等
            else
            {
                rect_v[10]=1.0f;     //  x
                rect_v[15]=1.0f;     //  x
                rect_v[6]= 1.0f;     //  y
                rect_v[16]=1.0f;     //  y

                //  调试对象
                if(this->pDebugObj != 0)
                {
                    this->pDebugObj->SetWidthHighFloat(1.0f, 1.0f);
                }
            }

        }
    }
    pthread_mutex_unlock(&this->data_mutex);

    //  切换完成后，释放之前的资源
    if(last_texture.pdata != 0)
    {
        delete[] (unsigned char*)last_texture.pdata;
        last_texture.pdata = 0;
    }

    //  操作完成
    return re;
}

//  刷新(纹理、坐标数据等)
void CEasyGL_2DImageAlphaEdgeObject::Refresh(void)
{
    //  检查参数是否合法,当合法
    if(this->pshader != 0)
    {
        pthread_mutex_lock(&this->data_mutex);
        this->pshader->SetVertex(this->pvertex, this->vertex_len);
        this->pshader->SetVertexElements(this->rect_v_index, 6);
        this->pshader->Move(move_x, move_y, move_z);
        this->pshader->MoveCenter(move_center_x, move_center_y, move_center_z);
        this->pshader->RotateRad(rotate_rad_x, rotate_rad_y, rotate_rad_z);
        this->pshader->Scale(scale_x, scale_y, scale_z);
        this->pshader->SetBlendColor(color_r, color_g, color_b, color_a);
        this->pshader->SetTexture2D(this->texture_alpha_edge_info.format,  //  纹理图片格式
                                    this->texture_alpha_edge_info.width,   //  宽度
                                    this->texture_alpha_edge_info.height,  //  高度
                                    this->texture_alpha_edge_info.pdata    //  纹理数据首地址
                                   );
        this->pshader->ShaderRender();
        pthread_mutex_unlock(&this->data_mutex);
    }
}

//---------------------------------------------------------------------
//  2D纹理贴图的非压缩类，增加Alpha边缘裁剪，即采用Draw方法绘图

//  构造函数
CEasyGL_2DImageAlphaCutObject::CEasyGL_2DImageAlphaCutObject()
{
}

//  析构函数
CEasyGL_2DImageAlphaCutObject::~CEasyGL_2DImageAlphaCutObject()
{
}

//  载入显示图片,根据图片自动计算显示比例(仅仅支持4通道带有alpha的图片)
//  参数a_size 表示外框边缘裁剪的alpha的像素个数
//  成功返回0  失败返回非0
int CEasyGL_2DImageAlphaCutObject::LoadImageAlphaCut(
        std::string filename, 
        int a_size, 
        CEasyGL_2DImageObject::ESizeCalcMode mode,
        float pixel             //  当像素比例时，每个单位的像素值
       )
{
    //  定义返回值
    int re = -1;

    //  检查alpha裁剪的尺寸
    if(a_size < 0) return -2;

    //  定义临时纹理资源结构对象
    CEasyGL_ShaderBase::STextureInfo image_info;
    image_info.format = GL_RGBA;
    image_info.width = 0;
    image_info.height = 0;
    image_info.pdata = 0;
    int channels = 0;
 
    //  检查着色器是否合法，当非法
    if(this->pshader == 0) return -3;

    //  尝试载入纹理图片
    image_info.pdata = stbi_load(filename.c_str(),
                                 &image_info.width,
                                 &image_info.height,
                                 &channels,
                                 0
                                );

    //  检测释放载入成功,当载入失败
    if(image_info.pdata == 0) return -1;

    //  当为4通道图像
    if(channels == 4)
    {
        image_info.format = GL_RGBA;
    }
    //  格式错误
    else
    {
        //  该格式虽然程序中并未考虑，但是需要释放资源
        stbi_image_free(image_info.pdata);
        return -2;
    }

    //  检查是否符合裁剪要求
    if(((a_size*2) >= image_info.width) || ((a_size*2) >= image_info.height))
    {
        //  裁剪超出范围
        stbi_image_free(image_info.pdata);
        return -2;
    }

    //  执行裁剪
    int xx=0;
    int yy=0;
    int a_cnt=0;
    //  裁剪上边
    for(a_cnt=0;a_cnt<a_size;a_cnt++)
    {
        for(xx=0;xx<(image_info.width);xx++)
        {
            memset((unsigned char*)image_info.pdata + a_cnt*image_info.width*4 + xx*4, 0x00, 4);
        }
    }
    //  裁剪下边
    for(a_cnt=image_info.height-a_size;a_cnt<image_info.height-0;a_cnt++)
    {
        for(xx=0;xx<(image_info.width);xx++)
        {
            memset((unsigned char*)image_info.pdata + a_cnt*image_info.width*4 + xx*4, 0x00, 4);
        }
    }
    //  裁剪左边
    for(yy=0;yy<image_info.height;yy++)
    {
        for(xx=0;xx<a_size;xx++)
        {
            memset((unsigned char*)image_info.pdata + yy*image_info.width*4 + xx*4, 0x00, 4);
        }
    }
    //  裁剪右边
    for(yy=0;yy<image_info.height;yy++)
    {
        for(xx=image_info.width-a_size;xx<image_info.width-0;xx++)
        {
            memset((unsigned char*)image_info.pdata + yy*image_info.width*4 + xx*4, 0x00, 4);
        }
    }

    //  操作成功
    re = 0;

    //  备份上一次的纹理资源
    pthread_mutex_lock(&this->data_mutex);
    CEasyGL_ShaderBase::STextureInfo last_texture = this->texture_info;

    //  更新当前纹理数据
    this->texture_info = image_info;

    //  当为像素比例
    if(mode == ESizeCalcMode_KeepPixel)
    {
        //  定义图像信息的临时变量
        CEasyGL_ShaderBase::STextureInfo tmp_info;

        //  获取图形信息
        tmp_info = this->texture_info;

        //  根据像素信息计算比例
        float x = (tmp_info.width * 1.0f) / pixel;
        float y = (tmp_info.height * 1.0f) / pixel;

        //  赋值设置
        rect_v[10]=x;     //  x
        rect_v[15]=x;     //  x
        rect_v[6]= y;     //  y
        rect_v[16]=y;     //  y

        //  调试对象
        if(this->pDebugObj != 0)
        {
            this->pDebugObj->SetWidthHighFloat(x, y);
        }
    }
    //  当为尺寸比例
    else
    {
        //  定义图像信息的临时变量
        CEasyGL_ShaderBase::STextureInfo tmp_info;

        //  获取图形信息
        tmp_info = this->texture_info;

        //  当图像信息可用
        if((tmp_info.width > 0) && (tmp_info.height > 0))
        {
            //  当宽度大于高度
            if(tmp_info.width > tmp_info.height)
            {
                float tmp_height = tmp_info.height;
                float tmp_width = tmp_info.width;
                rect_v[10]=1.0f;                       //  x
                rect_v[15]=1.0f;                       //  x
                rect_v[6]= tmp_height / tmp_width;     //  y
                rect_v[16]=tmp_height / tmp_width;     //  y

                //  调试对象
                if(this->pDebugObj != 0)
                {
                    this->pDebugObj->SetWidthHighFloat(1.0f, tmp_height / tmp_width);
                }
            }
            //  当宽度小于高度
            else if(tmp_info.width < tmp_info.height)
            {
                float tmp_height = tmp_info.height;
                float tmp_width = tmp_info.width;
                rect_v[10]=tmp_width / tmp_height;     //  x
                rect_v[15]=tmp_width / tmp_height;     //  x
                rect_v[6]= 1.0f;                       //  y
                rect_v[16]=1.0f;                       //  y

                //  调试对象
                if(this->pDebugObj != 0)
                {
                    this->pDebugObj->SetWidthHighFloat(tmp_width / tmp_height, 1.0f);
                }
            }
            //  否则相等
            else
            {
                rect_v[10]=1.0f;     //  x
                rect_v[15]=1.0f;     //  x
                rect_v[6]= 1.0f;     //  y
                rect_v[16]=1.0f;     //  y

                //  调试对象
                if(this->pDebugObj != 0)
                {
                    this->pDebugObj->SetWidthHighFloat(1.0f, 1.0f);
                }
            }

        }
    }
    pthread_mutex_unlock(&this->data_mutex);

    //  切换完成后，释放之前的资源
    if(last_texture.pdata != 0)
    {
        stbi_image_free(last_texture.pdata);
    }

    //  操作完成
    return re;
}

//---------------------------------------------------------------------
//  2D高速高效率纹理贴图，用于RAW的RGB视频播放，即采用Draw方法绘图

//  构造函数
CEasyGL_2DEfficientImageObject::CEasyGL_2DEfficientImageObject()
{
    this->texture_efficient_info0.format = GL_RGB;
    this->texture_efficient_info0.width = 0;
    this->texture_efficient_info0.height = 0;
    this->texture_efficient_info0.pdata = 0;
    this->texture_efficient_info1.format = GL_RGB;
    this->texture_efficient_info1.width = 0;
    this->texture_efficient_info1.height = 0;
    this->texture_efficient_info1.pdata = 0;
    this->current_buffer_index = 0;
}

//  析构函数
CEasyGL_2DEfficientImageObject::~CEasyGL_2DEfficientImageObject()
{
    if(this->texture_efficient_info0.pdata != 0)
    {
        delete[] (unsigned char*)this->texture_efficient_info0.pdata;
        this->texture_efficient_info0.pdata = 0;
    }
    if(this->texture_efficient_info1.pdata != 0)
    {
        delete[] (unsigned char*)this->texture_efficient_info1.pdata;
        this->texture_efficient_info1.pdata = 0;
    }
}

//  创建缓冲区
//  x和y为视频尺寸
//  成功返回0  失败返回非0
int CEasyGL_2DEfficientImageObject::CreateBuffer(
                             int x, int y,
                             CEasyGL_2DImageObject::ESizeCalcMode mode,
                             float pixel             //  当像素比例时，每个单位的像素值
                            )
{
    //  检查输入
    if(x <= 0) return -1;
    if(y <= 0) return -2;

    //  定义临时对象
    CEasyGL_ShaderBase::STextureInfo old_info0;
    CEasyGL_ShaderBase::STextureInfo old_info1;
    CEasyGL_ShaderBase::STextureInfo new_info0;
    CEasyGL_ShaderBase::STextureInfo new_info1;


    //  创建内存
    new_info0.format = GL_RGB;
    new_info0.width = x;
    new_info0.height = y;
    new_info0.pdata = new unsigned char[y * x * 3];
    if(new_info0.pdata == 0) return -3;
    memset(new_info0.pdata, 0x00, y*x*3);
    new_info1.format = GL_RGB;
    new_info1.width = x;
    new_info1.height = y;
    new_info1.pdata = new unsigned char[y * x * 3];
    if(new_info1.pdata == 0) return -4;
    memset(new_info1.pdata, 0x00, y*x*3);

    //  备份现有对象
    pthread_mutex_lock(&this->data_mutex);
    old_info0 = this->texture_efficient_info0;
    old_info1 = this->texture_efficient_info1;

    //  当为像素比例
    if(mode == ESizeCalcMode_KeepPixel)
    {
        //  根据像素信息计算比例
        float x = (new_info0.width * 1.0f) / pixel;
        float y = (new_info0.height * 1.0f) / pixel;

        //  赋值设置
        rect_v[10]=x;     //  x
        rect_v[15]=x;     //  x
        rect_v[6]= y;     //  y
        rect_v[16]=y;     //  y

        //  调试对象
        if(this->pDebugObj != 0)
        {
            this->pDebugObj->SetWidthHighFloat(x, y);
        }
    }
    //  当为尺寸比例
    else
    {
        //  当宽度大于高度
        if(new_info0.width > new_info0.height)
        {
            float tmp_height = new_info0.height;
            float tmp_width = new_info0.width;
            rect_v[10]=1.0f;                       //  x
            rect_v[15]=1.0f;                       //  x
            rect_v[6]= tmp_height / tmp_width;     //  y
            rect_v[16]=tmp_height / tmp_width;     //  y

            //  调试对象
            if(this->pDebugObj != 0)
            {
                this->pDebugObj->SetWidthHighFloat(1.0f, tmp_height / tmp_width);
            }
        }
        //  当宽度小于高度
        else if(new_info0.width < new_info0.height)
        {
            float tmp_height = new_info0.height;
            float tmp_width = new_info0.width;
            rect_v[10]=tmp_width / tmp_height;     //  x
            rect_v[15]=tmp_width / tmp_height;     //  x
            rect_v[6]= 1.0f;                       //  y
            rect_v[16]=1.0f;                       //  y

            //  调试对象
            if(this->pDebugObj != 0)
            {
                this->pDebugObj->SetWidthHighFloat(tmp_width / tmp_height, 1.0f);
            }
        }
        //  否则相等
        else
        {
            rect_v[10]=1.0f;     //  x
            rect_v[15]=1.0f;     //  x
            rect_v[6]= 1.0f;     //  y
            rect_v[16]=1.0f;     //  y

            //  调试对象
            if(this->pDebugObj != 0)
            {
                this->pDebugObj->SetWidthHighFloat(1.0f, 1.0f);
            }
        }

    }

    //  切换
    this->texture_efficient_info0 = new_info0;
    this->texture_efficient_info1 = new_info1;
    pthread_mutex_unlock(&this->data_mutex);

    //  释放上一次的资源
    if(old_info0.pdata != 0)
    {
        delete[] (unsigned char*)old_info0.pdata;
        old_info0.pdata = 0;
    }
    if(old_info1.pdata != 0)
    {
        delete[] (unsigned char*)old_info1.pdata;
        old_info1.pdata = 0;
    }
    
    //  操作成功
    return 0;
}

//  更新画面
//  成功返回0  失败返回非0
int CEasyGL_2DEfficientImageObject::UpdateImage(void* pdat)
{
    //  由于这里是高速操作，所以先读出当前使用的缓冲区是哪一个
    pthread_mutex_lock(&this->data_mutex);
    int cur_index = this->current_buffer_index;
    pthread_mutex_unlock(&this->data_mutex);

    //  当为0缓冲区
    if(cur_index == 0)
    {
        //  将数据复制到1缓冲区中
        //  由于另一线程并为操作1缓冲区，所以不用互斥量，这样复制数据的过程中也不会影响渲染
        memcpy(this->texture_efficient_info1.pdata, 
               pdat, 
               this->texture_efficient_info1.width * this->texture_efficient_info1.height * 3
              );

        //  操作切换
        pthread_mutex_lock(&this->data_mutex);
        this->current_buffer_index = 1;
        pthread_mutex_unlock(&this->data_mutex);

        //  操作成功
        return 0;
    }
    //  当为1缓冲区
    else if(cur_index == 1)
    {
        //  将数据复制到0缓冲区中
        //  由于另一线程并为操作0缓冲区，所以不用互斥量，这样复制数据的过程中也不会影响渲染
        memcpy(this->texture_efficient_info0.pdata, 
               pdat, 
               this->texture_efficient_info0.width * this->texture_efficient_info0.height * 3
              );

        //  操作切换
        pthread_mutex_lock(&this->data_mutex);
        this->current_buffer_index = 0;
        pthread_mutex_unlock(&this->data_mutex);

        //  操作成功
        return 0;
    }
    //  失败
    else
    {
        return -1;
    }
}

//  刷新(纹理、坐标数据等)
void CEasyGL_2DEfficientImageObject::Refresh(void)
{
    //  检查参数是否合法,当合法
    if(this->pshader != 0)
    {
        pthread_mutex_lock(&this->data_mutex);
        this->pshader->SetVertex(this->pvertex, this->vertex_len);
        this->pshader->SetVertexElements(this->rect_v_index, 6);
        this->pshader->Move(move_x, move_y, move_z);
        this->pshader->MoveCenter(move_center_x, move_center_y, move_center_z);
        this->pshader->RotateRad(rotate_rad_x, rotate_rad_y, rotate_rad_z);
        this->pshader->Scale(scale_x, scale_y, scale_z);
        this->pshader->SetBlendColor(color_r, color_g, color_b, color_a);
        if(this->current_buffer_index == 0)
        {
            this->pshader->SetTexture2D(this->texture_efficient_info0.format,  //  纹理图片格式
                                        this->texture_efficient_info0.width,   //  宽度
                                        this->texture_efficient_info0.height,  //  高度
                                        this->texture_efficient_info0.pdata    //  纹理数据首地址
                                       );
        }
        else if(this->current_buffer_index == 1)
        {
            this->pshader->SetTexture2D(this->texture_efficient_info1.format,  //  纹理图片格式
                                        this->texture_efficient_info1.width,   //  宽度
                                        this->texture_efficient_info1.height,  //  高度
                                        this->texture_efficient_info1.pdata    //  纹理数据首地址
                                       );
        }
        this->pshader->ShaderRender();
        pthread_mutex_unlock(&this->data_mutex);
    }
}

//---------------------------------------------------------------------
//  2D纹理贴图的多重采样非压缩类，即采用Draw方法绘图

//  多重采样的Shader
CEasyGL_2DTextureMultiSampleElementShader CEasyGL_2DImageMultiSampleObject::multisample_shader;

//  构造函数
CEasyGL_2DImageMultiSampleObject::CEasyGL_2DImageMultiSampleObject()
{
    //  配置着色器
    this->Init(&this->multisample_shader);
}

//  刷新(纹理、坐标数据等)
void CEasyGL_2DImageMultiSampleObject::Refresh(void)
{
    //  检查参数是否合法,当合法
    if(this->pshader != 0)
    {
        pthread_mutex_lock(&this->data_mutex);
        this->pshader->SetVertex(this->pvertex, this->vertex_len);
        this->pshader->SetVertexElements(this->rect_v_index, 6);
        this->pshader->Move(move_x, move_y, move_z);
        this->pshader->MoveCenter(move_center_x, move_center_y, move_center_z);
        this->pshader->RotateRad(rotate_rad_x, rotate_rad_y, rotate_rad_z);
        this->pshader->Scale(scale_x, scale_y, scale_z);
        this->pshader->SetBlendColor(color_r, color_g, color_b, color_a);
        this->pshader->SetTexture2D(this->texture_info.format,          //  纹理图片格式
                                    this->texture_info.width,           //  宽度
                                    this->texture_info.height,          //  高度
                                    this->texture_info.pdata            //  纹理数据首地址
                                   );

        //  用于抗锯齿
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        this->pshader->ShaderRender();
        pthread_mutex_unlock(&this->data_mutex);
    }
}

//---------------------------------------------------------------------
//  简单的框架类

//  静态线程框架运行函数
static void* EasyGL_Static_ThreadRunFunc(void* pin)
{
    //  获取本次调用的操作对象
    CEasyGL_PlatformBase* ppf = (CEasyGL_PlatformBase*)pin;

    //  执行，并返回结果
    return ppf->thread_run(pin);
}

//  构造函数
CEasyGL_SimplePlatform::CEasyGL_SimplePlatform()
{
    this->OptObjVec.clear();
    this->thread_handle = 0;
    this->thread_ctrl_flag = false;
    this->thread_run_flag = false;
    this->fps = 0.0f;
    this->pWinSys = 0;
    pthread_mutex_init(&this->fps_mutex, NULL);
    pthread_mutex_init(&this->thread_flag_mutex, NULL);
}

//  析构函数
CEasyGL_SimplePlatform::~CEasyGL_SimplePlatform()
{
    pthread_mutex_destroy(&this->fps_mutex);
    pthread_mutex_destroy(&this->thread_flag_mutex);
}

//  添加操作对象
//  成功返回0，失败返回非0
int CEasyGL_SimplePlatform::AddOperationObject(CEasyGL_OperationObjectBase* poo)
{
    //  当输入操作对象指针异常
    if(poo == 0) return -1;

    //  添加
    this->OptObjVec.insert(this->OptObjVec.end(), poo);

    //  操作成功
    return 0;
}

//  框架初始化，初始化时，需要指定视窗系统
//  成功返回0，失败返回非0
int CEasyGL_SimplePlatform::PlatformInit(CEasyGL_WindowSystemBase* pws)
{
    //  检查输入视窗系统
    if(pws == 0) return -1;

    //  添加视窗系统到本类对象中
    this->pWinSys = pws;

    //  操作完成
    return 0; 
}

//  开始渲染线程
//  成功返回0，失败返回非0
int CEasyGL_SimplePlatform::StartRenderThread(void)
{
    //  当线程正在运行
    bool tmp_bool = false;
    pthread_mutex_lock(&this->thread_flag_mutex);
    tmp_bool = this->thread_run_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);
    if(tmp_bool)  return -1;

    //  当线程启动命令已经发送
    tmp_bool = false;
    pthread_mutex_lock(&this->thread_flag_mutex);
    tmp_bool = this->thread_ctrl_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);
    if(tmp_bool)  return -2;

    //  发送启动线程控制命令
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_ctrl_flag = true;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  启动线程
    pthread_create(&this->thread_handle, 
                   NULL, 
                   EasyGL_Static_ThreadRunFunc, 
                   (void*)this
                  );

    //  操作成功
    return 0;
}

//  结束渲染线程
//  成功返回0，失败返回非0
int CEasyGL_SimplePlatform::StopRenderThread(void)
{
    //  当线程控制 和 实际运行状态都为停止，则跳过后续
    bool tmp_bool1 = false;
    bool tmp_bool2 = false;
    pthread_mutex_lock(&this->thread_flag_mutex);
    tmp_bool1 = this->thread_ctrl_flag;
    tmp_bool2 = this->thread_run_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);
    if((tmp_bool1 == false) && (tmp_bool2 == false)) return 0;

    //  发送停止标志
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_ctrl_flag = false;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  阻塞等待线程结束
    pthread_join(this->thread_handle, NULL);

    //  设置已经结束(冗余操作)
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_run_flag = false;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  操作成功
    return 0;
}

//  阻塞等待渲染结束,用于退出时等待，并释放资源前的判断
void CEasyGL_SimplePlatform::WaitForRenderEnd(void)
{
    this->StopRenderThread();
}

//  获取当前渲染帧率
float CEasyGL_SimplePlatform::GetFPS(void)
{
    float re = 0.0f;
    
    pthread_mutex_lock(&this->fps_mutex);
    re = this->fps;
    pthread_mutex_unlock(&this->fps_mutex);

    return re;
}

//  获取线程运行状态
bool CEasyGL_SimplePlatform::IsRun(void)
{
    bool tmp_bool = false;

    pthread_mutex_lock(&this->thread_flag_mutex);
    tmp_bool = this->thread_run_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    return tmp_bool;
}

//  线程主循环
void* CEasyGL_SimplePlatform::thread_run(void* arg)
{
    //  设置线程已经启动
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_run_flag = true;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  检查视窗系统
    if(this->pWinSys == 0) 
    {
        //  终止线程
        pthread_mutex_lock(&this->thread_flag_mutex);
        this->thread_run_flag = false;
        pthread_mutex_unlock(&this->thread_flag_mutex);
        return NULL;
    }

    //  当线程控制标志有效
    bool ctrl_flag = false;
    pthread_mutex_lock(&this->thread_flag_mutex);
    ctrl_flag = this->thread_ctrl_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  初始化视窗系统
    int re = this->pWinSys->WindowSystemInit();

    //  检查视窗系统是否可用
    if(re != 0)
    {
        //  打印错误代码
        printf("Window System Error!! Code = %d\r\n", re);

        //  终止线程
        pthread_mutex_lock(&this->thread_flag_mutex);
        this->thread_run_flag = false;
        pthread_mutex_unlock(&this->thread_flag_mutex);
        return NULL;
    }

    //  编译着色器
    int len = this->OptObjVec.size();
    int i=0;
    for(i=0;i<len;i++)
    {
        this->OptObjVec.at(i)->CompileShader();
    }

    //  打印OpenGLES信息
    printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
    printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );
    
    //  计算渲染时间相关
    struct timeval last_time;           //  上一次渲染时间
    struct timeval now_time;            //  现在时间

    //  获取当前时间
    gettimeofday(&last_time, NULL);

    //  开始线程主循环
    while(ctrl_flag)
    {
        //  通知视窗系统开始渲染一帧
        this->pWinSys->RenderBegin();

        //  清除上一帧显示内容
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //  设置背景色
        glClearColor(EASYGL_BACKGOUND_COLOR_R,
                     EASYGL_BACKGOUND_COLOR_G,
                     EASYGL_BACKGOUND_COLOR_B,
                     EASYGL_BACKGOUND_COLOR_A
                    );

        //  开启多重采样
        glEnable(GL_MULTISAMPLE);

        //  遍历渲染全部对象
        int len = this->OptObjVec.size();
        int i=0;
        for(i=0;i<len;i++)
        {
            //  当该对象显示
            if(this->OptObjVec.at(i)->GetShowStatus())
            {
                this->OptObjVec.at(i)->Refresh();     //  刷新纹理和坐标数据等
                //  取消刷新和渲染的分开机制
                //  采用刷新的同时,再没有释放互斥锁的时候,进行渲染
                //  解决极低概率出现的崩溃问题
                //  原有机制漏洞:
                //  当本线程刷新完成数据后,但是主线程抢占到资源,进行更新纹理数据
                //  然后由于数据没有及时刷新,导致被释放资源的内存被使用,导致崩溃
                //  现在是更新完数据后,不给主线程抢占的机会,而直接刷新完成一帧后
                //  再释放资源给主线程,即使被多次意外打断,也不会访问到错误的内存
                //this->OptObjVec.at(i)->Render();      //  调用着色器渲染
            }
        }

/*
        //  绑定读取缓冲区
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFramebuffer[0]);

        //  绑定绘制缓冲区
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebuffer[0]);

        glBlitFramebuffer(0, 0, width, height,
                          0, 0, width, height,
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                          GL_LINEAR);

        //  绑定采样渲染缓冲区
        glBindRenderbuffer(GL_RENDERBUFFER, defaultRenderbuffer[0]);
*/

        //  通知视窗系统结束渲染一帧
        this->pWinSys->RenderEnd();

        //  重新更新获取当前线程控制标志
        pthread_mutex_lock(&this->thread_flag_mutex);
        ctrl_flag = this->thread_ctrl_flag;
        pthread_mutex_unlock(&this->thread_flag_mutex);

        //  释放CPU
        usleep(1);

        //  计算渲染时间
        gettimeofday(&now_time, NULL);
        long timespan = 0L;
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
        double tmp_fps = 1000000.0 / timespan;
        pthread_mutex_lock(&this->fps_mutex);
        this->fps = tmp_fps;
        pthread_mutex_unlock(&this->fps_mutex);
        last_time = now_time;        //  更新时间
    }

    //  关闭窗口
    this->pWinSys->CloseWindows();   

    //  退出线程
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_run_flag = false;
    pthread_mutex_unlock(&this->thread_flag_mutex);
    return NULL;
}

#if EN_FFMPEG
//=========================================================视频解码相关
//---------------------------------------------------------------------
//  视频解码框架类

//  静态线程框架运行函数
static void* EasyGL_Static_FFmpegRunFunc(void* pin)
{
    //  获取本次调用的操作对象
    CEasyGL_FFmpaePlatform* ppf = (CEasyGL_FFmpaePlatform*)pin;

    //  执行，并返回结果
    return ppf->thread_run(pin);
}

//  构造函数
CEasyGL_FFmpaePlatform::CEasyGL_FFmpaePlatform()
{
    this->ffmpeg_context.p_fmt_ctx = NULL;
    this->ffmpeg_context.p_codec_ctx = NULL;
    this->ffmpeg_context.p_codec_par = NULL;
    this->ffmpeg_context.p_codec = NULL;
    this->ffmpeg_context.p_frm_raw = NULL;
    this->ffmpeg_context.p_packet = NULL;
    this->ffmpeg_context.buf_size = 0;
    this->ffmpeg_context.buffer = NULL;
    this->ffmpeg_context.v_idx = -1;

    this->ffmpeg_context.FrameRate = 0.0f;
    this->ffmpeg_context.Width = 0;
    this->ffmpeg_context.Height = 0;
    this->ffmpeg_context.TotalFrame = 0UL;
    this->ffmpeg_context.play_mode = EPlayMode_Cycle;   //  默认循环播放

    this->DecodeIsValid = false;

    this->thread_ctrl_flag = false;
    this->thread_run_flag = false;

    this->play_status = EPlayerStatus_NoConf;

    this->scale_mode = CEasyGL_2DImageObject::ESizeCalcMode_KeepPixel;
    this->scale_pixel = 500.0f;

    this->video_filename = "";

    pthread_mutex_init(&this->thread_flag_mutex, NULL);
    pthread_mutex_init(&this->play_status_mutex, NULL);
}

//  析构函数
CEasyGL_FFmpaePlatform::~CEasyGL_FFmpaePlatform()
{
    this->Close();
    pthread_mutex_destroy(&this->play_status_mutex);
    pthread_mutex_destroy(&this->thread_flag_mutex);
}

//  配置播放
//  成功返回0，失败返回非0
int CEasyGL_FFmpaePlatform::Config(
        std::string filename,                   //  播放视频文件
        CEasyGL_2DEfficientImageObject* pobj,   //  高效贴图对象
        CEasyGL_2DImageObject::ESizeCalcMode mode, //  显示比例模式
        float pixel                             //  当像素比例时，每个单位的像素值
       )
{
    //  定义返回值
    int re = 0;

    //  检查贴图对象
    if(pobj == 0) return -20;

    //  检查显示比例
    if((mode == CEasyGL_2DImageObject::ESizeCalcMode_KeepPixel) && (pixel <= 0.0f)) return -21;

    //  配置贴图对象
    this->pObject = pobj;

    //  打开视频文件
    re = avformat_open_input(&this->ffmpeg_context.p_fmt_ctx,
                             filename.c_str(),
                             NULL, NULL
                            );
    if(re != 0)
    {
        printf("ERROR:avformat_open_input()\r\n");
        this->ffmpeg_context.p_fmt_ctx = 0;
        return -1;
    }

    //  配置视频文件
    this->video_filename = filename;

    //  搜索流信息
    re = avformat_find_stream_info(this->ffmpeg_context.p_fmt_ctx,
                                   NULL
                                  );
    if(re != 0)
    {
        if(this->ffmpeg_context.p_fmt_ctx != 0)
        {
            avformat_close_input(&this->ffmpeg_context.p_fmt_ctx);
            this->ffmpeg_context.p_fmt_ctx = 0;
        }
        printf("ERROR:avformat_find_stream_info()\r\n");
        return -2;
    }

    //  打印流信息
    //av_dump_format(this->ffmpeg_context.p_fmt_ctx, 0, filename.c_str(), 0);

    //  查找第一个视频流
    this->ffmpeg_context.v_idx = -1;
    int i=0;
    for (i=0; i<this->ffmpeg_context.p_fmt_ctx->nb_streams; i++)
    {
        if (this->ffmpeg_context.p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            this->ffmpeg_context.v_idx = i;
            this->ffmpeg_context.TotalFrame = this->ffmpeg_context.p_fmt_ctx->streams[i]->nb_frames;
            printf("Find a video stream, index %d\r\n", this->ffmpeg_context.v_idx);
            printf("Total Frame = %ld\r\n", this->ffmpeg_context.TotalFrame);
            this->ffmpeg_context.FrameRate = 
                (this->ffmpeg_context.p_fmt_ctx->streams[i]->avg_frame_rate.num * 1.0f)/
                    this->ffmpeg_context.p_fmt_ctx->streams[i]->avg_frame_rate.den;
            break;
        }
    }
    if (this->ffmpeg_context.v_idx == -1)
    {
        printf("ERROR:Cann't find a video stream\r\n");
        if(this->ffmpeg_context.p_fmt_ctx != 0)
        {
            avformat_close_input(&this->ffmpeg_context.p_fmt_ctx);
            this->ffmpeg_context.p_fmt_ctx = 0;
        }
        return -3;
    }
    printf("frame_rate = %f fps\r\n", this->ffmpeg_context.FrameRate);

    //  为视频流构造解码器
    //  获取解码器参数
    this->ffmpeg_context.p_codec_par = 
        this->ffmpeg_context.p_fmt_ctx->streams[this->ffmpeg_context.v_idx]->codecpar;

    //  获取解码器
    this->ffmpeg_context.p_codec = avcodec_find_decoder(this->ffmpeg_context.p_codec_par->codec_id);
    if(this->ffmpeg_context.p_codec == NULL)
    {
        if(this->ffmpeg_context.p_fmt_ctx != 0)
        {
            avformat_close_input(&this->ffmpeg_context.p_fmt_ctx);
            this->ffmpeg_context.p_fmt_ctx = 0;
        }
        printf("ERROR:avcodec_find_decoder()\r\n");
        return -4;
    }

    //  构造解码器
    this->ffmpeg_context.p_codec_ctx = avcodec_alloc_context3(this->ffmpeg_context.p_codec);
    if(this->ffmpeg_context.p_codec_ctx == NULL)
    {
        if(this->ffmpeg_context.p_fmt_ctx != 0)
        {
            avformat_close_input(&this->ffmpeg_context.p_fmt_ctx);
            this->ffmpeg_context.p_fmt_ctx = 0;
        }
        printf("ERROR:avcodec_alloc_context3()\r\n");
        return -5;
    }

    //  解码器参数初始化
    re = avcodec_parameters_to_context(this->ffmpeg_context.p_codec_ctx, 
                                       this->ffmpeg_context.p_codec_par
                                      );
    if(re < 0)
    {
        if(this->ffmpeg_context.p_codec_ctx != 0)
        {
            avcodec_free_context(&this->ffmpeg_context.p_codec_ctx);
            this->ffmpeg_context.p_codec_ctx = 0;
        }
        if(this->ffmpeg_context.p_fmt_ctx != 0)
        {
            avformat_close_input(&this->ffmpeg_context.p_fmt_ctx);
            this->ffmpeg_context.p_fmt_ctx = 0;
        }
        printf("ERROR:avcodec_parameters_to_context()\r\n");
        return -6;
    }

    //  打开解码器
    re = avcodec_open2(this->ffmpeg_context.p_codec_ctx, this->ffmpeg_context.p_codec, NULL);
    if(re < 0)
    {
        if(this->ffmpeg_context.p_codec_ctx != 0)
        {
            avcodec_free_context(&this->ffmpeg_context.p_codec_ctx);
            this->ffmpeg_context.p_codec_ctx = 0;
        }
        if(this->ffmpeg_context.p_fmt_ctx != 0)
        {
            avformat_close_input(&this->ffmpeg_context.p_fmt_ctx);
            this->ffmpeg_context.p_fmt_ctx = 0;
        }
        printf("ERROR:avcodec_open2()\r\n");
        return -7;
    }

    //  分配帧
    this->ffmpeg_context.p_frm_raw = av_frame_alloc();
    if(this->ffmpeg_context.p_frm_raw == NULL)
    {
        if(this->ffmpeg_context.p_codec_ctx != 0)
        {
            avcodec_free_context(&this->ffmpeg_context.p_codec_ctx);
            this->ffmpeg_context.p_codec_ctx = 0;
        }
        if(this->ffmpeg_context.p_fmt_ctx != 0)
        {
            avformat_close_input(&this->ffmpeg_context.p_fmt_ctx);
            this->ffmpeg_context.p_fmt_ctx = 0;
        }
        printf("ERROR:av_frame_alloc()  YUV\r\n");
        return -8;
    }

    //  获取缓冲区大小
    this->ffmpeg_context.buf_size = av_image_get_buffer_size(
                                        AV_PIX_FMT_YUV420P,
                                        this->ffmpeg_context.p_codec_ctx->width,
                                        this->ffmpeg_context.p_codec_ctx->height,
                                        1
                                       );
    this->ffmpeg_context.buffer = (unsigned char*)av_malloc(this->ffmpeg_context.buf_size);
    if(this->ffmpeg_context.buffer == NULL)
    {
        if(this->ffmpeg_context.p_frm_raw != 0)
        {
            av_frame_free(&this->ffmpeg_context.p_frm_raw);
            this->ffmpeg_context.p_frm_raw = 0;
        }
        if(this->ffmpeg_context.p_codec_ctx != 0)
        {
            avcodec_free_context(&this->ffmpeg_context.p_codec_ctx);
            this->ffmpeg_context.p_codec_ctx = 0;
        }
        if(this->ffmpeg_context.p_fmt_ctx != 0)
        {
            avformat_close_input(&this->ffmpeg_context.p_fmt_ctx);
            this->ffmpeg_context.p_fmt_ctx = 0;
        }
        printf("ERROR:av_malloc()\r\n");
        return -9;
    }

    //  申请视频包空间
    this->ffmpeg_context.p_packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    if(this->ffmpeg_context.p_packet == NULL)
    {
        if(this->ffmpeg_context.buffer != 0)
        {
            av_free(this->ffmpeg_context.buffer);
            this->ffmpeg_context.buffer = 0;
        }
        if(this->ffmpeg_context.p_frm_raw != 0)
        {
            av_frame_free(&this->ffmpeg_context.p_frm_raw);
            this->ffmpeg_context.p_frm_raw = 0;
        }
        if(this->ffmpeg_context.p_codec_ctx != 0)
        {
            avcodec_free_context(&this->ffmpeg_context.p_codec_ctx);
            this->ffmpeg_context.p_codec_ctx = 0;
        }
        if(this->ffmpeg_context.p_fmt_ctx != 0)
        {
            avformat_close_input(&this->ffmpeg_context.p_fmt_ctx);
            this->ffmpeg_context.p_fmt_ctx = 0;
        }
        printf("ERROR:av_malloc()  AVPacket\r\n");
        return -11;
    }

    //  配置宽度、高度
    this->ffmpeg_context.Width = this->ffmpeg_context.p_codec_ctx->width;
    this->ffmpeg_context.Height = this->ffmpeg_context.p_codec_ctx->height;
    printf("width=%d, height=%d\r\n", this->ffmpeg_context.Width, this->ffmpeg_context.Height);

    //  配置显示比例
    this->scale_mode = mode;
    this->scale_pixel = pixel;

    //  解码器可用
    this->DecodeIsValid = true;

    //  设置播放状态,停止状态
    this->SetPlayStatus(EPlayerStatus_Stop);

    //  操作成功
    return 0;
}

//  关闭播放
//  成功返回0，失败返回非0
int CEasyGL_FFmpaePlatform::Close(void)
{
    //  告知不可用
    this->DecodeIsValid = false;

    //  结束线程
    this->StopPlayerThread();

    //  依次释放资源
    if(this->ffmpeg_context.p_packet != 0)
    {
        av_packet_unref(this->ffmpeg_context.p_packet);
        this->ffmpeg_context.p_packet = 0;
    }
    if(this->ffmpeg_context.buffer != 0)
    {
        av_free(this->ffmpeg_context.buffer);
        this->ffmpeg_context.buffer = 0;
    }
    if(this->ffmpeg_context.p_frm_raw != 0)
    {
        av_frame_free(&this->ffmpeg_context.p_frm_raw);
        this->ffmpeg_context.p_frm_raw = 0;
    }
    if(this->ffmpeg_context.p_codec_ctx != 0)
    {
        avcodec_free_context(&this->ffmpeg_context.p_codec_ctx);
        this->ffmpeg_context.p_codec_ctx = 0;
    }
    if(this->ffmpeg_context.p_fmt_ctx != 0)
    {
        avformat_close_input(&this->ffmpeg_context.p_fmt_ctx);
        this->ffmpeg_context.p_fmt_ctx = 0;
    }

    return 0;
}

//  开始播放线程
//  参数 mode 为播放模式，默认循环播放
//  成功返回0，失败返回非0
int CEasyGL_FFmpaePlatform::StartPlayerThread(CEasyGL_FFmpaePlatform::EPlayMode mode)
{
    //  当线程正在运行
    bool tmp_bool = false;
    pthread_mutex_lock(&this->thread_flag_mutex);
    tmp_bool = this->thread_run_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);
    if(tmp_bool)  return -1;

    //  当线程启动命令已经发送
    tmp_bool = false;
    pthread_mutex_lock(&this->thread_flag_mutex);
    tmp_bool = this->thread_ctrl_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);
    if(tmp_bool)  return -2;

    //  发送启动线程控制命令
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_ctrl_flag = true;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  设置播放状态,正在播放状态
    this->SetPlayStatus(EPlayerStatus_Playing);

    //  设置播放模式
    this->ffmpeg_context.play_mode = mode;

    //  启动线程
    pthread_create(&this->thread_handle, 
                   NULL, 
                   EasyGL_Static_FFmpegRunFunc, 
                   (void*)this
                  );

    //  操作成功
    return 0;
}

//  结束播放线程
//  成功返回0，失败返回非0
int CEasyGL_FFmpaePlatform::StopPlayerThread(void)
{
    //  当线程控制 和 实际运行状态都为停止，则跳过后续
    bool tmp_bool1 = false;
    bool tmp_bool2 = false;
    pthread_mutex_lock(&this->thread_flag_mutex);
    tmp_bool1 = this->thread_ctrl_flag;
    tmp_bool2 = this->thread_run_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);
    if((tmp_bool1 == false) && (tmp_bool2 == false)) return 0;

    //  发送停止标志
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_ctrl_flag = false;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  阻塞等待线程结束
    pthread_join(this->thread_handle, NULL);

    //  设置已经结束(冗余操作)
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_run_flag = false;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  设置播放状态,停止状态
    this->SetPlayStatus(EPlayerStatus_Stop);

    //  操作成功
    return 0;
}

//  获取线程运行状态
bool CEasyGL_FFmpaePlatform::IsRun(void)
{
    bool tmp_bool = false;

    pthread_mutex_lock(&this->thread_flag_mutex);
    tmp_bool = this->thread_run_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    return tmp_bool;
}

//  线程主循环
void* CEasyGL_FFmpaePlatform::thread_run(void* arg)
{
    //  设置线程已经启动
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_run_flag = true;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  检查播放系统是否可用
    if(!this->DecodeIsValid) 
    {
        //  终止线程
        pthread_mutex_lock(&this->thread_flag_mutex);
        this->thread_run_flag = false;
        pthread_mutex_unlock(&this->thread_flag_mutex);
        return NULL;
    }

    //  当线程控制标志有效
    bool ctrl_flag = false;
    pthread_mutex_lock(&this->thread_flag_mutex);
    ctrl_flag = this->thread_ctrl_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  配置贴图对象
    this->pObject->CreateBuffer(this->ffmpeg_context.Width, this->ffmpeg_context.Height,
                                this->scale_mode, this->scale_pixel
                               );

    //  根据视频的FPS值，来计算每一帧的延时
    long total_delay_time = 1000000L / this->ffmpeg_context.FrameRate;
    printf("Each FPS = %ld us\r\n", total_delay_time);

    //  计算渲染时间相关
    struct timeval last_time;           //  上一次渲染时间
    struct timeval now_time;            //  现在时间
    struct timeval begin_player_time;   //  开始播放时间
    unsigned long frame_cnt = 0UL;      //  帧计数器 

    //  申请显示内存
    unsigned char* pvideo_buffer = new unsigned char[this->ffmpeg_context.Height*this->ffmpeg_context.Width*3];
    memset(pvideo_buffer, 0xFF, this->ffmpeg_context.Height*this->ffmpeg_context.Width*3);

    //  获取当前时间
    gettimeofday(&last_time, NULL);
    gettimeofday(&begin_player_time, NULL);

    //  开始线程主循环
    while(ctrl_flag)
    {
        //  重新更新获取当前线程控制标志
        pthread_mutex_lock(&this->thread_flag_mutex);
        ctrl_flag = this->thread_ctrl_flag;
        pthread_mutex_unlock(&this->thread_flag_mutex);

        //  从视频文件中获取一个包
        while(av_read_frame(this->ffmpeg_context.p_fmt_ctx, this->ffmpeg_context.p_packet) == 0)
        {
            //  当读取到一帧视频的时候，则跳出
            if(this->ffmpeg_context.p_packet->stream_index == this->ffmpeg_context.v_idx)
            {
                break;
            }
        }

        //  视频解码
        //  向解码器喂数据
        int re = avcodec_send_packet(this->ffmpeg_context.p_codec_ctx, 
                                     this->ffmpeg_context.p_packet
                                    );
        if(re != 0)
        {
            pthread_mutex_lock(&this->thread_flag_mutex);
            this->thread_ctrl_flag = false;
            pthread_mutex_unlock(&this->thread_flag_mutex);
            ctrl_flag = false;
            break;
        }

        //  接收解码器输出的数据
        re = avcodec_receive_frame(this->ffmpeg_context.p_codec_ctx,
                                   this->ffmpeg_context.p_frm_raw
                                  );
        if(re != 0)
        {
            if (re == AVERROR_EOF)
            {
                printf("avcodec_receive_frame(): the decoder has been fully flushed\n");
                av_seek_frame(this->ffmpeg_context.p_fmt_ctx, -1, 0, AVSEEK_FLAG_BACKWARD);
                continue;
            }
            else if (re == AVERROR(EAGAIN))
            {
                printf("avcodec_receive_frame(): output is not available in this state - "
                        "user must try to send new input\n");
                continue;
            }
            else if (re == AVERROR(EINVAL))
            {
                printf("avcodec_receive_frame(): codec not opened, or it is an encoder\n");
            }
            else
            {
                printf("avcodec_receive_frame(): legitimate decoding errors\n");
            }
            pthread_mutex_lock(&this->thread_flag_mutex);
            this->thread_ctrl_flag = false;
            pthread_mutex_unlock(&this->thread_flag_mutex);
            ctrl_flag = false;
            break;
        }

        //  格式转换
        this->YUV2RGB(this->ffmpeg_context.p_frm_raw->width,
                      this->ffmpeg_context.p_frm_raw->height,
                      this->ffmpeg_context.p_frm_raw->data,
                      this->ffmpeg_context.p_frm_raw->linesize,
                      pvideo_buffer
                     );

        //  贴图
        this->pObject->UpdateImage(pvideo_buffer);

        //  完成一帧
        frame_cnt++;

//  高精度延时计算
//  每一帧进行计算
#if 0
        //  计算渲染时间
        long timespan = 0L;
        do
        {
            //  释放CPU
            usleep(1);

            //  计算上一次渲染 和 当前的时间差
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
        //  当时间差超过1帧的时间
        while(total_delay_time > timespan);
        last_time = now_time;        //  更新时间

//  带有冗余功能的叠加计算
#else
        //  计算渲染时间
        long timespan = 0L;
        long total_player_time = 0L;
        do
        {
            //  释放CPU
            usleep(1);

            //  计算视频刚刚开始播放 和 当前的时间差
            gettimeofday(&now_time, NULL);
            if(now_time.tv_sec != begin_player_time.tv_sec)
            {
                timespan = (now_time.tv_sec - begin_player_time.tv_sec - 1L) * 1000000L;
                timespan += 1000000L - begin_player_time.tv_usec;
                timespan += now_time.tv_usec;
            }
            else
            {
                timespan = now_time.tv_usec - begin_player_time.tv_usec;
            }

            //  根据已经播放的帧数量，得出理论上应该延时的时间
            total_player_time = frame_cnt * (1000000L / this->ffmpeg_context.FrameRate);
        }
        //  当时间差超过1帧的时间
        while(total_player_time > timespan);
#endif

        //  检查是否播放完成,当播放完成
        if(frame_cnt >= (this->ffmpeg_context.TotalFrame-1))
        {
            //  检查播放模式，当为单次播放
            if(this->ffmpeg_context.play_mode == EPlayMode_Once)
            {
                //  跳出循环，线程自动退出
                pthread_mutex_lock(&this->thread_flag_mutex);
                this->thread_ctrl_flag = false;
                pthread_mutex_unlock(&this->thread_flag_mutex);
                ctrl_flag = false;
                break;
            }
            //  当为循环播放模式
            else if(this->ffmpeg_context.play_mode == EPlayMode_Cycle)
            {
                //  重置播放
                this->ResetFFmpeg();
    
                //  获取当前时间
                gettimeofday(&last_time, NULL);
                gettimeofday(&begin_player_time, NULL);

                //  清零帧计数器
                frame_cnt = 0UL;
            }
            //  模式错误
            else
            {
                //  跳出循环，线程自动退出
                pthread_mutex_lock(&this->thread_flag_mutex);
                this->thread_ctrl_flag = false;
                pthread_mutex_unlock(&this->thread_flag_mutex);
                ctrl_flag = false;
                break;
            }
        }
    }

    //  释放显示内存
    delete[] (unsigned char*)pvideo_buffer;

    //  退出线程
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_run_flag = false;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  设置播放状态,停止状态
    this->SetPlayStatus(EPlayerStatus_Stop);

    return NULL;
}

//  图像转换
void CEasyGL_FFmpaePlatform::YUV2RGB(int in_width,
                                     int in_height,
                                     unsigned char** in_pdat_yuv,
                                     int* in_pline_size,
                                     unsigned char* out_pdat_rgb
                                    )
{
    int X = in_width;
    int Y = in_height;
    int line_cnt = 0;
    for(line_cnt=0;line_cnt<Y;line_cnt++)
    {
        int x_cnt = 0;
        for(x_cnt=0;x_cnt<X;x_cnt++)
        {
            int y = in_pdat_yuv[0][x_cnt + line_cnt * in_pline_size[0]] & 0x0FF;
            int pos = ((x_cnt+0)/2) + ((line_cnt+0)/2) * in_pline_size[1];
            int u = in_pdat_yuv[1][pos] & 0x0FF;
            int v = in_pdat_yuv[2][pos] & 0x0FF;
            int r = y + 1.402 * (v-128.0);
            int g = y - 0.34414 * (u-128.0) - 0.71414*(v-128.0);
            int b = y + 1.772*(u-128.0);
            if(r>255) r=255;  if(r<=0) r=0;
            if(g>255) g=255;  if(g<=0) g=0;
            if(b>255) b=255;  if(b<=0) b=0;
            out_pdat_rgb[x_cnt*3 + line_cnt*in_width*3 + 0] = r;
            out_pdat_rgb[x_cnt*3 + line_cnt*in_width*3 + 1] = g;
            out_pdat_rgb[x_cnt*3 + line_cnt*in_width*3 + 2] = b;
        }
    }
}

//  获取播放状态
CEasyGL_FFmpaePlatform::EPlayerStatus CEasyGL_FFmpaePlatform::GetPlayStatus(void)
{
    EPlayerStatus re = EPlayerStatus_NoConf;
    pthread_mutex_lock(&this->play_status_mutex);
    re = this->play_status;
    pthread_mutex_unlock(&this->play_status_mutex);
    return re;
}


//  设置播放状态
void CEasyGL_FFmpaePlatform::SetPlayStatus(CEasyGL_FFmpaePlatform::EPlayerStatus st)
{
    pthread_mutex_lock(&this->play_status_mutex);
    this->play_status = st;
    pthread_mutex_unlock(&this->play_status_mutex);
}

//  重置ffmpeg,用于循环播放
void CEasyGL_FFmpaePlatform::ResetFFmpeg(void)
{
//  重启播放的方式配置
//  简单低风险的配置
#if 1
    //  重启解码器
    avcodec_close(this->ffmpeg_context.p_codec_ctx);
    avcodec_open2(this->ffmpeg_context.p_codec_ctx, this->ffmpeg_context.p_codec, NULL);
//  复杂彻底的配置
#else
    //  依次释放资源
    if(this->ffmpeg_context.p_packet != 0)
    {
        av_packet_unref(this->ffmpeg_context.p_packet);
        this->ffmpeg_context.p_packet = 0;
    }
    if(this->ffmpeg_context.buffer != 0)
    {
        av_free(this->ffmpeg_context.buffer);
        this->ffmpeg_context.buffer = 0;
    }
    if(this->ffmpeg_context.p_frm_raw != 0)
    {
        av_frame_free(&this->ffmpeg_context.p_frm_raw);
        this->ffmpeg_context.p_frm_raw = 0;
    }
    if(this->ffmpeg_context.p_codec_ctx != 0)
    {
        avcodec_free_context(&this->ffmpeg_context.p_codec_ctx);
        this->ffmpeg_context.p_codec_ctx = 0;
    }
    if(this->ffmpeg_context.p_fmt_ctx != 0)
    {
        avformat_close_input(&this->ffmpeg_context.p_fmt_ctx);
        this->ffmpeg_context.p_fmt_ctx = 0;
    }

    //  重新配置
    this->Config(this->video_filename,
                 this->pObject,
                 this->scale_mode,
                 this->scale_pixel
                );
#endif

    //  从头播放
    av_seek_frame(this->ffmpeg_context.p_fmt_ctx, 
                  this->ffmpeg_context.v_idx, 
                  0, 
                  AVSEEK_FLAG_BACKWARD
                 );
}



#endif  //  EN_FFMPEG

//================================================FreeType2字体引擎相关
#if EN_FREETYPE2

//  构造函数
CEasyGL_2DTextObject::CEasyGL_2DTextObject()
{
    this->texture_text_info.format = GL_RGBA;
    this->texture_text_info.width = 0;
    this->texture_text_info.height = 0;
    this->texture_text_info.pdata = 0;

    this->text_context.angle_deg = 0.0;
    this->text_context.font_width = 0;
    this->text_context.font_height = 0;
    this->text_context.frame_width = 0;
    this->text_context.span = 0;

    this->text_valid = false;
}

//  析构函数
CEasyGL_2DTextObject::~CEasyGL_2DTextObject()
{
    if(this->texture_text_info.pdata != 0)
    {
        delete[] (unsigned char*)this->texture_text_info.pdata;
        this->texture_text_info.pdata = 0;
    }

    if(this->text_valid)
    {
        FT_Done_Face(this->text_context.face);
        FT_Done_FreeType(this->text_context.library);
    }
}

//  配置文字信息
//  成功返回0  失败返回非0
int CEasyGL_2DTextObject::Config(
                   std::string filename,            //  TTF字体文件
                   int font_width,                  //  单个字符的宽度
                   int font_height,                 //  单个字符的高度
                   int frame_width,                 //  整个字符串外框的留边宽度
                   int span,                        //  每个文字的间距空隙
                   double angle_deg                 //  单个字符的旋转角度
                  )
{
    //  定义返回的错误变量
    FT_Error error;

    //  如果已经初始化过
    if(this->text_valid)
    {
        FT_Done_Face(this->text_context.face);
        FT_Done_FreeType(this->text_context.library);
    }

    //  初始化字体引擎库
    error = FT_Init_FreeType(&this->text_context.library);
    if(error)
    {
        return -1;
    }

    //  载入字体文件
    error = FT_New_Face(this->text_context.library,
                        filename.c_str(),
                        0,
                        &this->text_context.face
                       );
    if(error)
    {
        FT_Done_FreeType(this->text_context.library);
        return -2;
    }

    //  设置文字大小
    this->text_context.font_width = font_width;
    this->text_context.font_height = font_height;
    error = FT_Set_Pixel_Sizes(this->text_context.face,
                               font_width,
                               font_height
                              );

    //  获取操作参数
    this->text_context.slot = this->text_context.face->glyph;

    //  配置画笔
    this->text_context.pen.x = 0;
    this->text_context.pen.y = 0;

    //  配置矩阵
    this->text_context.angle_deg = angle_deg;
    double angle_rad = (this->text_context.angle_deg / 360.0) * M_PI * 2.0;
    this->text_context.matrix.xx = (FT_Fixed)( cos(angle_rad) * 0x10000L);
    this->text_context.matrix.xy = (FT_Fixed)(-sin(angle_rad) * 0x10000L);
    this->text_context.matrix.yx = (FT_Fixed)( sin(angle_rad) * 0x10000L);
    this->text_context.matrix.yy = (FT_Fixed)( cos(angle_rad) * 0x10000L);

    //  配置信息
    this->text_context.span = span;
    this->text_context.frame_width = frame_width;

    //  文字可用
    this->text_valid = true;

    //  操作成功
    return 0;
}

//  绘制文字
//  成功返回0  失败返回非0
int CEasyGL_2DTextObject::DrawText(
                     const wchar_t* pstr,           //  基于UNICODE编码的文字
                     int r, int g, int b, int a,    //  绘制文字时的颜色
                     CEasyGL_2DImageObject::ESizeCalcMode mode, //  显示比例模式
                     float pixel                    //  当像素比例时，每个单位的像素值
                    )
{
    //  定义临时变量
    CEasyGL_ShaderBase::STextureInfo new_info;
    new_info.format = GL_RGBA;
    new_info.width = 0;
    new_info.height = 0;
    new_info.pdata = 0;
    CEasyGL_ShaderBase::STextureInfo old_info;
    old_info.format = GL_RGBA;
    old_info.width = 0;
    old_info.height = 0;
    old_info.pdata = 0;

    //  对颜色数值进行限制处理
    if(r<0) r=0; if(r>255) r=255;
    if(g<0) g=0; if(g>255) g=255;
    if(b<0) b=0; if(b>255) b=255;
    if(a<0) a=0; if(a>255) a=255;

    //  当文字不可用
    if(!this->text_valid) return -1;

    //  检查字符串
    if(pstr == 0) return -2;

    //  获取字符串长度
    int len = wcslen(pstr);
    if(len <= 0) return -3;

    //  配置画笔
    this->text_context.pen.x = 0;
    this->text_context.pen.y = 0;

    //  根据文字大小申请显示缓冲区
    //  计算最大缓冲区大小
    int buff_w = 0;
    int buff_h = 0;
    int buff_size = 0;
    //  当间距大于0
    if(this->text_context.span > 0)
    {
        buff_w = 
            //  长
            len *           //  文字数量
            (this->text_context.font_width + this->text_context.span) +  //  每个字算间距的宽度
            this->text_context.frame_width * 2   //  左右框
            ;
        buff_h = 
            this->text_context.font_height + this->text_context.frame_width * 2;
        buff_size = buff_w * buff_h * 4;
    }
    //  当间距小于0
    else
    {
        buff_w = 
            //  长
            len *           //  文字数量
            (this->text_context.font_width) +  //  每个字不算间距的宽度
            this->text_context.frame_width * 2   //  左右框
            ;
        buff_h = 
            this->text_context.font_height + this->text_context.frame_width * 2;
        buff_size = buff_w * buff_h * 4;
    }

    //  申请内存
    new_info.format = GL_RGBA;
    new_info.width = buff_w;
    new_info.height = buff_h;
    new_info.pdata = new unsigned char[buff_size];

    //  设置文字衬底颜色(与文字显示颜色相同)
    int x=0;
    int y=0;
    for(y=0;y<buff_h;y++)
    {
        for(x=0;x<buff_w;x++)
        {
            ((unsigned char*)new_info.pdata)[y*buff_w*4 + x*4 + 0] = r;
            ((unsigned char*)new_info.pdata)[y*buff_w*4 + x*4 + 1] = g;
            ((unsigned char*)new_info.pdata)[y*buff_w*4 + x*4 + 2] = b;
            ((unsigned char*)new_info.pdata)[y*buff_w*4 + x*4 + 3] = 0;
        }
    }

    //  绘制每一个文字
    int char_cnt=0;
    int buff_y=this->text_context.frame_width;
    int buff_x=this->text_context.frame_width;
    int font_y=0;
    int font_x=0;
    int font_w=0;
    int font_h=0;
    unsigned char* pbitmap=0;
    FT_Error error;
    for(char_cnt=0;char_cnt<len;char_cnt++)
    {
        //  应用画笔和矩阵
        FT_Set_Transform(this->text_context.face,
                         &this->text_context.matrix,
                         &this->text_context.pen
                        );

        //  获取该文字的位图
        error = FT_Load_Char(this->text_context.face,
                             pstr[char_cnt],
                             FT_LOAD_RENDER
                            );
        if(error) continue;

        //  获取该文字信息
        font_w = this->text_context.slot->bitmap.width;
        font_h = this->text_context.slot->bitmap.rows;
        pbitmap = this->text_context.slot->bitmap.buffer;

        //  渲染该文字
        for(font_y=0;font_y<font_h;font_y++)
        {
            for(font_x=0;font_x<font_w;font_x++)
            {
                unsigned char val = pbitmap[font_y*font_w + font_x];
                if(val != 0)
                {
                    buff_y = (buff_h -
                             (font_h - font_y)) - this->text_context.frame_width;
                    buff_x = this->text_context.frame_width +
                             this->text_context.slot->bitmap_left + font_x;
                    //  为了提高绘制效率,去掉没必要的绘制动作
                    //((unsigned char*)new_info.pdata)[buff_y*buff_w*4 + buff_x*4 + 0] = r;
                    //((unsigned char*)new_info.pdata)[buff_y*buff_w*4 + buff_x*4 + 1] = g;
                    //((unsigned char*)new_info.pdata)[buff_y*buff_w*4 + buff_x*4 + 2] = b;
                    ((unsigned char*)new_info.pdata)[buff_y*buff_w*4 + buff_x*4 + 3] = a;
                }
                else
                {
                    //  为了提高绘制效率,去掉没必要的绘制动作
                    //buff_y = (buff_h - 
                    //         (font_h - font_y)) - this->text_context.frame_width;
                    //buff_x = this->text_context.frame_width +
                    //         this->text_context.slot->bitmap_left + font_x;
                    //((unsigned char*)new_info.pdata)[buff_y*buff_w*4 + buff_x*4 + 0] = 0;
                    //((unsigned char*)new_info.pdata)[buff_y*buff_w*4 + buff_x*4 + 1] = 0;
                    //((unsigned char*)new_info.pdata)[buff_y*buff_w*4 + buff_x*4 + 2] = 0;
                    //((unsigned char*)new_info.pdata)[buff_y*buff_w*4 + buff_x*4 + 3] = 0;
                }
            }
        }

        //  计算下一个字符的画笔位置
        this->text_context.pen.x += this->text_context.slot->advance.x;
        this->text_context.pen.y += this->text_context.slot->advance.y;
    }

    //  计算几何对象尺寸
    pthread_mutex_lock(&this->data_mutex);     //  开始改变渲染参数



    //  当为像素比例
    if(mode == ESizeCalcMode_KeepPixel)
    {
        //  根据像素信息计算比例
        float x = (new_info.width * 1.0f) / pixel;
        float y = (new_info.height * 1.0f) / pixel;

        //  赋值设置
        rect_v[10]=x;     //  x
        rect_v[15]=x;     //  x
        rect_v[6]= y;     //  y
        rect_v[16]=y;     //  y

        //  调试对象
        if(this->pDebugObj != 0)
        {
            this->pDebugObj->SetWidthHighFloat(x, y);
        }
    }
    //  当为尺寸比例
    else
    {
        //  当宽度大于高度
        if(new_info.width > new_info.height)
        {
            float tmp_height = new_info.height;
            float tmp_width = new_info.width;
            rect_v[10]=1.0f;                       //  x
            rect_v[15]=1.0f;                       //  x
            rect_v[6]= tmp_height / tmp_width;     //  y
            rect_v[16]=tmp_height / tmp_width;     //  y

            //  调试对象
            if(this->pDebugObj != 0)
            {
                this->pDebugObj->SetWidthHighFloat(1.0f, tmp_height / tmp_width);
            }
        }
        //  当宽度小于高度
        else if(new_info.width < new_info.height)
        {
            float tmp_height = new_info.height;
            float tmp_width = new_info.width;
            rect_v[10]=tmp_width / tmp_height;     //  x
            rect_v[15]=tmp_width / tmp_height;     //  x
            rect_v[6]= 1.0f;                       //  y
            rect_v[16]=1.0f;                       //  y

            //  调试对象
            if(this->pDebugObj != 0)
            {
                this->pDebugObj->SetWidthHighFloat(tmp_width / tmp_height, 1.0f);
            }
        }
        //  否则相等
        else
        {
            rect_v[10]=1.0f;     //  x
            rect_v[15]=1.0f;     //  x
            rect_v[6]= 1.0f;     //  y
            rect_v[16]=1.0f;     //  y

            //  调试对象
            if(this->pDebugObj != 0)
            {
                this->pDebugObj->SetWidthHighFloat(1.0f, 1.0f);
            }
        }
    }

    //  备份现有缓冲区内容
    old_info = this->texture_text_info;

    //  执行切换
    this->texture_text_info = new_info;
    pthread_mutex_unlock(&this->data_mutex);      //  结束改变渲染参数

    //  检查并释放上一次视频缓冲区内容
    if(old_info.pdata != 0)
    {
        delete[] (unsigned char*)old_info.pdata;
        old_info.pdata = 0;
    }

    //  操作成功
    return 0;
}               
                
//  刷新(纹理、坐标数据等)
void CEasyGL_2DTextObject::Refresh(void)
{
    //  检查参数是否合法,当合法
    if(this->pshader != 0)
    {
        pthread_mutex_lock(&this->data_mutex);
        this->pshader->SetVertex(this->pvertex, this->vertex_len);
        this->pshader->SetVertexElements(this->rect_v_index, 6);
        this->pshader->Move(move_x, move_y, move_z);
        this->pshader->MoveCenter(move_center_x, move_center_y, move_center_z);
        this->pshader->RotateRad(rotate_rad_x, rotate_rad_y, rotate_rad_z);
        this->pshader->Scale(scale_x, scale_y, scale_z);
        this->pshader->SetBlendColor(color_r, color_g, color_b, color_a);
        this->pshader->SetTexture2D(this->texture_text_info.format,  //  纹理图片格式
                                    this->texture_text_info.width,   //  宽度
                                    this->texture_text_info.height,  //  高度
                                    this->texture_text_info.pdata    //  纹理数据首地址
                                   );


        //  用于去除文字阴影
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        this->pshader->ShaderRender();
        pthread_mutex_unlock(&this->data_mutex);
    }
}

#endif  //  EN_FREETYPE2

//=========================================================调试图层相关
//---------------------------------------------------------------------
//  定义调试图层类

//  自己的Shader
CEasyGL_OnlyGeometryElementShader EasyGL_2DDebugObj::my_geometry_shader;

//  构造函数
EasyGL_2DDebugObj::EasyGL_2DDebugObj()
{
    //  配置顶点
    rect_v_debug[0]= 0.0f;rect_v_debug[1]= 0.0f;rect_v_debug[2]= -EASYGL_DEBUG_Z_MIN; 
    rect_v_debug[3]= 0.0f;rect_v_debug[4]= 1.0f;rect_v_debug[5]= -EASYGL_DEBUG_Z_MIN; 
    rect_v_debug[6]= 1.0f;rect_v_debug[7]= 0.0f;rect_v_debug[8]= -EASYGL_DEBUG_Z_MIN; 
    rect_v_debug[9]= 1.0f;rect_v_debug[10]=1.0f;rect_v_debug[11]=-EASYGL_DEBUG_Z_MIN; 

    //  配置元素
    rect_v_index_debug[0]=3;rect_v_index_debug[1]= 2;rect_v_index_debug[2]= 0;
    rect_v_index_debug[3]=3;rect_v_index_debug[4]= 1;rect_v_index_debug[5]= 0;
    rect_v_index_debug[6]=1;rect_v_index_debug[7]= 2;rect_v_index_debug[8]= 3;

    //  配置着色器
    this->Init(&this->my_geometry_shader);

    //  载入顶点信息
    this->ConfigVertex((GLfloat*)this->rect_v_debug, 12);

    //  配置调试对象
    this->pDebugObj = 0;
}

//  析构函数
EasyGL_2DDebugObj::~EasyGL_2DDebugObj()
{
}

//  设置长宽尺寸(调试用的浮点值)
void EasyGL_2DDebugObj::SetWidthHighFloat(float x, float y)
{
    pthread_mutex_lock(&this->data_mutex);
    rect_v_debug[6]=x;      //  x
    rect_v_debug[9]=x;      //  x
    rect_v_debug[4]= y;     //  y
    rect_v_debug[10]=y;     //  y
    pthread_mutex_unlock(&this->data_mutex);
}

//  刷新(纹理、坐标数据等)
void EasyGL_2DDebugObj::Refresh(void)
{
    //  检查参数是否合法,当合法
    if(this->pshader != 0)
    {
        pthread_mutex_lock(&this->data_mutex);
        this->pshader->SetVertex(this->pvertex, 12);
        this->pshader->SetVertexElements(this->rect_v_index_debug, 9);
        this->pshader->Move(move_x, move_y, move_z);
        this->pshader->MoveCenter(move_center_x, move_center_y, move_center_z);
        this->pshader->RotateRad(rotate_rad_x, rotate_rad_y, rotate_rad_z);
        this->pshader->Scale(scale_x, scale_y, scale_z);
        this->pshader->SetBlendColor(color_r, color_g, color_b, color_a);
        this->pshader->SetTexture2D(this->texture_info.format,          //  纹理图片格式
                                    this->texture_info.width,           //  宽度
                                    this->texture_info.height,          //  高度
                                    this->texture_info.pdata            //  纹理数据首地址
                                   );
        this->pshader->ShaderRender();
        pthread_mutex_unlock(&this->data_mutex);
    }
}

//---------------------------------------------------------------------
//  控制台位置调试类

//  非按回车就能获取键盘字符,类似windows下的getch函数
char linux_getch(void)
{
 
    // 保存并修改终端参数
    struct termios stored_settings;
    struct termios new_settings;
    tcgetattr (0, &stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;
    tcsetattr (0, TCSANOW, &new_settings);
 
    int ret = 0;
    char c;
 
    c = getchar();
    putchar('\b'); // 删除回显
 
    //printf("input:  [%c]\n", c);
 
    tcsetattr (0, TCSANOW, &stored_settings); // 恢复终端参数
 
    return c; 
}

//  静态线程框架运行函数
static void* EasyGL_Static_ConsoleDebugThreadRunFunc(void* pin)
{
    //  获取本次调用的操作对象
    EasyGL_ConsolePositionDebug* ppf = (EasyGL_ConsolePositionDebug*)pin;

    //  执行，并返回结果
    return ppf->thread_run(pin);
}

//  构造函数
EasyGL_ConsolePositionDebug::EasyGL_ConsolePositionDebug()
{
    this->pObjDebug = 0;
    this->thread_handle = 0;
    this->thread_ctrl_flag = false;
    this->thread_run_flag = false;

    this->pos_x = 0.0f;
    this->pos_y = 0.0f;
    this->center_x = 0.0f;
    this->center_y = 0.0f;
    this->scale_x = 1.0f;
    this->scale_y = 1.0f;
    this->scale_z = 1.0f;
    this->rotate_deg_z = 0.0f;
    this->transparent = 1.0f;

    this->AuxLineShow = true;
    this->DebugObjectShow = true;

    this->precision = EPrecisionType_0_1;

    pthread_mutex_init(&this->thread_flag_mutex, NULL);
}

//  析构函数
EasyGL_ConsolePositionDebug::~EasyGL_ConsolePositionDebug()
{
    pthread_mutex_destroy(&this->thread_flag_mutex);
}

//  配置被调试的物体
void EasyGL_ConsolePositionDebug::ConfigDebug(
                             CEasyGL_2DImageObject* pobj,
                             float aux_line_color_r,
                             float aux_line_color_g,
                             float aux_line_color_b,
                             float aux_line_color_a,
                             bool aux_line_show 
                            )
{
    this->pObjDebug = pobj;

    //  当被调试物体有效
    if(this->pObjDebug != 0)
    {
        //  设置辅助线颜色
        this->AuxLineObject.SetColor(aux_line_color_r,
                                     aux_line_color_g, 
                                     aux_line_color_b,
                                     aux_line_color_a
                                    );

        //  绑定辅助线到被调试物体
        this->pObjDebug->ConfigDebug(&this->AuxLineObject);

        //  设定辅助线显示
        this->AuxLineShow = aux_line_show;
        this->AuxLineObject.Show(this->AuxLineShow);
    }
}

//  获取辅助线对象
CEasyGL_2DImageObject* EasyGL_ConsolePositionDebug::GetAuxLineObject(void)
{
    //  当被调试物体有效
    if(this->pObjDebug != 0)
    {
        return &this->AuxLineObject;
    }
    //  无效
    else
    {
        return 0;
    }
}

//  开始线程
//  成功返回0，失败返回非0
int EasyGL_ConsolePositionDebug::StartThread(void)
{
    //  当线程正在运行
    bool tmp_bool = false;
    pthread_mutex_lock(&this->thread_flag_mutex);
    tmp_bool = this->thread_run_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);
    if(tmp_bool)  return -1;

    //  当线程启动命令已经发送
    tmp_bool = false;
    pthread_mutex_lock(&this->thread_flag_mutex);
    tmp_bool = this->thread_ctrl_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);
    if(tmp_bool)  return -2;

    //  发送启动线程控制命令
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_ctrl_flag = true;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  启动线程
    pthread_create(&this->thread_handle, 
                   NULL, 
                   EasyGL_Static_ConsoleDebugThreadRunFunc, 
                   (void*)this
                  );

    //  操作成功
    return 0;
}

//  结束线程
//  成功返回0，失败返回非0
int EasyGL_ConsolePositionDebug::StopThread(void)
{
    //  当线程控制 和 实际运行状态都为停止，则跳过后续
    bool tmp_bool1 = false;
    bool tmp_bool2 = false;
    pthread_mutex_lock(&this->thread_flag_mutex);
    tmp_bool1 = this->thread_ctrl_flag;
    tmp_bool2 = this->thread_run_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);
    if((tmp_bool1 == false) && (tmp_bool2 == false)) return 0;

    //  发送停止标志
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_ctrl_flag = false;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  阻塞等待线程结束
    pthread_join(this->thread_handle, NULL);

    //  设置已经结束(冗余操作)
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_run_flag = false;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  操作成功
    return 0;
}

//  阻塞等待渲染结束,用于退出时等待，并释放资源前的判断
void EasyGL_ConsolePositionDebug::WaitForThreadEnd(void)
{
    this->StopThread();
}

//  获取线程运行状态
bool EasyGL_ConsolePositionDebug::IsRun(void)
{
    bool tmp_bool = false;

    pthread_mutex_lock(&this->thread_flag_mutex);
    tmp_bool = this->thread_run_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    return tmp_bool;
}

//  线程主循环
void* EasyGL_ConsolePositionDebug::thread_run(void* arg)
{
    //  设置线程已经启动
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_run_flag = true;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  当线程控制标志有效
    bool ctrl_flag = false;
    pthread_mutex_lock(&this->thread_flag_mutex);
    ctrl_flag = this->thread_ctrl_flag;
    pthread_mutex_unlock(&this->thread_flag_mutex);

    //  开始线程主循环
    while(ctrl_flag)
    {
        //  重新更新获取当前线程控制标志
        pthread_mutex_lock(&this->thread_flag_mutex);
        ctrl_flag = this->thread_ctrl_flag;
        pthread_mutex_unlock(&this->thread_flag_mutex);

        //  释放CPU
        usleep(1);

        //  从控制台获取一个字符
        int input = linux_getch();

        //  当调试对象可用
        if(this->pObjDebug != 0)
        {
            //  检查字符功能
            switch(input)
            {
                //-------------------------------------------  辅助线显示开关控制
                case 't':
                case 'T':
                {
                    if(this->AuxLineShow)
                    {
                        this->AuxLineShow = false;
                    }
                    else
                    {
                        this->AuxLineShow = true;
                    }
                    this->AuxLineObject.Show(this->AuxLineShow);
                }break;

                //-------------------------------------------  调试物体本身显示开关控制
                case 'r':
                case 'R':
                {
                    if(this->DebugObjectShow)
                    {
                        this->DebugObjectShow = false;
                    }
                    else
                    {
                        this->DebugObjectShow = true;
                    }
                    this->pObjDebug->Show(this->DebugObjectShow);
                }break;

                //-------------------------------------------  数据归零
                case 'o':
                case 'O':
                case '0':
                {
                    this->pos_x = 0.0f;
                    this->pos_y = 0.0f;
                    this->center_x = 0.0f;
                    this->center_y = 0.0f;
                    this->scale_x = 1.0f;
                    this->scale_y = 1.0f;
                    this->scale_z = 1.0f;
                    this->rotate_deg_z = 0.0f;
                    this->Update2D();
                }break;

                //-------------------------------------------  透明度 变不透明
                case '[':
                case '{':
                {
                    this->transparent = this->add(this->transparent);
                    this->Update2D();
                }break;

                //-------------------------------------------  透明度 变透明
                case ']':
                case '}':
                {
                    this->transparent = this->sub(this->transparent);
                    this->Update2D();
                }break;

                //-------------------------------------------  平移  上
                case 'w':
                case 'W':
                {
                    this->pos_y = this->add(this->pos_y);
                    this->Update2D();
                }break;

                //-------------------------------------------  平移  下
                case 's':
                case 'S':
                {
                    this->pos_y = this->sub(this->pos_y);
                    this->Update2D();
                }break;

                //-------------------------------------------  平移  左
                case 'a':
                case 'A':
                {
                    this->pos_x = this->sub(this->pos_x);
                    this->Update2D();
                }break;

                //-------------------------------------------  平移  右
                case 'd':
                case 'D':
                {
                    this->pos_x = this->add(this->pos_x);
                    this->Update2D();
                }break;

                //-------------------------------------------  中心  上
                case 'i':
                case 'I':
                {
                    this->center_y = this->add(this->center_y);
                    this->Update2D();
                }break;

                //-------------------------------------------  中心  下
                case 'k':
                case 'K':
                {
                    this->center_y = this->sub(this->center_y);
                    this->Update2D();
                }break;

                //-------------------------------------------  中心  左
                case 'j':
                case 'J':
                {
                    this->center_x = this->sub(this->center_x);
                    this->Update2D();
                }break;

                //-------------------------------------------  中心  右
                case 'l':
                case 'L':
                {
                    this->center_x = this->add(this->center_x);
                    this->Update2D();
                }break;

                //-------------------------------------------  旋转  左(逆时针)
                case 'q':
                case 'Q':
                {
                    this->rotate_deg_z = this->add(this->rotate_deg_z / 100.0f) * 100.0f;
                    this->Update2D();
                }break;

                //-------------------------------------------  旋转  右(顺时针)
                case 'e':
                case 'E':
                {
                    this->rotate_deg_z = this->sub(this->rotate_deg_z / 100.0f) * 100.0f;
                    this->Update2D();
                }break;

                //-------------------------------------------  缩小
                case 'z':
                case 'Z':
                {
                    this->scale_x = this->sub(this->scale_x);
                    this->scale_y = this->sub(this->scale_y);
                    this->scale_z = this->sub(this->scale_z);
                    this->Update2D();
                }break;

                //-------------------------------------------  放大
                case 'x':
                case 'X':
                {
                    this->scale_x = this->add(this->scale_x);
                    this->scale_y = this->add(this->scale_y);
                    this->scale_z = this->add(this->scale_z);
                    this->Update2D();
                }break;

                //-------------------------------------------  分辨率变小(精度变高)
                case ',':
                case '<':
                {
                    if(this->precision == EPrecisionType_0_1)
                    {
                        this->precision = EPrecisionType_0_01;
                        printf("Precision is 0.01 M\r\n");
                    }
                    else if(this->precision == EPrecisionType_0_01)
                    {
                        this->precision = EPrecisionType_0_001;
                        printf("Precision is 0.001 M\r\n");
                    }
                    else if(this->precision == EPrecisionType_0_001)
                    {
                        this->precision = EPrecisionType_0_0001;
                        printf("Precision is 0.0001 M\r\n");
                    }
                    else if(this->precision == EPrecisionType_0_0001)
                    {
                        printf("Precision is 0.0001 M\r\n");
                    }
                    else
                    {
                        this->precision = EPrecisionType_0_1;
                        printf("Precision is 0.1 M\r\n");
                    }
                }break;

                //-------------------------------------------  分辨率变大(精度变低)
                case '.':
                case '>':
                {
                    if(this->precision == EPrecisionType_0_1)
                    {
                        printf("Precision is 0.1 M\r\n");
                    }
                    else if(this->precision == EPrecisionType_0_01)
                    {
                        this->precision = EPrecisionType_0_1;
                        printf("Precision is 0.1 M\r\n");
                    }
                    else if(this->precision == EPrecisionType_0_001)
                    {
                        this->precision = EPrecisionType_0_01;
                        printf("Precision is 0.01 M\r\n");
                    }
                    else if(this->precision == EPrecisionType_0_0001)
                    {
                        this->precision = EPrecisionType_0_001;
                        printf("Precision is 0.001 M\r\n");
                    }
                    else
                    {
                        this->precision = EPrecisionType_0_1;
                        printf("Precision is 0.1 M\r\n");
                    }
                }break;

                //-------------------------------------------  打印参数
                case 'p':
                case 'P':
                {
                    printf("------console position debug------\r\n");
                    printf("    pos_x = %f\r\n",        this->pos_x);
                    printf("    pos_y = %f\r\n",        this->pos_y);
                    printf("    center_x = %f\r\n",     this->center_x);
                    printf("    center_y = %f\r\n",     this->center_y);
                    printf("    scale_x = %f\r\n",      this->scale_x);
                    printf("    scale_y = %f\r\n",      this->scale_y);
                    printf("    scale_z = %f\r\n",      this->scale_z);
                    printf("    rotate_deg_z = %f\r\n", this->rotate_deg_z);
                    printf("    transparent = %f\r\n",  this->transparent);
                    if(this->precision == EPrecisionType_0_1)
                    {
                        printf("    Precision is 0.1 M\r\n");
                    }
                    else if(this->precision == EPrecisionType_0_01)
                    {
                        printf("    Precision is 0.01 M\r\n");
                    }
                    else if(this->precision == EPrecisionType_0_001)
                    {
                        printf("    Precision is 0.001 M\r\n");
                    }
                    else if(this->precision == EPrecisionType_0_0001)
                    {
                        printf("    Precision is 0.0001 M\r\n");
                    }
                    else
                    {
                        printf("    Precision is Error!!\r\n");
                    }
                }break;

                //-------------------------------------------  错误
                default:
                {
                }break;
            }
        }
        
    }

    //  退出线程
    pthread_mutex_lock(&this->thread_flag_mutex);
    this->thread_run_flag = false;
    pthread_mutex_unlock(&this->thread_flag_mutex);
    return NULL;
}


//  根据精度做自增
float EasyGL_ConsolePositionDebug::add(float in)
{
    switch(this->precision)
    {
        case EPrecisionType_0_1:    in += 0.1f;    break;
        case EPrecisionType_0_01:   in += 0.01f;   break;
        case EPrecisionType_0_001:  in += 0.001f;  break;
        case EPrecisionType_0_0001: in += 0.0001f; break;
        default:                                   break;
    }
    return in;
}

//  根据精度做自减
float EasyGL_ConsolePositionDebug::sub(float in)
{
    switch(this->precision)
    {
        case EPrecisionType_0_1:    in -= 0.1f;    break;
        case EPrecisionType_0_01:   in -= 0.01f;   break;
        case EPrecisionType_0_001:  in -= 0.001f;  break;
        case EPrecisionType_0_0001: in -= 0.0001f; break;
        default:                                   break;
    }
    return in;
}

//  更新平面部分
void EasyGL_ConsolePositionDebug::Update2D(void)
{
    //  当调试对象可用
    if(this->pObjDebug != 0)
    {
        float z = pObjDebug->GetZ();
        pObjDebug->Move(this->pos_x, this->pos_y, z);
        pObjDebug->MoveCenter(this->center_x, this->center_y, 0.0f);
        pObjDebug->RotateDeg(0.0f, 0.0f, this->rotate_deg_z);
        pObjDebug->Scale(this->scale_x, this->scale_y, this->scale_z);
        pObjDebug->SetColor(1.0f, 1.0f, 1.0f, this->transparent);
    }
}


//  为了实现直接对框架追加对象进行赋值
//  采用基于操作对象基类实现
//  可选基本初始化
//  成功返回0，失败返回非0
int EasyGL_ConsolePositionDebug::Init(CEasyGL_ShaderBase* ps)
{
    return 0;
}

//  编译着色器
int EasyGL_ConsolePositionDebug::CompileShader(void)
{
    this->AuxLineObject.CompileShader();
}

//  刷新(纹理、坐标数据)
void EasyGL_ConsolePositionDebug::Refresh(void)
{
    this->AuxLineObject.Refresh();
}

//  渲染操作
void EasyGL_ConsolePositionDebug::Render(void)
{
    this->AuxLineObject.Render();
}

//  设置显示状态
void EasyGL_ConsolePositionDebug::Show(bool status)
{
    this->AuxLineObject.Show(status);
}

//  获取当前显示状态
bool EasyGL_ConsolePositionDebug::GetShowStatus(void)
{
    return this->AuxLineObject.GetShowStatus();
}

//  平移
void EasyGL_ConsolePositionDebug::Move(float x, float y, float z)
{
    //  由于几何操作由被调试对象接管,所以这里不作处理
}

//  移动物体的中心点
void EasyGL_ConsolePositionDebug::MoveCenter(float x, float y, float z)
{
    //  由于几何操作由被调试对象接管,所以这里不作处理
}

//  以角度旋转
void EasyGL_ConsolePositionDebug::RotateDeg(float x, float y, float z)
{
    //  由于几何操作由被调试对象接管,所以这里不作处理
}

//  以弧度旋转
void EasyGL_ConsolePositionDebug::RotateRad(float x, float y, float z)
{
    //  由于几何操作由被调试对象接管,所以这里不作处理
}

//  缩放
void EasyGL_ConsolePositionDebug::Scale(float x, float y, float z)
{
    //  由于几何操作由被调试对象接管,所以这里不作处理
}

//  获取当前对象的Z轴位置(调试用途)
float EasyGL_ConsolePositionDebug::GetZ(void)
{
    return this->AuxLineObject.GetZ();
}


