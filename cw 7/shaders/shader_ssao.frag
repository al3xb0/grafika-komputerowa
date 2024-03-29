#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

int kernelSize = 64;
uniform float radius;
uniform float bias;

const vec2 noiseScale = vec2(1000/4.0, 1000/4.0); 

uniform mat4 perspectiveMatrix;

void main()
{
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        vec3 sample = TBN * samples[i];
        sample = fragPos + sample * radius;
        vec4 offset = vec4(sample, 1.0);
        offset = perspectiveMatrix * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        vec3 occluredPos = texture(gPosition, offset.xy).xyz;
    
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - occluredPos.z));
        occlusion += (occluredPos.z >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    FragColor = occlusion;
}