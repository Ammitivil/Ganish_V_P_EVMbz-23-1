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

// Параметры камеры
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

// Углы поворотов (в градусах)
float angleStand = 0.0f;          // вращение станины вокруг Y (горизонтальное)
float angleBrush = 0.0f;          // вращение кисти+манипулятора вокруг оси Z
float angleManipSelf = 0.0f;      // вращение манипулятора вокруг своей оси (X)

// Ограничение для угла кисти: ±10 градусов
const float MAX_BRUSH_ANGLE = 10.0f;
const float MIN_BRUSH_ANGLE = -10.0f;

// Скорость вращения (градусов в секунду)
const float ROTATION_SPEED = 45.0f;

// Точка крепления станины к основанию
const glm::vec3 pivotStand = glm::vec3(0.522907f, 0.424285f, -4.541827f);
// Пересечение кисти с основанием
const glm::vec3 pivotBrushBase = glm::vec3(0.522907f, 0.0f, -4.541827f);

// Начальный поворот модели 
const glm::mat4 baseRotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

// Вспомогательная функция: матрица поворота вокруг произвольной точки
glm::mat4 getRotationAroundPoint(const glm::vec3& pivot, float angleDeg, const glm::vec3& axis) {
    glm::mat4 mat = glm::mat4(1.0f);
    mat = glm::translate(mat, pivot);
    mat = glm::rotate(mat, glm::radians(angleDeg), axis);
    mat = glm::translate(mat, -pivot);
    return mat;
}


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

    GLFWwindow* window = glfwCreateWindow(1024, 768, "Grafic Model Viewer - Manipulator Rotation Around X", NULL, NULL);
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

    // Загрузка модели
    std::cout << "\n=== Загрузка модели Grafic ===\n";
    Model ourModel("Grafic.obj");
    std::cout << "Модель загружена. Количество мешей: " << ourModel.meshes.size() << std::endl;

    if (ourModel.meshes.size() < 4) {
        std::cout << "ОШИБКА: модель должна содержать как минимум 4 меша.\n";
        std::cout << "Завершение программы.\n";
        glfwTerminate();
        return 1;
    }
    std::cout << "Предполагаемый порядок мешей: 0 - Основание, 1 - Станина, 2 - Кисть, 3 - Манипулятор.\n";

    // Вычисляем центр манипулятора
    glm::vec3 localPivotSelf = ourModel.getMeshCenter(3);
    std::cout << "Вычисленный центр манипулятора: ("
        << localPivotSelf.x << ", "
        << localPivotSelf.y << ", "
        << localPivotSelf.z << ")\n";
    std::cout << "================================\n\n";

    // Шейдер
    Shader shader;
    if (shader.load("vert_shader.glsl", "frag_shader.glsl") == 0) {
        std::cout << "Ошибка загрузки шейдеров!" << std::endl;
        return 1;
    }
    shader.use();

    glEnable(GL_DEPTH_TEST);

    // Параметры освещения
    glm::vec3 lightPos = glm::vec3(3.0f, 5.0f, 4.0f);
    glm::vec3 lightAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 lightDiffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 lightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

    glm::vec3 materialAmbient = glm::vec3(0.3f, 0.3f, 0.3f);
    glm::vec3 materialDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 materialSpecular = glm::vec3(0.8f, 0.8f, 0.8f);
    float materialShininess = 64.0f;

    std::cout << "=== Управление иерархическими аффинными преобразованиями ===\n";
    std::cout << "Станина (горизонтальный поворот вокруг Y):               Q / E\n";
    std::cout << "Кисть+манипулятор (вращение вокруг оси Z, центр = пересечение кисти с основанием): R / F\n";
    std::cout << "  Ограничение: угол не более ±10 градусов от начального\n";
    std::cout << "Вращение манипулятора вокруг своей оси (X):             T / G\n";
    std::cout << "============================================================\n\n";

    // Главный цикл
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Управление камерой
        float cameraSpeed = 2.5f * deltaTime;
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

        // Управление аффинными преобразованиями
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            angleStand += ROTATION_SPEED * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            angleStand -= ROTATION_SPEED * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            angleBrush += ROTATION_SPEED * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
            angleBrush -= ROTATION_SPEED * deltaTime;
        if (angleBrush > MAX_BRUSH_ANGLE) angleBrush = MAX_BRUSH_ANGLE;
        if (angleBrush < MIN_BRUSH_ANGLE) angleBrush = MIN_BRUSH_ANGLE;

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            angleManipSelf += ROTATION_SPEED * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
            angleManipSelf -= ROTATION_SPEED * deltaTime;

        // Матрицы проекции и вида
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setFloat("time", currentFrame);

        // Параметры освещения
        shader.setVec3("light.position", lightPos);
        shader.setVec3("light.ambient", lightAmbient);
        shader.setVec3("light.diffuse", lightDiffuse);
        shader.setVec3("light.specular", lightSpecular);
        shader.setVec3("material.ambient", materialAmbient);
        shader.setVec3("material.diffuse", materialDiffuse);
        shader.setVec3("material.specular", materialSpecular);
        shader.setFloat("material.shininess", materialShininess);
        shader.setVec3("viewPos", cameraPos);

        // Очистка экрана
        glClearColor(0.8f, 0.2f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // 1. Основание – только начальный поворот (неподвижно)
        glm::mat4 modelBase = baseRotation;
        shader.setMat4("model", modelBase);
        ourModel.meshes[0].Draw(shader);

        // 2. Станина – начальный поворот + вращение вокруг pivotStand (ось Y)
        glm::mat4 modelStand = baseRotation * getRotationAroundPoint(pivotStand, angleStand, glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model", modelStand);
        ourModel.meshes[1].Draw(shader);

        // 3. Кисть – наследует станину, затем вращение вокруг оси Z с ограниченным углом
        glm::mat4 modelBrush = modelStand * getRotationAroundPoint(pivotBrushBase, angleBrush, glm::vec3(0.0f, 0.0f, 1.0f));
        shader.setMat4("model", modelBrush);
        ourModel.meshes[2].Draw(shader);

        // 4. Манипулятор – наследует кисть, плюс собственное вращение вокруг своей оси X (как колесо)
        // Используем вычисленный центр localPivotSelf (в локальных координатах кисти)
        glm::mat4 modelManip = modelBrush * getRotationAroundPoint(localPivotSelf, angleManipSelf, glm::vec3(1.0f, 0.0f, 0.0f));
        shader.setMat4("model", modelManip);
        ourModel.meshes[3].Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}