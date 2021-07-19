#version 430 core
out float FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform sampler2D gViewPos;

uniform vec3 samples[64];
uniform mat4 proj;
uniform mat4 view;

const vec2 noiseScale = vec2(1000.0/4.0, 1000.0/4.0);


vec3 toViewSpace(vec3 fg){
	vec4 fragP = vec4(fg, 1.0);
	vec4 viewFragP = view * fragP;
	vec3 result = viewFragP.xyz/ viewFragP.w;
	return result;
}


void main(){
	vec3 fragPos = texture(gPosition, TexCoords).xyz;
	vec3 normal    = normalize(texture(gNormal, TexCoords).rgb);
	vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	int kernelSize = 64;
	float radius = 0.8;
	float bias = 0.002;
	float verify1 = texture(gNormal, TexCoords).w;
	if (verify1 == 0.0) {
		for(int i = 0; i < kernelSize; i++)
		{
			vec3 samplePos = TBN * samples[i]; 
			samplePos = fragPos + samplePos * radius; // world space here
			vec4 offset = vec4(samplePos, 1.0);    // offset in [-1.0, 1.0]
			vec4 check2 = view * offset;
			offset      = proj * view * offset;    // to camera space for depth check
			double ssao_guessed_depth = check2.z/ check2.w;
			offset.xyz /= offset.w;   
			offset.xyz  = offset.xyz * 0.5 + 0.5; // NDC to texture space [0.0, 1.0]
			if (offset.x < 1 && offset.y < 1 && offset.x > 0 && offset.y > 0){
				vec3 aaa = texture(gViewPos, offset.xy).xyz;
				double gbuffer_depth = aaa.z;
				float verify3 = texture(gNormal, offset.xy).w;
				if (verify3 == 0.0) {
					if (0.5 >= abs(ssao_guessed_depth - gbuffer_depth)){
						occlusion       += ((gbuffer_depth > ssao_guessed_depth + bias) ? 1.0 : 0.0);
					}
				}
			}
		}  
	}
	occlusion = 1.0 - (occlusion / kernelSize);
	FragColor = occlusion; 
}