#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Материальные свойства из Assimp
uniform vec3 material_ambient;
uniform vec3 material_diffuse;
uniform vec3 material_specular;
uniform float material_shininess;

// Переопределение материала
uniform bool use_material_override;
uniform vec3 material_override_diffuse;

// Улучшения для отображения материалов
uniform float materialBrightness;
uniform bool enhanceContrast;

// Lighting
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform vec3 ambientStrength;

// Текстуры (опциональные)
uniform sampler2D texture_diffuse1;
uniform bool has_diffuse_texture;

// Функция для улучшения контраста
vec3 enhanceColor(vec3 color) {
    if (enhanceContrast) {
        // Увеличиваем контраст и насыщенность
        color = pow(color, vec3(0.9)); // Гамма коррекция

        // Увеличиваем насыщенность
        float luminance = dot(color, vec3(0.299, 0.587, 0.114));
        color = mix(vec3(luminance), color, 1.3); // Увеличиваем насыщенность

        // Небольшое увеличение контраста
        color = (color - 0.5) * 1.1 + 0.5;
    }

    return clamp(color, 0.0, 1.0);
}

void main()
{
    // Определяем финальный цвет материала
    vec3 finalMaterialColor;

    if (use_material_override) {
        // Используем переопределенный цвет
        finalMaterialColor = material_override_diffuse;
    } else {
        // Используем цвет материала из модели (.mtl файла)
        finalMaterialColor = material_diffuse;

        // Проверяем, что материал не слишком темный или белый
        if (length(finalMaterialColor) < 0.1) {
            // Если материал слишком темный, используем серый по умолчанию
            finalMaterialColor = vec3(0.7, 0.7, 0.7);
        }
    }

    // Применяем множитель яркости
    finalMaterialColor *= materialBrightness;

    // Если есть диффузная текстура, смешиваем с материалом
    if (has_diffuse_texture) {
        vec3 textureColor = texture(texture_diffuse1, TexCoords).rgb;
        // Модулируем текстуру с цветом материала
        finalMaterialColor = finalMaterialColor * textureColor;
    }

    // Улучшаем цвет если включено
    finalMaterialColor = enhanceColor(finalMaterialColor);

    // Normalize normal vector
    vec3 norm = normalize(Normal);

    // Enhanced ambient lighting - используем настраиваемую силу ambient
    vec3 ambient = ambientStrength * finalMaterialColor;

    // Diffuse lighting
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * finalMaterialColor * lightColor;

    // Specular lighting - используем specular материала или default
    vec3 specularColor = use_material_override ? vec3(0.5, 0.5, 0.5) : material_specular;
    float shininess = use_material_override ? 32.0 : max(material_shininess, 1.0);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * specularColor * lightColor;

    // Добавим дополнительное заполняющее освещение для лучшей видимости
    vec3 fillLight = 0.15 * finalMaterialColor * vec3(0.8, 0.9, 1.0); // Холодный заполняющий свет

    // Добавим rim lighting для лучшего контура объектов
    float rimDot = 1.0 - dot(viewDir, norm);
    vec3 rimLight = vec3(0.1) * pow(rimDot, 3.0) * lightColor;

    // Combine results
    vec3 result = ambient + diffuse + specular + fillLight + rimLight;

    // Тонирование для более естественного вида
    result = result / (result + vec3(1.0)); // Tone mapping

    // Финальная гамма коррекция
    result = pow(result, vec3(1.0/2.2));

    // Output final color
    FragColor = vec4(result, 1.0);
}