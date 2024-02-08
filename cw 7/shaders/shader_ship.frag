#version 430 core

float AMBIENT = 0.1;

uniform vec3 color;
uniform vec3 lightPos;

uniform sampler2D colorTexture;
uniform sampler2D scratches;
uniform vec3 cameraPos;
uniform sampler2D rust; 
uniform float exposure;

in vec2 fragTexCoord;
in vec3 vecNormal;
in vec3 worldPos;

out vec4 outColor;
void main()
{

	float scratchMask = texture(scratches, fragTexCoord).r;

	vec4 shipColor = texture(colorTexture, fragTexCoord);
    vec4 scratchesColor = texture(scratches, fragTexCoord);
    vec4 rustColor = texture(rust, fragTexCoord);
	vec4 finalColor = mix(mix(shipColor, scratchesColor, scratchMask), rustColor, scratchMask);

	vec3 lightDir = normalize(lightPos-worldPos);
	vec3 normal = normalize(vecNormal);
	float diffuse=max(0,dot(normal,lightDir));

	vec3 viewDir = normalize(cameraPos - worldPos);
	vec3 ref = reflect(lightDir, vecNormal);
	float spec = (pow(max(dot(viewDir, ref), 0.0), 25));

	float distance = length(lightPos - worldPos);
    float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);

	outColor = vec4((finalColor*min(1,AMBIENT+diffuse+spec)*attenuation*exposure).xyz, 1.0);

	

}
