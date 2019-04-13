#version 150

in  vec4 vPosition;
uniform mat4 ModelView, Projection;
uniform vec4 Color;
in vec4 vColor;
flat out vec4 color;

void main() 
{
  gl_Position = Projection * ModelView * vPosition;
  color = Color;
} 

