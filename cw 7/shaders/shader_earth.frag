#version 430 core

float AMBIENT = 0.1;

uniform vec3 color;
uniform vec3 lightPos;
uniform sampler2D colorTexture;
uniform sampler2D normalSampler;  
uniform float exposure;
uniform vec3 cameraPos;

in vec2 fragTexCoord; 
in vec3 worldPos;
in vec3 viewDirTS;
in vec3 lightDirTS;

out vec4 outColor;

void main()
{
    
    vec3 baseColor = texture2D(colorTexture, fragTexCoord).rgb;

    vec3 N = texture2D(normalSampler, fragTexCoord).xyz;
   
    N = 2.0 * N - 1.0;
    N = normalize(N);

    vec3 viewDir = normalize(viewDirTS);
    vec3 lightDir = normalize(lightDirTS);

    float diffuse = max(0, dot(N, lightDir));

    vec3 ref = reflect(-lightDir, N);
    float spec = pow(max(dot(viewDir, ref), 0.0), 25);

    float distance = length(lightPos - worldPos);
    float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);

    outColor = vec4(baseColor * min(1, AMBIENT + diffuse + spec) * attenuation * exposure, 1.0);
}

