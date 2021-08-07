//#version 310 es
//#extension GL_EXT_gpu_shader4 : enable
//#extension GL_ARB_texture_multisample : enable
precision highp float;
varying vec2 tex_color_pos;
uniform vec4 in_blend_color;
//layout (location = 0) out vec4 gl_FragColor;
uniform sampler2DMS texture1;

vec4 color_1;
vec4 color_2;
vec4 color_3;
vec4 color_4;
vec4 color_5;
vec4 color_6;
vec4 color_7;
vec4 color_8;
vec4 color_9;

void main() 
{
    ivec2 texSize = textureSize(texture1);
    ivec2 ttt;
    ttt.x = int(float(texSize.x) * tex_color_pos.x);
    ttt.y = int(float(texSize.y) * tex_color_pos.y);

    vec4 fTexCol = vec4(0.0);


    //color_1 = texelFetch(texture1, ttt + ivec2(0, 0),   32);
    //color_2 = texelFetch(texture1, ttt + ivec2(0, 1),   32);
    //color_3 = texelFetch(texture1, ttt + ivec2(0, -1),  32);
    //color_4 = texelFetch(texture1, ttt + ivec2(1, 0),   32);
    //color_5 = texelFetch(texture1, ttt + ivec2(1, 1),   32);
    //color_6 = texelFetch(texture1, ttt + ivec2(1, -1),  32);
    //color_7 = texelFetch(texture1, ttt + ivec2(-1, 0),  32);
    //color_8 = texelFetch(texture1, ttt + ivec2(-1, 1),  32);
    //color_9 = texelFetch(texture1, ttt + ivec2(-1, -1), 32);
    //gl_FragColor = ((color_1 + color_2 + color_3 + color_4 + color_5 + color_6 + color_7 + color_8 + color_9) / 9.0) * in_blend_color;

    //gl_FragColor = texelFetch(texture1, ttt, 8) * in_blend_color;

    for(int i = 0; i < 32; ++i)
    {
       fTexCol += texelFetch(texture1, ttt, i);  
    }
 
    gl_FragColor = (fTexCol / 32.0) * in_blend_color;
    
    //vec4 out1;
    //if(texSize.x >= 10) out1 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    //else             out1 = vec4(0.0f, 1.0f, 0.0f, 1.0f);
    //gl_FragColor = out1;
}

