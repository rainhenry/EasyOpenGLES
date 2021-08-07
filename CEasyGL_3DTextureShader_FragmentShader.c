//#version 310 es
precision highp float;
varying vec2 tex_color_pos;
uniform vec4 in_blend_color;
//layout (location = 0) out vec4 outColor;
uniform sampler2D texture1;
void main() 
{
    gl_FragColor = texture2D(texture1, tex_color_pos) * in_blend_color;
}

