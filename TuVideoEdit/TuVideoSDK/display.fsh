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
uniform bool applyEffect2;

void main()
{
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
    } else if (applyEffect2) {
        vec2 ypos = yTextureCoord;
        if (ypos.x <= 0.5) {
            ypos.x *= 2.0;
        } else {
            ypos.x = (ypos.x - 0.5) * 2.0;
        }

        if (ypos.y <= 0.75) {
            ypos.y = 0.5 + (ypos.y - 0.5) * 2.0;
        } else {
            ypos.y = 0.5 + (ypos.y - 0.75) * 2.0;
        }

        vec2 upos = uTextureCoord;
        if (upos.x <= 0.25) {
            upos.x *= 2.0;
        } else {
            upos.x = (upos.x - 0.25) * 2.0;
        }

        if (upos.y <= 0.125) {
            upos.y *= 2.0;
        } else {
            upos.y = (upos.y - 0.125) * 2.0;
        }

        vec2 vpos = vTextureCoord;
        if (vpos.x <= 0.25) {
            vpos.x *= 2.0;
        } else {
            vpos.x = (vpos.x - 0.25) * 2.0;
        }

        if (vpos.y <= 0.375) {
            vpos.y *= 0.25 + (vpos.y - 0.25) * 2.0;
        } else {
            vpos.y = 0.25 + (vpos.y - 0.375) * 2.0;
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
