#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 Tangent;
in vec3 Bitangent;

// Material properties
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

// Lighting
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

// Material properties
uniform float shininess = 32.0;
uniform float ambientStrength = 0.1;
uniform float specularStrength = 0.5;

void main()
{
    // Sample textures
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    vec3 specularMap = texture(texture_specular1, TexCoords).rgb;

    // Normal mapping
    vec3 normal = normalize(Normal);
    vec3 tangent = normalize(Tangent);
    vec3 bitangent = normalize(Bitangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // Sample normal map and transform to world space
    vec3 normalMap = texture(texture_normal1, TexCoords).rgb;
    if (length(normalMap) > 0.1) {
        normalMap = normalMap * 2.0 - 1.0;
        normal = normalize(TBN * normalMap);
    }

    // Ambient lighting
    vec3 ambient = ambientStrength * lightColor * color;

    // Diffuse lighting
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * color;

    // Specular lighting (Blinn-Phong)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor * specularMap;

    // Combine results
    vec3 result = ambient + diffuse + specular;

    // Apply gamma correction
    result = pow(result, vec3(1.0/2.2));

    FragColor = vec4(result, 1.0);
}