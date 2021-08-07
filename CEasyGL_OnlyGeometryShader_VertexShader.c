//#version 310 es
precision mediump float;
attribute vec3 in_pos;
uniform vec3 vec3_move;
uniform mat3 mat3_scale;
uniform mat3 mat3_rotate_x;
uniform mat3 mat3_rotate_y;
uniform mat3 mat3_rotate_z;
uniform vec3 vec3_move_center;
void main() 
{
   gl_Position = vec4(((in_pos + vec3_move_center) *
                       mat3_rotate_x *
                       mat3_rotate_y *
                       mat3_rotate_z *
                       mat3_scale)+vec3_move, 1.0);
}

