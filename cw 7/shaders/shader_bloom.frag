#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;

void main()
{
    vec3 hdrColor = vec3(texture(scene, TexCoords).rgb);

    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    if(bloom)
        hdrColor += bloomColor;
    FragColor = vec4(hdrColor, 1.0);
}