

//  程序版本  REV 0.1

//  包含头文件
#include "DashGL.h"

//  Shader
#include "ShaderSrc.h"

//---------------------------------------------------------------------
//  圆形表盘渐变拖尾着色器
//  构造函数
CDashGL_CircularGradientTrackShader::CDashGL_CircularGradientTrackShader()
{
    this->start_angle_deg = 0.0f;
    this->track_angle_deg = 0.0f;

    pthread_mutex_init(&this->track_mutex, NULL);
}

//  析构函数
CDashGL_CircularGradientTrackShader::~CDashGL_CircularGradientTrackShader()
{
    pthread_mutex_destroy(&this->track_mutex);
}

//  设置轨迹拖尾
void CDashGL_CircularGradientTrackShader::SetTrack(float start, float track)
{
    pthread_mutex_lock(&this->move_mutex);
    this->start_angle_deg = start;
    this->track_angle_deg = track;
    pthread_mutex_unlock(&this->move_mutex);
}

//  获取顶点着色器源码
const GLchar* CDashGL_CircularGradientTrackShader::GetVertexShaderSrc(void)
{
    return CDashGL_CircularGradientTrackShader_VertexShader_c;
}

//  片段着色器源码
const GLchar* CDashGL_CircularGradientTrackShader::GetFragmentShaderSrc(void)
{
    return CDashGL_CircularGradientTrackShader_FragmentShader_c;
}

//  着色器渲染,由操作对象调用，不可用由框架直接调用
void CDashGL_CircularGradientTrackShader::ShaderRender(void)
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

    //  设置拖尾参数
    pthread_mutex_lock(&this->track_mutex);
    int id_start_angle_deg = glGetUniformLocation(this->ShaderHandle, "start_angle_deg");
    int id_track_angle_deg = glGetUniformLocation(this->ShaderHandle, "track_angle_deg");
    glUniform1f(id_start_angle_deg, this->start_angle_deg);
    glUniform1f(id_track_angle_deg, this->track_angle_deg);
    pthread_mutex_unlock(&this->track_mutex);

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
std::string CDashGL_CircularGradientTrackShader::ClassName(void)
{
    std::string re_str = "CDashGL_CircularGradientTrackShader";
    return re_str;
}

//---------------------------------------------------------------------
//  圆形表盘实心拖尾着色器

//  片段着色器源码
const GLchar* CDashGL_CircularSolidTrackShader::GetFragmentShaderSrc(void)
{
    return CDashGL_CircularSolidTrackShader_FragmentShader_c;
}

//  当前类的名字
std::string CDashGL_CircularSolidTrackShader::ClassName(void)
{
    std::string re_str = "CDashGL_CircularSolidTrackShader";
    return re_str;
}

//---------------------------------------------------------------------
//  圆形表盘渐变拖尾对象

//  定义自己的Shader
CDashGL_CircularGradientTrackShader CDashGL_CircularGradientTrackObject::gradient_shader;

//  构造函数
CDashGL_CircularGradientTrackObject::CDashGL_CircularGradientTrackObject()
{
    this->start_angle_deg = 0.0f;
    this->track_angle_deg = 0.0f;

    //  配置着色器
    this->Init(&this->gradient_shader);
}

//  设置轨迹拖尾
void CDashGL_CircularGradientTrackObject::SetTrack(float start, float track)
{
    pthread_mutex_lock(&this->data_mutex);
    this->start_angle_deg = start;
    this->track_angle_deg = track;
    pthread_mutex_unlock(&this->data_mutex);
}

//  刷新(纹理、坐标数据等)
void CDashGL_CircularGradientTrackObject::Refresh(void)
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
        this->gradient_shader.SetTrack(this->start_angle_deg, this->track_angle_deg);
        this->pshader->ShaderRender();
        pthread_mutex_unlock(&this->data_mutex);
    }
}

//---------------------------------------------------------------------
//  圆形表盘实心拖尾对象

//  定义自己的Shader
CDashGL_CircularSolidTrackShader CDashGL_CircularSolidTrackObject::solid_shader;

//  构造函数
CDashGL_CircularSolidTrackObject::CDashGL_CircularSolidTrackObject()
{
    //  配置着色器
    this->Init(&this->solid_shader);
}

//  刷新(纹理、坐标数据等)
void CDashGL_CircularSolidTrackObject::Refresh(void)
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
        this->solid_shader.SetTrack(this->start_angle_deg, this->track_angle_deg);
        this->pshader->ShaderRender();
        pthread_mutex_unlock(&this->data_mutex);
    }
}

