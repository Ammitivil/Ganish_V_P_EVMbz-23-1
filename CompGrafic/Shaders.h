#pragma once

#define GLEW_DLL
#define GLFW_DLL

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
private:
    std::string readShFile(const char* path);
    void validate(float& _v, float _min = 0.0f, float _max = 1.0f);
    void validate(int& _v, int _min = 0, int _max = 255);
    float intToFloat(int& _v);

public:
    GLuint shaderProgram = 0;

    Shader();
    int load(const char* vert_sh_path, const char* frag_sh_path);
    void use();

    // Унифицированные методы для передачи uniform-переменных
    void setMat4(const char* name, const glm::mat4& mat) {
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void setVec3(const char* name, const glm::vec3& vec) {
        glUniform3fv(glGetUniformLocation(shaderProgram, name), 1, glm::value_ptr(vec));
    }

    void setFloat(const char* name, float value) {
        glUniform1f(glGetUniformLocation(shaderProgram, name), value);
    }

    void glUniform(const char* cl_name, float r, float g, float b);
    void glUniform(const char* cl_name, float r, float g, float b, float a);
    void glUniform(const char* cl_name, int r, int g, int b);
    void glUniform(const char* cl_name, int r, int g, int b, float a);
};