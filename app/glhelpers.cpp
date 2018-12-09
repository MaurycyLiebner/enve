#include "glhelpers.h"

#include <fstream>
#include <sstream>
#include <iostream>

std::string GL_PLAIN_VERT =
        "/home/ailuropoda/Dev/AniVect/src/shaders/plain.vert";
GLuint GL_PLAIN_SQUARE_VAO;
GLuint GL_PLAIN_SQUARE_VBO;

std::string GL_TEXTURED_VERT =
        "/home/ailuropoda/Dev/AniVect/src/shaders/textured.vert";
GLuint GL_TEXTURED_SQUARE_VAO;
GLuint GL_TEXTURED_SQUARE_VBO;

GLuint GL_BLUR_PROGRAM;

void iniTexturedVShaderData(QGL33c* gl) {
    float vertices[] = {
        // positions        // texture coords
         1.f, -1.f, 0.0f,   1.0f, 0.0f,   // bottom right
         1.f,  1.f, 0.0f,   1.0f, 1.0f,   // top right
        -1.f, -1.f, 0.0f,   0.0f, 0.0f,   // bottom left
        -1.f,  1.f, 0.0f,   0.0f, 1.0f    // top left
    };

    gl->glGenVertexArrays(1, &GL_TEXTURED_SQUARE_VAO);

    gl->glGenBuffers(1, &GL_TEXTURED_SQUARE_VBO);

    gl->glBindVertexArray(GL_TEXTURED_SQUARE_VAO);

    gl->glBindBuffer(GL_ARRAY_BUFFER, GL_TEXTURED_SQUARE_VBO);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                     vertices, GL_STATIC_DRAW);

    // position attribute
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              8 * sizeof(float), (void*)0);
    gl->glEnableVertexAttribArray(0);
    // texture coord attribute
    gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                              5 * sizeof(float), (void*)(3 * sizeof(float)));
    gl->glEnableVertexAttribArray(1);
}

void iniPlainVShaderData(QGL33c* gl) {
    float vertices[] = {
        // positions
         1.f, -1.f, 0.0f,   // bottom right
         1.f,  1.f, 0.0f,   // top right
        -1.f, -1.f, 0.0f,   // bottom left
        -1.f,  1.f, 0.0f    // top left
    };

    gl->glGenVertexArrays(1, &GL_TEXTURED_SQUARE_VAO);

    gl->glGenBuffers(1, &GL_TEXTURED_SQUARE_VBO);

    gl->glBindVertexArray(GL_TEXTURED_SQUARE_VAO);

    gl->glBindBuffer(GL_ARRAY_BUFFER, GL_TEXTURED_SQUARE_VBO);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                     vertices, GL_STATIC_DRAW);

    // position attribute
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              8 * sizeof(float), (void*)0);
    gl->glEnableVertexAttribArray(0);
}

void checkCompileErrors(QGL33c* gl,
                        const GLuint& shader,
                        const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    if(type != "PROGRAM") {
        gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success) {
            gl->glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        gl->glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success) {
            gl->glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

void iniProgram(QGL33c* gl,
                GLuint& program,
                const std::string& vShaderPath,
                const std::string& fShaderPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try {
        // open files
        vShaderFile.open(vShaderPath);
        fShaderFile.open(fShaderPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLuint vertexShader = gl->glCreateShader(GL_VERTEX_SHADER);
    gl->glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
    gl->glCompileShader(vertexShader);
    checkCompileErrors(gl, vertexShader, "VERTEX");

    GLuint fragmentShader = gl->glCreateShader(GL_FRAGMENT_SHADER);
    gl->glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
    gl->glCompileShader(fragmentShader);
    checkCompileErrors(gl, fragmentShader, "FRAGMENT");

    program = gl->glCreateProgram();
    gl->glAttachShader(program, vertexShader);
    gl->glAttachShader(program, fragmentShader);
    gl->glLinkProgram(program);
    checkCompileErrors(gl, program, "PROGRAM");

    gl->glDeleteShader(vertexShader);
    gl->glDeleteShader(fragmentShader);
}
