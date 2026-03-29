#version 410 core
out vec4 frag_colour;

in vec3 FragPos;
in vec3 Normal;

uniform float time;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

// Структура материала 
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// Структура источника света
struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Униформы для материала и света
uniform Material material;
uniform Light light;

// Функция преобразования HSV (оттенка) в RGB
vec3 hue2rgb(float hue) {
    float r = clamp(abs(hue * 6.0 - 3.0) - 1.0, 0.0, 1.0);
    float g = clamp(2.0 - abs(hue * 6.0 - 2.0), 0.0, 1.0);
    float b = clamp(2.0 - abs(hue * 6.0 - 4.0), 0.0, 1.0);
    return vec3(r, g, b);
}

void main() {
    // Плавный циклический перелив цвета от времени 
    float hue = fract(time * 0.1);
    vec3 rainbowColor = hue2rgb(hue);
    
    // Нормализуем нормаль
    vec3 norm = normalize(Normal);
    
    // Направление к источнику света (используем позицию из структуры Light)
    vec3 lightDir = normalize(light.position - FragPos);
    
    // Диффузное освещение по формуле из PDF:
    // Diffuse = LightColor * MaterialDiffuse * max(dot(N, L), 0)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
    // Спекулярное освещение по формуле из PDF:
    // Specular = LightColor * MaterialSpecular * pow(max(dot(R, V), 0), Shininess)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);
    
    // Окружающее освещение по формуле из PDF:
    // Ambient = LightColor * MaterialAmbient
    vec3 ambient = light.ambient * material.ambient;
    
    // Итоговый цвет = (окружающий + диффузный + зеркальный) * цвет перелива
    vec3 result = (ambient + diffuse + specular) * rainbowColor;
    frag_colour = vec4(result, 1.0);
}