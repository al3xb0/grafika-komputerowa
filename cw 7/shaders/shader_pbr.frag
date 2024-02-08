#version 430 core
#define PI 3.1415926535897932384626433832795
float AMBIENT = 0.9;

uniform vec3 cameraPos;

uniform vec3 color;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ao;
uniform vec3 spotlightPos;
uniform vec3 spotlightColor;
uniform vec3 spotlightConeDir;
uniform vec3 spotlightPhi;
uniform sampler2D texture;  
uniform float exposure;
uniform sampler2D normalSampler; 

in vec2 fragTexCoord;  

uniform float exposition;

in vec3 vecNormal;
in vec3 worldPos;

in vec3 fragPos;
in vec3 fragTangent;
in vec3 fragBitangent;

out vec4 outColor;

uniform float metallic=0.65;  
uniform float roughness = 0.05;  
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

vec3 phongLight(vec3 lightDir, vec3 lightColor, vec3 normal, vec3 viewDir) {
    float diffuse = max(0, dot(normal, lightDir));
    vec3 R = reflect(-lightDir, normal);
    float specular = pow(max(dot(viewDir, R), 0.0), 32);
    vec3 resultColor = color * diffuse * lightColor + lightColor * specular;
    return resultColor;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{ 
    
    vec3 viewDirTS = normalize(cameraPos - worldPos);

    vec3 N = normalize(vecNormal);
    vec3 V = normalize(cameraPos - worldPos);
    vec3 normal = normalize(vecNormal);
    vec3 viewDir = normalize(viewDirTS);

    vec3 lightDir = normalize(lightPos - fragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, color, metallic);
    vec3 albedo = texture(texture, fragTexCoord).rgb;
    vec3 Lo = vec3(0.0);
    vec3 L = normalize(lightPos - worldPos);
    vec3 H = normalize(V + L);
    float distance = length(lightPos - worldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor * attenuation;
    float NDF = DistributionGGX(N, H, roughness);   
    float G   =1.5 * GeometrySmith(N, V, L, roughness);      
    vec3 F    =1.5* fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    float NdotL = max(dot(N, L), 0.0);        

    Lo += (kD * albedo / PI + specular) * radiance * NdotL;  

    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    vec3 Nor = texture(normalSampler, fragTexCoord).xyz;
    Nor = normalize(Nor * 2.0 - 1.0);  

    float diffuse = max(0, dot(N, lightDir));

    vec3 ref = reflect(-lightDir, Nor);
    float spec = pow(max(dot(viewDir, ref), 0.0), 25);

    outColor = vec4(color * min(1, AMBIENT + diffuse + spec) * exposure, 1.0);
}
