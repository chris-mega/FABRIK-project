#version 150

out vec4 fColor;
flat in vec4 color;

void main() 
{ 
    float depth = gl_FragCoord.z;
    fColor = vec4(depth, depth, depth, 1) * color;
}