#version 430 core
out float FragColor;
  
in vec2 TexCoords;

uniform int sampleSize;

uniform sampler2D Input;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(Input, 0));
    float result = 0.0;
    for (int x = -sampleSize; x < sampleSize; ++x) 
    {
        for (int y = -sampleSize; y < sampleSize; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(Input, TexCoords + offset).r;
        }
    }
    FragColor = result / (4.0 * sampleSize * sampleSize);
}  