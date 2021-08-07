//#version 310 es
precision mediump float;
uniform vec4 in_blend_color;
//layout (location = 0) out vec4 outColor;
void main() 
{
   gl_FragColor = in_blend_color;
}

