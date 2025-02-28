#version 330

// Input
in vec2 texture_coord;

// Uniform properties
uniform sampler2D textureImage;
uniform ivec2 screenSize;
uniform int flipVertical;
uniform int outputMode = 2; // 0: original, 1: grayscale, 2: blur

// Output
layout(location = 0) out vec4 out_color;

// Local variables
vec2 textureCoord = vec2(texture_coord.x, (flipVertical != 0) ? 1 - texture_coord.y : texture_coord.y); // Flip texture


vec4 grayscale()
{
    vec4 color = texture(textureImage, textureCoord);
    float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b;
    return vec4(gray, gray, gray, 0);
}

float toGrayscale(vec4 color)
{
    return 0.21 * color.r + 0.71 * color.g + 0.07 * color.b;
}

// hasurare
float hashPattern(vec2 coord, float a, float b, float c) {
    float value = sin(a * coord.x + b * coord.y);
    return step(c, value);  
}
vec4 hashEffect1() // linii groase negre pe fundal alb
{
    vec4 color = texture(textureImage, textureCoord);
    float intensity = toGrayscale(color);

    vec2 uv = textureCoord * screenSize;

    float pattern1 = hashPattern(uv, 10.0, 10.0, 0.5);  

    float result = 1;
    if (intensity < 0.3)
    {
		result = pattern1;
	}

    return vec4(vec3(result), 1.0);  
}

vec4 hashEffect2() // linii negre subtiri pe fundal alb
{
    vec4 color = texture(textureImage, textureCoord);
    float intensity = toGrayscale(color);

    vec2 uv = textureCoord * screenSize;

    float pattern2 = hashPattern(uv, 0.0, 5, 0.1);

    float result = 1;
    if (intensity > 0.2 && intensity < 0.7)
    {
        result = pattern2;
    }

    return vec4(vec3(result), 1.0);  
}

vec4 hashEffect3() 
{
    vec4 color = texture(textureImage, textureCoord);
    float intensity = toGrayscale(color);

    vec2 uv = textureCoord * screenSize;

    float pattern3 = 1 - hashPattern(uv, 1, 2, 0.99); 

    float result = 1;
    if (intensity > 0.5)
    {
        result = pattern3;
    }

    return vec4(vec3(result), 1.0);  
}

vec4 hashEffect123()
{
    vec4 color = texture(textureImage, textureCoord);
    float intensity = toGrayscale(color);

    vec2 uv = textureCoord * screenSize;

    float pattern1 = hashPattern(uv, 10.0, 10.0, 0.5);
    float pattern2 = hashPattern(uv, 0.0, 5, 0.1);
    float pattern3 = 1 - hashPattern(uv, 1, 2, 0.7);  

    float result = 0;
    if (intensity < 0.3)
    {
		result += pattern1;
	}
    if (intensity > 0.2 && intensity < 0.7)
    {
		result += pattern2;
	}
    if (intensity > 0.5)
    {
		result += pattern3;
	}


    return vec4(vec3(result), 1.0);  
}




vec4 blur(int blurRadius)
{
    vec2 texelSize = 1.0f / screenSize;
    vec4 sum = vec4(0);
    for (int i = -blurRadius; i <= blurRadius; i++)
    {
        for (int j = -blurRadius; j <= blurRadius; j++)
        {
            sum += texture(textureImage, textureCoord + vec2(i, j) * texelSize);
        }
    }
    float samples = pow((2 * blurRadius + 1), 2);
    return sum / samples;
}

vec4 horizontalBlur()
{
    vec2 texelSize = 1.0f / screenSize;
    vec4 sum = vec4(0);
    // (25x1)
    for (int i = -12; i <= 12; i++) 
    {
        sum += texture(textureImage, textureCoord + vec2(i, 0) * texelSize);
    }
    vec4 horizontalBlur = sum / 25.0;
    return horizontalBlur;
}

vec4 verticalBlur() {
    vec2 texelSize = 1.0f / screenSize;
	vec4 sum = vec4(0);
	// (1x25)
    for (int j = -12; j <= 12; j++)
    {
		sum += texture(textureImage, textureCoord + vec2(0, j) * texelSize);
	}
	vec4 verticalBlur = sum / 25.0;
	return verticalBlur;
}

const mat3 dx = mat3(
    vec3(-1.0, 0.0, 1.0),
    vec3(-2.0, 0.0, 2.0),
    vec3(-1.0, 0.0, 1.0)
);
const mat3 dy = mat3(
    vec3(-1.0, -2.0, -1.0),
    vec3(0.0, 0.0, 0.0),
    vec3(1.0, 2.0, 1.0)
);

float sobel()
{
    vec2 texelSize = 1.0f / screenSize;
    float sumX = 0, sumY = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            sumX += toGrayscale(texture(textureImage, textureCoord + vec2(i, j) * texelSize))
                * dx[i + 1][j + 1];
            sumY += toGrayscale(texture(textureImage, textureCoord + vec2(i, j) * texelSize))
                * dy[i + 1][j + 1];

        }
    }
    float x = pow(pow(sumX, 2) + pow(sumY, 2), 0.5);
    if (x <= 0.5)
        return 1;
    return 0;
}

void main()
{
    switch (outputMode)
    {
    case 1:
    {
        out_color = texture(textureImage, textureCoord);
        break;
    }

    case 2:
    {
        out_color = sobel() < 0.5 ? vec4(0) : vec4(1);
        break;
    }

    case 3:
    {
        out_color = horizontalBlur();
        break;
    }

    case 4:
    {
        out_color = verticalBlur();
        break;
    }

    case 5:
    {
        out_color = blur(25);
        out_color = hashEffect1();
        //out_color = sobel() < 0.5 ? vec4(0) : out_color;
        break;
	}

    case 6:
    {
        out_color = blur(25);
        out_color = hashEffect2();
        //out_color = sobel() < 0.5 ? vec4(0) : out_color;
        break;
    }
    case 7:
    {
        out_color = blur(25);
        out_color = hashEffect3();
        //out_color = sobel() < 0.5 ? vec4(0) : out_color;
        break;
	}

    case 8:
    {
        out_color = blur(7);
        out_color = hashEffect123();
        out_color = sobel() < 0.5 ? vec4(0) : out_color;
        break;
    }
    case 9: 
    {
        out_color = blur(8);
    }

    default:
        out_color = texture(textureImage, textureCoord);
        break;
    }
}
