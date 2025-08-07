#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Материальные свойства (как в BabylonJS StandardMaterial)
uniform vec3 material_ambient;
uniform vec3 material_diffuse;
uniform vec3 material_specular;
uniform float material_shininess;

// Переопределение материала
uniform bool use_material_override;
uniform vec3 material_override_diffuse;

// Освещение (как в BabylonJS HemisphericLight)
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 ambientStrength;

// Прозрачность (как в BabylonJS)
uniform float alpha;
uniform bool useAlpha;

// Текстуры
uniform sampler2D texture_diffuse1;
uniform bool has_diffuse_texture;

void main()
{
    // Определяем цвет материала (как в BabylonJS StandardMaterial)
    vec3 materialColor;
    
    if (use_material_override) {
        materialColor = material_override_diffuse;
    } else {
        materialColor = material_diffuse;
        
        // Если материал слишком темный, используем серый (как в BabylonJS)
        if (length(materialColor) < 0.1) {
            materialColor = vec3(0.4, 0.4, 0.4);
        }
    }

    // Если есть текстура, смешиваем с материалом
    if (has_diffuse_texture) {
        vec3 textureColor = texture(texture_diffuse1, TexCoords).rgb;
        materialColor = materialColor * textureColor;
    }

    // Нормализуем нормаль
    vec3 norm = normalize(Normal);

    // Ambient освещение (как в BabylonJS)
    vec3 ambient = ambientStrength * materialColor;
    
    // Diffuse освещение
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * materialColor * lightColor;
    
    // Specular освещение
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material_shininess);
    vec3 specular = spec * material_specular * lightColor;
    
    // Hemispheric lighting (как в BabylonJS HemisphericLight)
    vec3 groundColor = vec3(0.2, 0.2, 0.2); // Цвет земли
    vec3 skyColor = vec3(0.7, 0.8, 1.0);    // Цвет неба
    
    float hemisphereWeight = (norm.y + 1.0) * 0.5;
    vec3 hemisphericLight = mix(groundColor, skyColor, hemisphereWeight);
    
    // Комбинируем все источники света (как в BabylonJS)
    vec3 result = ambient + diffuse + specular + hemisphericLight * 0.3;

    // Применяем прозрачность (как в BabylonJS)
    float finalAlpha = useAlpha ? alpha : 1.0;
    
    FragColor = vec4(result, finalAlpha);
}