#version 300 es

precision highp float;
out vec4 fragColor;
in vec2 textureCoord;

uniform sampler2D myTexture;

void main()
{
    fragColor = texture(myTexture, textureCoord);
}
