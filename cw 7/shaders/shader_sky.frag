#version 430 core

uniform samplerCube textureSampler;

in vec3 texCoord;

out vec4 out_color;

void main()
{
	out_color = texture(textureSampler,texCoord);
}