#include "glhelpers.h"
#include "exceptions.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

QString GL_PLAIN_VERT = ":/shaders/plain.vert";
GLuint GL_PLAIN_SQUARE_VBO;

QString GL_TEXTURED_VERT = ":/shaders/textured.vert";
GLuint GL_TEXTURED_SQUARE_VBO;

void iniTexturedVShaderVBO(QGL33 * const gl) {
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

void iniTexturedVShaderVAO(QGL33 * const gl, GLuint &VAO) {
    gl->glGenVertexArrays(1, &VAO);
    if(VAO == 0) RuntimeThrow("glGenVertexArrays failed");
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

void iniPlainVShaderVBO(QGL33 * const gl) {
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

void iniPlainVShaderVAO(QGL33 * const gl, GLuint &VAO) {
    gl->glGenVertexArrays(1, &VAO);
    if(VAO == 0) RuntimeThrow("glGenVertexArrays failed");
    gl->glBindVertexArray(VAO);

    gl->glBindBuffer(GL_ARRAY_BUFFER, GL_PLAIN_SQUARE_VBO);

    // position attribute
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              3 * sizeof(float), BUFFER_OFFSET(0));
    gl->glEnableVertexAttribArray(0);
}

//! @brief Checks for errors after program linking and shader compilation.
void checkCompileErrors(QGL33 * const gl,
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
            RuntimeThrow("ERROR::PROGRAM_LINKING_ERROR of type: " +
                         type + "\n" + infoLog);
        }
    }
}
#include <QFile>
void iniProgram(QGL33 * const gl, GLuint& program,
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
    if(program == 0) RuntimeThrow("Error while creating program");
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

void Texture::bind(QGL33 * const gl) const {
    gl->glBindTexture(GL_TEXTURE_2D, fId);
}

void Texture::clear(QGL33 * const gl) {
    if(fId) gl->glDeleteTextures(1, &fId);
    fId = 0;
    fWidth = 0;
    fHeight = 0;
}

void Texture::gen(QGL33 * const gl) {
    gl->glGenTextures(1, &fId);
    if(fId == 0) RuntimeThrow("Failed to generate texture");
    gl->glBindTexture(GL_TEXTURE_2D, fId);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::gen(QGL33 * const gl,
                  const int width, const int height,
                  const void * const data) {
    gen(gl);
    set(gl, width, height, data);
}

void Texture::set(QGL33 * const gl,
                  const int width, const int height,
                  const void * const data) {
    if(width <= 0 || height <= 0) RuntimeThrow("Invalid texture size");
    if(fWidth == width && fHeight == height && data) {
        gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                            GL_RGBA, GL_UNSIGNED_BYTE, data);
    } else {
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                         0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        fWidth = width;
        fHeight = height;
    }
}

#include "skia/skiahelpers.h"
SkBitmap Texture::bitmapSnapshot(QGL33 * const gl) const {
    bind(gl);
    SkBitmap bitmap;
    const auto info = SkiaHelpers::getPremulRGBAInfo(fWidth, fHeight);
    bitmap.allocPixels(info);
    gl->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, bitmap.getPixels());
//    glReadPixels(0, 0, fWidth, fHeight,
//                 GL_RGBA, GL_UNSIGNED_BYTE, btmp.getPixels());
    return bitmap;
}

sk_sp<SkImage> Texture::imageSnapshot(QGL33 * const gl) const {
    auto bitmap = bitmapSnapshot(gl);
    return SkiaHelpers::transferDataToSkImage(bitmap);
}

void TextureFrameBuffer::clear(QGL33 * const gl) {
    if(fFBOId) gl->glDeleteFramebuffers(1, &fFBOId);
    fFBOId = 0;
    fWidth = 0;
    fHeight = 0;
    unbind();

    fTexture.clear(gl);
}

void TextureFrameBuffer::bind(QGL33 * const gl) {
    if(fBound) return;
    gl->glBindFramebuffer(GL_FRAMEBUFFER, fFBOId);
    fBound = true;
}

void TextureFrameBuffer::bindTexture(QGL33 * const gl) {
    fTexture.bind(gl);
}

void TextureFrameBuffer::gen(QGL33 * const gl,
                             const int width, const int height) {
    fWidth = width;
    fHeight = height;
    gl->glGenFramebuffers(1, &fFBOId);
    bind(gl);
    // create a color attachment texture
    fTexture.gen(gl, width, height, nullptr);
    gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, fTexture.fId, 0);
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if(gl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        RuntimeThrow("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
}
