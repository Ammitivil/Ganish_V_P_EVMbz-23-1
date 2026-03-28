#version 410 core
out vec4 frag_colour;

in vec3 FragPos;
in vec3 Normal;

uniform float time;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

// Функция преобразования HSV (оттенка) в RGB
vec3 hue2rgb(float hue) {
    float r = clamp(abs(hue * 6.0 - 3.0) - 1.0, 0.0, 1.0);
    float g = clamp(2.0 - abs(hue * 6.0 - 2.0), 0.0, 1.0);
    float b = clamp(2.0 - abs(hue * 6.0 - 4.0), 0.0, 1.0);
    return vec3(r, g, b);
}

void main() {
    // Радужный цвет: hue зависит от времени и координаты X фрагмента
    // Изменяем параметры для более красивого перелива
    float hue = fract(time * 0.3 + FragPos.x * 0.2 + FragPos.y * 0.1 + FragPos.z * 0.15);
    
    // Получаем радужный цвет
    vec3 rainbowColor = hue2rgb(hue);
    
    // Нормализуем нормаль
    vec3 norm = normalize(Normal);
    
    // Направление света
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Диффузное освещение
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Спекулярное освещение
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = 0.5 * spec * lightColor;
    
    // Ambient освещение
    vec3 ambient = 0.3 * lightColor;
    
    // Итоговый цвет = освещение * радужный цвет
    vec3 result = (ambient + diffuse + specular) * rainbowColor;
    frag_colour = vec4(result, 1.0);
}