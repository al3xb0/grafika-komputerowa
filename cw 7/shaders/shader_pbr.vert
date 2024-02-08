#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

out vec3 vecNormal;
out vec3 worldPos;
out vec3 fragPos;  
out vec3 fragTangent;  
out vec3 fragBitangent;  
out vec2 fragTexCoord;  // Added for texture coordinates

uniform mat4 transformation;
uniform mat4 modelMatrix;

void main()
{
    worldPos = (modelMatrix * vec4(vertexPosition, 1)).xyz;
    fragPos = worldPos; 
    vecNormal = normalize((modelMatrix * vec4(vertexNormal, 0)).xyz);
    gl_Position = transformation * vec4(vertexPosition, 1.0);
    vec3 w_tangent = normalize(mat3(modelMatrix) * vertexTangent);
    vec3 w_bitangent = normalize(mat3(modelMatrix) * vertexBitangent);
    mat3 TBN = transpose(mat3(w_tangent, w_bitangent, vecNormal));

    fragTangent = TBN * vertexTangent;  
    fragBitangent = TBN * vertexBitangent;  

    fragTexCoord = vertexTexCoord;  
}
