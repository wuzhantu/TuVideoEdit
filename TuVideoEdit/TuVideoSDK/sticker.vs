#version 300 es

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textureCoordInput;

out vec2 textureCoord;

void main()
{
    textureCoord = textureCoordInput;
    gl_Position = vec4(position, 1.0);
}
