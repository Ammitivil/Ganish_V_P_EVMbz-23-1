#define GLEW_DLL
#define GLFW_DLL

#include <iostream>
#include <cmath>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shaders.h"
#include "Model.h"

//Параметры камеры
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 1024.0f / 2.0f;
float lastY = 1024.0f / 2.0f;
bool firstMouse = true;
float sensitivity = 0.1f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Функция обратного вызова мыши
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

int main() {
    // Инициализация GLFW
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3.\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1024, 768, "Grafic Model Viewer - Rainbow Effect", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Инициализация GLEW
    glewExperimental = GL_TRUE;
    GLenum ret = glewInit();
    if (ret != GLEW_OK) {
        fprintf(stderr, "ERROR: %s\n", glewGetErrorString(ret));
        return 1;
    }

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));

    // Вывод информации о способах ориентации камеры
    std::cout << "\n=== Способы задания ориентации камеры ===\n";
    std::cout << "1. Look-At (glm::lookAt) - используется в программе.\n";
    std::cout << "   Преимущества: простота, интуитивность.\n";
    std::cout << "   Недостатки: ограниченность, зависимость от вектора 'вверх'.\n\n";
    std::cout << "2. Кватернионы (не используются).\n";
    std::cout << "   Преимущества: отсутствие Gimbal Lock, плавная интерполяция.\n";
    std::cout << "   Недостатки: менее интуитивны, требуют нормализации.\n\n";
    std::cout << "3. Углы Эйлера (используются для управления мышью).\n";
    std::cout << "   Преимущества: интуитивность, компактность.\n";
    std::cout << "   Недостатки: Gimbal Lock, порядок вращений имеет значение.\n";
    std::cout << "========================================\n\n";

    // Создание и загрузка модели
    std::cout << "\n=== Загрузка модели Grafic ===\n";
    Model ourModel("Grafic.obj");
    std::cout << "Модель загружена. Количество мешей: " << ourModel.meshes.size() << std::endl;
    std::cout << "================================\n\n";

    //Шейдер
    Shader* shader = new Shader();
    if (shader->load("vert_shader.glsl", "frag_shader.glsl") == 0) {
        std::cout << "Ошибка загрузки шейдеров!" << std::endl;
        return 1;
    }
    shader->use();

    glEnable(GL_DEPTH_TEST);

    // Параметры освещения
    // Позиция источника света
    glm::vec3 lightPos = glm::vec3(3.0f, 5.0f, 4.0f);

    // Настройка компонентов источника света 
    // Окружающий свет - низкая интенсивность, чтобы не доминировать
    glm::vec3 lightAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
    // Диффузный свет - яркий белый цвет
    glm::vec3 lightDiffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    // Зеркальный свет - полная интенсивность
    glm::vec3 lightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

    // Настройка материала объекта 
    // Материал для радужного эффекта: диффузный компонент будет модулироваться цветом перелива
    glm::vec3 materialAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
    glm::vec3 materialDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);   // Полная интенсивность для яркого перелива
    glm::vec3 materialSpecular = glm::vec3(0.8f, 0.8f, 0.8f);  // Яркие блики
    float materialShininess = 64.0f;  // Более резкие блики для лучшего эффекта

    // Проверка uniform переменных в шейдере
    std::cout << "\n=== Проверка uniform переменных ===\n";
    GLint loc;
    loc = glGetUniformLocation(shader->shaderProgram, "light.position");
    std::cout << "light.position location: " << loc << std::endl;
    loc = glGetUniformLocation(shader->shaderProgram, "light.ambient");
    std::cout << "light.ambient location: " << loc << std::endl;
    loc = glGetUniformLocation(shader->shaderProgram, "light.diffuse");
    std::cout << "light.diffuse location: " << loc << std::endl;
    loc = glGetUniformLocation(shader->shaderProgram, "light.specular");
    std::cout << "light.specular location: " << loc << std::endl;
    loc = glGetUniformLocation(shader->shaderProgram, "material.ambient");
    std::cout << "material.ambient location: " << loc << std::endl;
    loc = glGetUniformLocation(shader->shaderProgram, "material.diffuse");
    std::cout << "material.diffuse location: " << loc << std::endl;
    loc = glGetUniformLocation(shader->shaderProgram, "material.specular");
    std::cout << "material.specular location: " << loc << std::endl;
    loc = glGetUniformLocation(shader->shaderProgram, "material.shininess");
    std::cout << "material.shininess location: " << loc << std::endl;
    loc = glGetUniformLocation(shader->shaderProgram, "time");
    std::cout << "time location: " << loc << std::endl;
    loc = glGetUniformLocation(shader->shaderProgram, "viewPos");
    std::cout << "viewPos location: " << loc << std::endl;
    std::cout << "==================================\n\n";

    std::cout << "=== Радужный эффект активирован ===\n";
    std::cout << "Цвет модели переливается в зависимости от времени и положения!\n";
    std::cout << "Освещение настроено по модели Фонга (окружающий + диффузный + зеркальный)\n\n";

    // Главный цикл 
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float cameraSpeed = 2.5f * deltaTime;

        //Управление клавиатурой
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        //Построение матриц
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);

        // Поворот модели для лучшего обзора
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", model);

        // Передача времени для анимации радужного эффекта
        shader->setFloat("time", currentFrame);

        // Передача параметров источника света (используем структуру Light)
        shader->setVec3("light.position", lightPos);
        shader->setVec3("light.ambient", lightAmbient);
        shader->setVec3("light.diffuse", lightDiffuse);
        shader->setVec3("light.specular", lightSpecular);

        // Передача параметров материала (используем структуру Material)
        shader->setVec3("material.ambient", materialAmbient);
        shader->setVec3("material.diffuse", materialDiffuse);
        shader->setVec3("material.specular", materialSpecular);
        shader->setFloat("material.shininess", materialShininess);

        // Передача позиции камеры для расчета зеркальных бликов
        shader->setVec3("viewPos", cameraPos);

        // Отрисовка (цвет фона не изменен)
        glClearColor(0.8f, 0.2f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourModel.Draw(*shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    delete shader;
    return 0;
}