#version 310 es
precision mediump float;
in vec2 tex_color_pos;
uniform vec4 in_blend_color;
uniform float start_angle_deg;       //  开始角度
uniform float track_angle_deg;       //  拖尾角度,含方向
layout (location = 0) out vec4 outColor;
uniform sampler2D texture1;
void main() {
   //  整理坐标
   vec2 pos_xy = tex_color_pos - vec2(0.5, 0.5);
   //  得到当前点的所在弧度
   float angle_rad;
   if(pos_xy.y < 0.0) angle_rad = (2.0 * 3.1415926535898) - atan(0.0 - pos_xy.y, pos_xy.x);
   else               angle_rad = atan(pos_xy.y, pos_xy.x);
   //  转换得到角度
   float angle_deg = 180.0 * angle_rad / 3.1415926535898;
   //  计算出结束角度
   //  将开始和结束角度全部转换为0~360度以内
   float end_deg = start_angle_deg + track_angle_deg;
   if(end_deg >= 360.0) end_deg = end_deg - 360.0;
   if(end_deg < 0.0) end_deg = 360.0 + end_deg;
   float start_deg = start_angle_deg;
   if(start_deg >= 360.0) start_deg = start_deg - 360.0;
   if(start_deg < 0.0) start_deg = start_deg + 360.0;

   //  当开始角度大于结束角度
   if(start_deg > end_deg)
   {
       //  当轨迹为正
       if(track_angle_deg >= 0.0)
       {
           //  当为大弧度一侧
           if(abs(track_angle_deg) > 180.0)
           {
               if((end_deg <= angle_deg) && (angle_deg <=  start_deg))
               {
                   outColor = vec4(0.0, 0.0, 0.0, 0.0);
               }
               else
               {
                   outColor = texture(texture1, tex_color_pos) * in_blend_color;
               }
           }
           //  当为小弧度一侧
           else
           {
               if((end_deg >= angle_deg) || (angle_deg >=  start_deg))
               {
                   outColor = texture(texture1, tex_color_pos) * in_blend_color;
               }
               else
               {
                   outColor = vec4(0.0, 0.0, 0.0, 0.0);
               }
           }
       }
       //  当轨迹为负
       else
       {
           //  当为大弧度一侧
           if(abs(track_angle_deg) > 180.0)
           {
               if((end_deg <= angle_deg) && (angle_deg <=  start_deg))
               {
                   outColor = texture(texture1, tex_color_pos) * in_blend_color;
               }
               else
               {
                   outColor = vec4(0.0, 0.0, 0.0, 0.0);
               }
           }
           //  当为小弧度一侧
           else
           {
               if((end_deg <= angle_deg) && (angle_deg <=  start_deg))
               {
                   outColor = texture(texture1, tex_color_pos) * in_blend_color;
               }
               else
               {
                   outColor = vec4(0.0, 0.0, 0.0, 0.0);
               }
           }
       }
   }
   //  当开始角度小于结束角度
   else
   {
       //  当轨迹为正
       if(track_angle_deg >= 0.0)
       {
           //  当为大弧度一侧
           if(abs(track_angle_deg) > 180.0)
           {
               if((start_deg <= angle_deg) && (angle_deg <=  end_deg))
               {
                   outColor = texture(texture1, tex_color_pos) * in_blend_color;
               }
               else
               {
                   outColor = vec4(0.0, 0.0, 0.0, 0.0);
               }
           }
           //  当为小弧度一侧
           else
           {
               if((start_deg <= angle_deg) && (angle_deg <=  end_deg))
               {
                   outColor = texture(texture1, tex_color_pos) * in_blend_color;
               }
               else
               {
                   outColor = vec4(0.0, 0.0, 0.0, 0.0);
               }
           }
       }
       //  当轨迹为负
       else
       {
           //  当为大弧度一侧
           if(abs(track_angle_deg) > 180.0)
           {
               if((start_deg <= angle_deg) && (angle_deg <=  end_deg))
               {
                   outColor = vec4(0.0, 0.0, 0.0, 0.0);
               }
               else
               {
                   outColor = texture(texture1, tex_color_pos) * in_blend_color;
               }
           }
           //  当为小弧度一侧
           else
           {
               if((start_deg >= angle_deg) || (angle_deg >=  end_deg))
               {
                   outColor = texture(texture1, tex_color_pos) * in_blend_color;
               }
               else
               {
                   outColor = vec4(0.0, 0.0, 0.0, 0.0);
               }
           }
       }
   }
};

