#version 300 es

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 yTextureCoordInput;
layout(location = 2) in vec2 uTextureCoordInput;
layout(location = 3) in vec2 vTextureCoordInput;

out vec2 yTextureCoord;
out vec2 uTextureCoord;
out vec2 vTextureCoord;

void main()
{
    yTextureCoord = yTextureCoordInput;
    uTextureCoord = uTextureCoordInput;
    vTextureCoord = vTextureCoordInput;
    gl_Position = vec4(position, 1.0);
}
