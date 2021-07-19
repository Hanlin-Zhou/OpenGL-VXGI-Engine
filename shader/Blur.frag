#version 430 core
out vec4 FragColor;
  
in vec2 TexCoords;
//  in flat int sampleSize;

uniform sampler2D ssaoInput;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, TexCoords + offset).r;
        }
    }
    float pp = result / (4.0 * 4.0);
    FragColor = vec4(pp, pp, pp, 1.0);
}  