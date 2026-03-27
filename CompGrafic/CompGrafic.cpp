#define GLEW_DLL
#define GLFW_DLL

#include <iostream>
#include <cmath>
#include <string>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shaders.h"
#include "Model.h"

// Глобальные переменные для камеры
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 100.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// Параметры для углов Эйлера
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;

// Скорость движения
float cameraSpeed = 0.005f;
float mouseSensitivity = 0.01f;

// Параметры вращения камеры вокруг модели
float orbitRadius = 20.0f;     // Радиус орбиты (увеличен в 20 раз: было 5, стало 100)
float orbitAngle = 0.0f;        // Горизонтальный угол (для A/D)
float orbitPitch = 0.0f;        // Вертикальный угол (для W/S)
float orbitSpeed = 0.002f;       // Скорость вращения
float maxPitch = 89.0f;         // Максимальный угол наклона
float minPitch = -89.0f;        // Минимальный угол наклона

// Параметры освещения
glm::vec3 lightPos = glm::vec3(1.0f, 2.0f, 2.0f);
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 objectColor = glm::vec3(0.8f, 0.5f, 0.3f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
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

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Направление взгляда (для мыши)
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    // Позиция камеры на орбите в соответствии с направлением взгляда
    cameraPos = glm::normalize(cameraFront) * orbitRadius;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float currentSpeed = orbitSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        currentSpeed *= 3.0f;

    // Вращение вокруг модели по горизонтали
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        orbitAngle += currentSpeed;
        yaw += currentSpeed * 57.2958f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        orbitAngle -= currentSpeed;
        yaw -= currentSpeed * 57.2958f;
    }

    // Вращение вокруг модели по вертикали
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        orbitPitch += currentSpeed;
        pitch += currentSpeed * 57.2958f;
        if (pitch > maxPitch) {
            pitch = maxPitch;
            orbitPitch = maxPitch / 57.2958f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        orbitPitch -= currentSpeed;
        pitch -= currentSpeed * 57.2958f;
        if (pitch < minPitch) {
            pitch = minPitch;
            orbitPitch = minPitch / 57.2958f;
        }
    }

    // Направление взгляда на основе углов
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    // Позиция камеры на орбите
    cameraPos = cameraFront * orbitRadius;
}

int main(int argc, char* argv[]) {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3.\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Model Import - ASSIMP (Orbit Camera)", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    GLenum ret = glewInit();
    if (ret != GLEW_OK) {
        fprintf(stderr, "ERROR: %s\n", glewGetErrorString(ret));
        return 1;
    }

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Загрузка модели
    string modelPath = "Grafic.obj";
    if (argc > 1) {
        modelPath = argv[1];
    }

    cout << "Loading model: " << modelPath << endl;
    Model* model = new Model(modelPath);
    cout << "Model loaded. Meshes count: " << model->meshes.size() << endl;

    Shader* shader = new Shader();
    if (shader->load("vert_shader.glsl", "frag_shader.glsl") == 0) {
        return 1;
    }

    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);

    // Модельная матрица (модель находится в центре)
    glm::mat4 modelMat = glm::mat4(1.0f);

    // Переменная для анимации цвета
    float colorTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader->use();

        // Обновление матриц проекции и вида
        glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
        float aspect = static_cast<float>(scrWidth) / static_cast<float>(scrHeight);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
        // Камера смотрит на центр (0, 0, 0), где находится модель
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);

        // Анимация цвета
        colorTime += 0.01f;
        objectColor = glm::vec3(
            (sin(colorTime) + 1.0f) / 2.0f,
            (sin(colorTime + 2.0f) + 1.0f) / 2.0f,
            (sin(colorTime + 4.0f) + 1.0f) / 2.0f
        );

        // Передача uniform-переменных в шейдер
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", modelMat);
        shader->setVec3("objectColor", objectColor);
        shader->setVec3("lightColor", lightColor);
        shader->setVec3("lightPos", lightPos);
        shader->setVec3("viewPos", cameraPos);

        // Отрисовка модели
        model->Draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete model;
    delete shader;
    glfwTerminate();
    return 0;
}