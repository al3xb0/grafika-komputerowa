
#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

uniform mat4 transformation;
uniform mat4 modelMatrix;
uniform vec3 lightPos;
uniform vec3 cameraPos;

out vec2 fragTexCoord;
out vec3 viewDirTS;
out vec3 lightDirTS;
out vec3 worldPos;

void main()
{
    
    worldPos = (modelMatrix* vec4(vertexPosition,1)).xyz;
    vec3 vertexPosition = (modelMatrix * vec4(vertexPosition, 1)).xyz;
    vec3 vertexNormal = normalize((modelMatrix * vec4(vertexNormal, 0)).xyz);
    vec3 vertexTangent = normalize((modelMatrix * vec4(vertexTangent, 0)).xyz);
    vec3 vertexBitangent = normalize((modelMatrix * vec4(vertexBitangent, 0)).xyz);

    
    vec3 vecNormal = vertexNormal;
    vec3 vecTangent = vertexTangent;
    vec3 vecBitangent = vertexBitangent;

   
    mat3 TBN = transpose(mat3(vecTangent, vecBitangent, vecNormal));

    fragTexCoord = vertexTexCoord;

   
    viewDirTS = TBN * normalize(cameraPos - vertexPosition);
    lightDirTS = TBN * normalize(lightPos - vertexPosition);

    
    gl_Position = transformation * vec4(vertexPosition, 1.0);
}
