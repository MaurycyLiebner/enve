#include "glhelpers.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

std::string GL_PLAIN_VERT =
        "/home/ailuropoda/Dev/AniVect/src/shaders/plain.vert";
GLuint GL_PLAIN_SQUARE_VBO;

std::string GL_TEXTURED_VERT =
        "/home/ailuropoda/Dev/AniVect/src/shaders/textured.vert";
GLuint GL_TEXTURED_SQUARE_VBO;

BlurProgram GL_BLUR_PROGRAM;

void assertNoGlErrors() {
    GLenum glError = glGetError();
    assert(glError == GL_NO_ERROR);
}

void iniTexturedVShaderVBO(QGL33c* gl) {
    float vertices[] = {
        // positions        // texture coords
         1.f, -1.f, 0.0f,   1.0f, 0.0f,   // bottom right
         1.f,  1.f, 0.0f,   1.0f, 1.0f,   // top right
        -1.f, -1.f, 0.0f,   0.0f, 0.0f,   // bottom left
        -1.f,  1.f, 0.0f,   0.0f, 1.0f    // top left
    };

    gl->glGenBuffers(1, &GL_TEXTURED_SQUARE_VBO);
    gl->glBindBuffer(GL_ARRAY_BUFFER, GL_TEXTURED_SQUARE_VBO);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                     vertices, GL_STATIC_DRAW);
}

void iniTexturedVShaderVAO(QGL33c* gl, GLuint &VAO) {
    gl->glGenVertexArrays(1, &VAO);
    gl->glBindVertexArray(VAO);

    gl->glBindBuffer(GL_ARRAY_BUFFER, GL_TEXTURED_SQUARE_VBO);

    // position attribute
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              5 * sizeof(float), (void*)0);
    gl->glEnableVertexAttribArray(0);
    // texture coord attribute
    gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                              5 * sizeof(float), (void*)(3 * sizeof(float)));
    gl->glEnableVertexAttribArray(1);
}

void iniPlainVShaderVBO(QGL33c* gl) {
    float vertices[] = {
        // positions
         1.f, -1.f, 0.0f,   // bottom right
         1.f,  1.f, 0.0f,   // top right
        -1.f, -1.f, 0.0f,   // bottom left
        -1.f,  1.f, 0.0f    // top left
    };

    gl->glGenBuffers(1, &GL_PLAIN_SQUARE_VBO);
    gl->glBindBuffer(GL_ARRAY_BUFFER, GL_PLAIN_SQUARE_VBO);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                     vertices, GL_STATIC_DRAW);
}

void iniPlainVShaderVAO(QGL33c* gl, GLuint &VAO) {
    gl->glGenVertexArrays(1, &VAO);
    gl->glBindVertexArray(VAO);

    gl->glBindBuffer(GL_ARRAY_BUFFER, GL_PLAIN_SQUARE_VBO);

    // position attribute
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              3 * sizeof(float), (void*)0);
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

Texture Texture::createTextureFromImage(QGL33c *gl,
                                        const std::string &imagePath) {
    Texture tex;
    tex.gen(gl);
    tex.loadImage(gl, imagePath);

    return tex;
}

void Texture::bind(QGL33c *gl) {
    gl->glBindTexture(GL_TEXTURE_2D, fID);
}

void Texture::deleteTexture(QGL33c *gl) {
    gl->glDeleteTextures(1, &fID);
}

void Texture::gen(QGL33c *gl) {
    gl->glGenTextures(1, &fID);
    gl->glBindTexture(GL_TEXTURE_2D, fID);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::gen(QGL33c *gl,
                  const int &width, const int &height,
                  const void * const data) {
    gen(gl);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void Texture::loadImage(QGL33c *gl, const std::string &imagePath) {
    int nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(imagePath.c_str(), &fWidth, &fHeight,
                                    &nrChannels, 0);
    if(data) {
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fWidth, fHeight,
                         0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        gl->glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << imagePath << std::endl;
        exit(EXIT_FAILURE);
    }
    stbi_image_free(data);
    assertNoGlErrors();
}

sk_sp<SkImage> TextureFrameBuffer::toImage() {
    SkBitmap btmp;
    SkImageInfo info = SkImageInfo::Make(fWidth, fHeight,
                                         kBGRA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    btmp.allocPixels(info);
    glReadPixels(0, 0, fWidth, fHeight,
                 GL_RGBA, GL_UNSIGNED_BYTE, btmp.getPixels());
    btmp.setImmutable();
    return SkImage::MakeFromBitmap(btmp);
}

void TextureFrameBuffer::deleteTexture(QGL33c *gl) {
    fTexture.deleteTexture(gl);
}

void TextureFrameBuffer::deleteFrameBuffer(QGL33c *gl) {
    gl->glDeleteFramebuffers(1, &fFrameBufferId);
}

void TextureFrameBuffer::bindFrameBuffer(QGL33c *gl) {
    gl->glBindFramebuffer(GL_FRAMEBUFFER, fFrameBufferId);
}

void TextureFrameBuffer::bindTexture(QGL33c *gl) {
    fTexture.bind(gl);
}

void TextureFrameBuffer::gen(QGL33c *gl,
                             const int &widthT, const int &heightT) {
    fWidth = widthT;
    fHeight = heightT;
    gl->glGenFramebuffers(1, &fFrameBufferId);
    gl->glBindFramebuffer(GL_FRAMEBUFFER, fFrameBufferId);
    // create a color attachment texture
    fTexture.gen(gl, widthT, heightT, nullptr);
    gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, fTexture.fID, 0);
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if(gl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
}
