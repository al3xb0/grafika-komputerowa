#version 430 core

float AMBIENT = 0.1;

uniform vec3 color;
uniform vec3 lightPos;
uniform sampler2D colorTexture;
uniform float exposure;
uniform vec3 cameraPos;

in vec3 vecNormal;
in vec3 worldPos;
in vec2 fragTexCoord; 

out vec4 outColor;
void main()
{
	vec3 lightDir = normalize(lightPos-worldPos);
    vec3 viewDir = normalize(cameraPos - worldPos);
	vec3 normal = normalize(vecNormal);
	float diffuse=max(0,dot(normal,lightDir));

    vec3 ref = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, ref), 0.0), 25);

    float distance = length(lightPos - worldPos);
    float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);

    vec3 baseColor = texture2D(colorTexture, fragTexCoord).rgb;
	outColor = vec4(baseColor * min(1, AMBIENT + diffuse + spec) * attenuation * exposure, 1.0);
}



