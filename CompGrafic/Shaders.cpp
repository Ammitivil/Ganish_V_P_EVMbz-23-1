#define GLEW_DLL
#define GLFW_DLL

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "Shaders.h"

Shader::Shader() {

};

std::string Shader::readShFile(const char* path) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        fprintf(stderr, "Ошибка открытия файла шейдера: %s\n", path);
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
};

int Shader::load(const char* vert_sh_path, const char* frag_sh_path) {
    std::string vert_shader_str = readShFile(vert_sh_path);
    std::string frag_shader_str = readShFile(frag_sh_path);

    const char* vert_shader = vert_shader_str.c_str();
    const char* frag_shader = frag_shader_str.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vert_shader, NULL);
    glCompileShader(vs);

    GLint success;
    GLchar infoLog[1024];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 1024, NULL, infoLog);
        fprintf(stderr, "Ошибка компиляции вершинного шейдера %s\n", infoLog);
        return 0;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &frag_shader, NULL);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fs, 1024, NULL, infoLog);
        fprintf(stderr, "Ошибка компиляции фрагментного шейдера: %s\n", infoLog);
        glDeleteShader(vs);
        return 0;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    // Проверка линковки
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 1024, NULL, infoLog);
        fprintf(stderr, "Ошибка линковки шейдерной программы: %s\n", infoLog);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return 1;
};

void Shader::use() {
    glUseProgram(shaderProgram);
};

void Shader::validate(float& _v, float _min, float _max) {
    if (_v < _min) { _v = _min; return; }
    if (_v > _max) { _v = _max; return; }
};

void Shader::validate(int& _v, int _min, int _max) {
    if (_v < _min) { _v = _min; return; }
    if (_v > _max) { _v = _max; return; }
};

float Shader::intToFloat(int& _v) {
    float _nv = _v / 255.0f;
    validate(_nv, 0.0f, 1.0f);
    return _nv;
};

void Shader::glUniform(const char* cl_name, float r, float g, float b) {
    validate(r, 0.0f, 1.0f);
    validate(g, 0.0f, 1.0f);
    validate(b, 0.0f, 1.0f);
    glUniform4f(glGetUniformLocation(shaderProgram, cl_name), r, g, b, 1.0f);
};

void Shader::glUniform(const char* cl_name, float r, float g, float b, float a) {
    validate(r, 0.0f, 1.0f);
    validate(g, 0.0f, 1.0f);
    validate(b, 0.0f, 1.0f);
    validate(a, 0.0f, 1.0f);
    glUniform4f(glGetUniformLocation(shaderProgram, cl_name), r, g, b, a);
};

void Shader::glUniform(const char* cl_name, int r, int g, int b) {
    validate(r, 0, 255);
    validate(g, 0, 255);
    validate(b, 0, 255);
    glUniform4f(glGetUniformLocation(shaderProgram, cl_name), intToFloat(r), intToFloat(g), intToFloat(b), 1.0f);
};

void Shader::glUniform(const char* cl_name, int r, int g, int b, float a) {
    validate(r, 0, 255);
    validate(g, 0, 255);
    validate(b, 0, 255);
    validate(a, 0.0f, 1.0f);
    glUniform4f(glGetUniformLocation(shaderProgram, cl_name), intToFloat(r), intToFloat(g), intToFloat(b), a);
};