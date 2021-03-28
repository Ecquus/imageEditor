#include "displaywidget.h"

const char* const DisplayWidget::vertexShaderSource =
R"END(
#version 120

attribute vec2 in_Position;
attribute vec2 in_TexCoords;
varying   vec2 out_TexCoords;

uniform   mat4 matrix;

void main(void)
{
    gl_Position = matrix * vec4(in_Position, 0.0, 1.0);
    out_TexCoords = in_TexCoords;
};
)END";

const char* const DisplayWidget::fragmentShaderSource =
R"END(
#version 120

varying vec2 out_TexCoords;

uniform sampler2D texture;

uniform float redLevel;
uniform float greenLevel;
uniform float blueLevel;
uniform float brightLevel;
uniform float contrastLevel;

vec4 toYCbCr(vec4 rgba)
{
    float r = rgba.x, g = rgba.y, b = rgba.z;
    
    float y  = clamp( (( 0.299f  * r) + (0.587f  * g) + (0.114f  * b))                  , 0.0f, 1.0f );
    float cb = clamp( ((-0.1687f * r) - (0.3313f * g) + (0.5f    * b) + (128.0f/255.0f)), 0.0f, 1.0f );
    float cr = clamp( (( 0.5f    * r) - (0.4187f * g) - (0.0813f * b) + (128.0f/255.0f)), 0.0f, 1.0f );

    return vec4(y, cb, cr, rgba.w);
}
    
vec4 toRGB(vec4 ycbcr)
{
    float y = ycbcr.x, cb = ycbcr.y, cr = ycbcr.z;
    
    float r = clamp( (y                                      + (1.402f  * (cr - (128.0f/255.0f)))), 0.0f, 1.0f );
    float g = clamp( (y - (0.3441f * (cb - (128.0f/255.0f))) - (0.7141f * (cr - (128.0f/255.0f)))), 0.0f, 1.0f );
    float b = clamp( (y + (1.722f  * (cb - (128.0f/255.0f))))                                     , 0.0f, 1.0f );

    return vec4(r, g, b, ycbcr.w);
}

float applyRed(float ch)
{
    if (redLevel < 0.5f)
        return 2.0f * ch * redLevel;
    else
        return 2.0f * (1.0f - ch) * redLevel + 2.0f * ch - 1.0f;
}

float applyGreen(float ch)
{
    if (greenLevel < 0.5f)
        return 2.0f * ch * greenLevel;
    else
        return 2.0f * (1.0f - ch) * greenLevel + 2.0f * ch - 1.0f;
}

float applyBlue(float ch)
{
    if (blueLevel < 0.5f)
        return 2.0f * ch * blueLevel;
    else
        return 2.0f * (1.0f - ch) * blueLevel + 2.0f * ch - 1.0f;
}

float lowContrast(float ch)
{
    float a1 = contrastLevel / (contrastLevel * pow(0.5f, contrastLevel - 1.0f));
    return a1 * pow(ch, contrastLevel);
}

float highContrast(float ch)
{
    float a2 = contrastLevel / (contrastLevel * pow(0.5f, contrastLevel - 1.0f));
    return 1.0f - a2 * pow(1.0f - ch, contrastLevel);
}

float contrast(float ch)
{
    if (ch < 0.5f)
        return lowContrast(ch);
    else if (ch > 0.5f)
        return highContrast(ch);
    else
        return ch;
}

vec4 IO(vec4 pix)
{
    // apply color operations on the pixel
    pix.xyz = vec3(applyRed(pix.x), applyGreen(pix.y), applyBlue(pix.z));

    // apply intensity operations on the pixel
    vec4 ycbcr = toYCbCr(pix);
    float y  = contrast(ycbcr.x) + brightLevel;
    float cb = contrast(ycbcr.y);
    float cr = contrast(ycbcr.z);
    
    return toRGB(vec4(y, cb, cr, ycbcr.w));
}

void main(void)
{
    // gl_FragColor = IO(texture2D(texture, out_TexCoords.st));
    gl_FragColor = IO(texture2D(texture, out_TexCoords.st));
};
)END";

const char* const DisplayWidget::grayscaleFragmentShaderSource =
R"END(
#version 120

varying vec2 out_TexCoords;

uniform sampler2D texture;

vec4 toYCbCr(vec4 rgba)
{
    float r = rgba.x, g = rgba.y, b = rgba.z;
    
    float y  = clamp( (( 0.299f  * r) + (0.587f  * g) + (0.114f  * b))                  , 0.0f, 1.0f );
    float cb = clamp( ((-0.1687f * r) - (0.3313f * g) + (0.5f    * b) + (128.0f/255.0f)), 0.0f, 1.0f );
    float cr = clamp( (( 0.5f    * r) - (0.4187f * g) - (0.0813f * b) + (128.0f/255.0f)), 0.0f, 1.0f );

    return vec4(y, cb, cr, rgba.w);
}

vec4 grayscale(vec4 rgba)
{
    float y = toYCbCr(rgba).x;
    return vec4(y, y, y, rgba.w);
}

void main(void)
{
    gl_FragColor = grayscale(texture2D(texture, out_TexCoords.st));
};
)END";