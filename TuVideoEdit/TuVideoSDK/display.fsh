#version 300 es

precision highp float;
out vec4 fragColor;
in vec2 yTextureCoord;
in vec2 uTextureCoord;
in vec2 vTextureCoord;

uniform sampler2D yuvTexture;

uniform bool applyInversionFilter;
uniform bool applyGrayscaleFilter;

uniform bool applyEffect1;

void main()
{
    // yuv420就是四个y共用一个u和一个v,所以纹理采样的时候多少有点精度问题
    float y = texture(yuvTexture, yTextureCoord).r;
    float u = texture(yuvTexture, uTextureCoord).r - 0.5;
    float v = texture(yuvTexture, vTextureCoord).r - 0.5;

    float r = y + 1.402 * v;
    float g = y - 0.344136 * u - 0.714136 * v;
    float b = y + 1.772 * u;

    vec4 color = vec4(r, g, b, 1.0);

    if (applyEffect1) { // 二分特效
        vec2 ypos = yTextureCoord;
        if (ypos.x > 0.5) {
            ypos.x = 1.0 - ypos.x;
        }
        vec2 upos = uTextureCoord;
        if (upos.x > 0.25) {
            upos.x = 0.5 - upos.x;
        }
        vec2 vpos = vTextureCoord;
        if (vpos.x > 0.25) {
            vpos.x = 0.5 - vpos.x;
        }
        float y = texture(yuvTexture, ypos).r;
        float u = texture(yuvTexture, upos).r - 0.5;
        float v = texture(yuvTexture, vpos).r - 0.5;

        float r = y + 1.402 * v;
        float g = y - 0.344136 * u - 0.714136 * v;
        float b = y + 1.772 * u;

        color = vec4(r, g, b, 1.0);
    }

    if (applyInversionFilter) {
        color = vec4(1.0 - color.r, 1.0 - color.g, 1.0 - color.b, 1.0); //反相
    } else if (applyGrayscaleFilter) {
        float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b; //灰度
        color = vec4(average, average, average, 1.0);
    }
    
    fragColor = color;
}
