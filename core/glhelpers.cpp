#include "glhelpers.h"
#include "exceptions.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

QString GL_PLAIN_VERT = ":/shaders/plain.vert";
GLuint GL_PLAIN_SQUARE_VBO;

QString GL_TEXTURED_VERT = ":/shaders/textured.vert";
GLuint GL_TEXTURED_SQUARE_VBO;

void checkGlErrors(const std::string& msg) {
    const GLenum glError = glGetError();
    if(glError == GL_NO_ERROR) return;
    RuntimeThrow("OpenGL error " + std::to_string(glError) + " " + msg);
}

void iniTexturedVShaderVBO(QGL33c * const gl) {
    float vertices[] = {
    //  positions  | texture coords
         1, -1, 0,   1, 0,   // bottom right
         1,  1, 0,   1, 1,   // top right
        -1, -1, 0,   0, 0,   // bottom left
        -1,  1, 0,   0, 1    // top left
    };

    gl->glGenBuffers(1, &GL_TEXTURED_SQUARE_VBO);
    gl->glBindBuffer(GL_ARRAY_BUFFER, GL_TEXTURED_SQUARE_VBO);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                     vertices, GL_STATIC_DRAW);
}

void iniTexturedVShaderVAO(QGL33c * const gl, GLuint &VAO) {
    gl->glGenVertexArrays(1, &VAO);
    gl->glBindVertexArray(VAO);

    gl->glBindBuffer(GL_ARRAY_BUFFER, GL_TEXTURED_SQUARE_VBO);

    // position attribute
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              5 * sizeof(float), BUFFER_OFFSET(0));
    gl->glEnableVertexAttribArray(0);
    // texture coord attribute
    gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                              5 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
    gl->glEnableVertexAttribArray(1);
}

void iniPlainVShaderVBO(QGL33c * const gl) {
    float vertices[] = {
        // positions
         1, -1, 0,   // bottom right
         1,  1, 0,   // top right
        -1, -1, 0,   // bottom left
        -1,  1, 0    // top left
    };

    gl->glGenBuffers(1, &GL_PLAIN_SQUARE_VBO);
    gl->glBindBuffer(GL_ARRAY_BUFFER, GL_PLAIN_SQUARE_VBO);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                     vertices, GL_STATIC_DRAW);
}

void iniPlainVShaderVAO(QGL33c * const gl, GLuint &VAO) {
    gl->glGenVertexArrays(1, &VAO);
    gl->glBindVertexArray(VAO);

    gl->glBindBuffer(GL_ARRAY_BUFFER, GL_PLAIN_SQUARE_VBO);

    // position attribute
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              3 * sizeof(float), BUFFER_OFFSET(0));
    gl->glEnableVertexAttribArray(0);
}

//! @brief Checks for errors after program linking and shader compilation.
void checkCompileErrors(QGL33c * const gl,
                        const GLuint& shader,
                        const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    if(type != "PROGRAM") {
        gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success) {
            gl->glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            RuntimeThrow("ERROR::SHADER_COMPILATION_ERROR of type: " +
                         type + "\n" + infoLog);
        }
    } else {
        gl->glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success) {
            gl->glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            RuntimeThrow(
                "ERROR::PROGRAM_LINKING_ERROR of type: " +
                        type + "\n" + infoLog);
        }
    }
}
#include <QFile>
void iniProgram(QGL33c * const gl,
                GLuint& program,
                const QString& vShaderPath,
                const QString& fShaderPath) {
    Q_INIT_RESOURCE(coreresources);
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // Use QFile to enable reading from resource file
        QFile vShaderFile(vShaderPath);
        if(!vShaderFile.open(QIODevice::ReadOnly))
            RuntimeThrow("Could not open " + vShaderPath);
        const QByteArray vData = vShaderFile.readAll();
        vertexCode = vData.toStdString();
        vShaderFile.close();

        QFile fShaderFile(fShaderPath);
        if(!fShaderFile.open(QIODevice::ReadOnly))
            RuntimeThrow("Could not open " + fShaderPath);
        const QByteArray fData = fShaderFile.readAll();
        fragmentCode = fData.toStdString();
        fShaderFile.close();
    } catch(...) {
        RuntimeThrow("Could not load shader data from file.");
    }
    const char* const vShaderCode = vertexCode.c_str();
    const char* const fShaderCode = fragmentCode.c_str();

    const GLuint vertexShader = gl->glCreateShader(GL_VERTEX_SHADER);
    gl->glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
    gl->glCompileShader(vertexShader);
    try {
        checkCompileErrors(gl, vertexShader, "VERTEX");
    } catch(...) {
        RuntimeThrow("Error compiling vertex shader.");
    }

    const GLuint fragmentShader = gl->glCreateShader(GL_FRAGMENT_SHADER);
    gl->glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
    gl->glCompileShader(fragmentShader);
    try {
        checkCompileErrors(gl, fragmentShader, "FRAGMENT");
    } catch(...) {
        RuntimeThrow("Error compiling fragment shader.");
    }

    program = gl->glCreateProgram();
    gl->glAttachShader(program, vertexShader);
    gl->glAttachShader(program, fragmentShader);
    gl->glLinkProgram(program);
    try {
        checkCompileErrors(gl, program, "PROGRAM");
    } catch(...) {
        RuntimeThrow("Error linking program.");
    }

    gl->glDeleteShader(vertexShader);
    gl->glDeleteShader(fragmentShader);
}

Texture Texture::sCreateTextureFromImage(QGL33c * const gl,
                                         const std::string &imagePath) {
    Texture tex;
    tex.gen(gl);
    tex.loadImage(gl, imagePath);
    return tex;
}

void Texture::bind(QGL33c * const gl) const {
    gl->glBindTexture(GL_TEXTURE_2D, fID);
}

void Texture::clear(QGL33c * const gl) {
    gl->glDeleteTextures(1, &fID);
    fID = 0;
    fWidth = 0;
    fHeight = 0;
}

void Texture::gen(QGL33c * const gl) {
    gl->glGenTextures(1, &fID);
    gl->glBindTexture(GL_TEXTURE_2D, fID);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::gen(QGL33c * const gl,
                  const int width, const int height,
                  const void * const data) {
    gen(gl);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    fWidth = width;
    fHeight = height;
}

bool Texture::loadImage(QGL33c * const gl, const std::string &imagePath) {
    int nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char * const data = stbi_load(imagePath.c_str(),
                                           &fWidth, &fHeight,
                                           &nrChannels, 0);
    if(data) {
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fWidth, fHeight,
                         0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        //gl->glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        RuntimeThrow("Failed to load texture" + imagePath);
    }
    stbi_image_free(data);
    return true;
}

#include "skia/skiahelpers.h"
sk_sp<SkImage> Texture::toImage(QGL33c * const gl) {
    bind(gl);
    SkBitmap btmp;
    const auto info = SkiaHelpers::getPremulBGRAInfo(fWidth, fHeight);
    btmp.allocPixels(info);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, btmp.getPixels());
//    glReadPixels(0, 0, fWidth, fHeight,
//                 GL_RGBA, GL_UNSIGNED_BYTE, btmp.getPixels());
    return SkiaHelpers::transferDataToSkImage(btmp);
}

void TextureFrameBuffer::clear(QGL33c * const gl) {
    gl->glDeleteFramebuffers(1, &fFBOId);
    fTexture.clear(gl);
    fFBOId = 0;
    fWidth = 0;
    fHeight = 0;
}

void TextureFrameBuffer::bind(QGL33c * const gl) {
    gl->glBindFramebuffer(GL_FRAMEBUFFER, fFBOId);
}

void TextureFrameBuffer::bindTexture(QGL33c * const gl) {
    fTexture.bind(gl);
}

void TextureFrameBuffer::gen(QGL33c * const gl,
                             const int width, const int height) {
    fWidth = width;
    fHeight = height;
    gl->glGenFramebuffers(1, &fFBOId);
    gl->glBindFramebuffer(GL_FRAMEBUFFER, fFBOId);
    // create a color attachment texture
    fTexture.gen(gl, width, height, nullptr);
    gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, fTexture.fID, 0);
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if(gl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        RuntimeThrow("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
}
