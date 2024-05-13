#ifndef SHADER_HPP_CM510QXM
#define SHADER_HPP_CM510QXM

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

class Shader
{
public:
    const gl::GLuint m_id;

    Shader()                         = delete;
    Shader(const Shader&)            = delete;
    Shader(Shader&&)                 = delete;
    Shader& operator=(const Shader&) = delete;
    Shader& operator=(Shader&&)      = delete;

    Shader(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath)
        : m_id{ gl::glCreateProgram() }    // create program
    {
        // quick dirty hack to prevent data race :>
        std::unique_lock lock{ s_mutex };

        std::string   vsSource;
        std::ifstream vsFile{ vsPath };
        if (!vsFile) {
            std::cerr << "Error reading vertex shader file: " << vsPath << '\n';
        } else {
            std::stringstream buffer;
            buffer << vsFile.rdbuf();
            vsSource = buffer.str();
        }

        std::string   fsSource;
        std::ifstream fsFile{ fsPath };
        if (!fsFile) {
            std::cerr << "Error reading fragment shader file: " << fsPath << '\n';
        } else {
            std::stringstream buffer;
            buffer << fsFile.rdbuf();
            fsSource = buffer.str();
        }

        auto vsId{ prepareShader(vsSource, ShaderStage::VERTEX) };
        auto fsId{ prepareShader(fsSource, ShaderStage::FRAGMENT) };

        // link shaders to shader program
        gl::glAttachShader(m_id, vsId);
        gl::glAttachShader(m_id, fsId);
        gl::glLinkProgram(m_id);
        shaderLinkInfo(m_id);

        // delete shader objects
        gl::glDeleteShader(vsId);
        gl::glDeleteShader(fsId);
    }

    ~Shader() { gl::glDeleteProgram(m_id); }

    void use() const { gl::glUseProgram(m_id); }

private:
    enum class ShaderStage
    {
        VERTEX,
        FRAGMENT,
    };

    // global mutex for data race prevention when reading file (a hack)
    static inline std::mutex s_mutex;

    void shaderCompileInfo(gl::GLuint shader, ShaderStage stage)
    {
        std::string_view name;
        switch (stage) {
        case ShaderStage::VERTEX: name = "VERTEX"; break;
        case ShaderStage::FRAGMENT: name = "FRAGMENT"; break;
        }

        gl::GLint status{};
        gl::glGetShaderiv(shader, gl::GL_COMPILE_STATUS, &status);
        if (status != 1) {
            gl::GLint maxLength{};
            gl::GLint logLength{};

            gl::glGetShaderiv(shader, gl::GL_INFO_LOG_LENGTH, &maxLength);
            auto* log{ new gl::GLchar[(std::size_t)maxLength] };
            gl::glGetShaderInfoLog(shader, maxLength, &logLength, log);
            std::cerr << std::format("Shader compilation of type {} failed:\n{}\n", name, log);
            delete[] log;
        }
    }

    void shaderLinkInfo(gl::GLuint program)
    {
        gl::GLint status{};
        glGetProgramiv(program, gl::GL_LINK_STATUS, &status);
        if (status != 1) {
            gl::GLint maxLength{};
            gl::GLint logLength{};

            glGetProgramiv(program, gl::GL_INFO_LOG_LENGTH, &maxLength);
            auto* log{ new gl::GLchar[(std::size_t)maxLength] };
            gl::glGetProgramInfoLog(program, maxLength, &logLength, log);
            std::cerr << "Program linking failed: \n" << log << '\n';
            delete[] log;
        }
    }

    gl::GLuint prepareShader(const std::string& vsSource, ShaderStage stage)
    {
        gl::GLenum type;
        switch (stage) {
        case ShaderStage::VERTEX: type = gl::GL_VERTEX_SHADER; break;
        case ShaderStage::FRAGMENT: type = gl::GL_FRAGMENT_SHADER; break;
        }

        // compile vertex shader
        gl::GLuint  vsId{ glCreateShader(type) };
        const char* vsSourceCharPtr{ vsSource.c_str() };
        gl::glShaderSource(vsId, 1, &vsSourceCharPtr, nullptr);
        gl::glCompileShader(vsId);
        shaderCompileInfo(vsId, stage);

        return vsId;
    }
};

#endif /* end of include guard: SHADER_HPP_CM510QXM */
