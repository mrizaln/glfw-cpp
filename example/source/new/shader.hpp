#ifndef SHADER_HPP_CM510QXM
#define SHADER_HPP_CM510QXM

#include <fmt/core.h>
#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

class Shader
{
public:
    const gl::GLuint id;

    Shader()                         = delete;
    Shader(const Shader&)            = delete;
    Shader(Shader&&)                 = delete;
    Shader& operator=(const Shader&) = delete;
    Shader& operator=(Shader&&)      = delete;

    Shader(std::string_view vs_source, std::string_view fs_source)
        : id{ gl::glCreateProgram() }
    {
        auto vs_id = prepare_shader(vs_source, ShaderStage::Vertex);
        auto fs_id = prepare_shader(fs_source, ShaderStage::Fragment);

        // link shaders to shader program
        gl::glAttachShader(id, vs_id);
        gl::glAttachShader(id, fs_id);
        gl::glLinkProgram(id);

        shader_link_info(id);

        // delete shader objects
        gl::glDeleteShader(vs_id);
        gl::glDeleteShader(fs_id);
    }

    ~Shader() { gl::glDeleteProgram(id); }

    void use() const { gl::glUseProgram(id); }

private:
    enum class ShaderStage
    {
        Vertex,
        Fragment,
    };

    // // global mutex for data race prevention when reading file (a hack)
    // static inline std::mutex s_mutex;

    void shader_compile_info(gl::GLuint shader, ShaderStage stage)
    {
        auto name = std::string_view{};
        switch (stage) {
        case ShaderStage::Vertex: name = "VERTEX"; break;
        case ShaderStage::Fragment: name = "FRAGMENT"; break;
        }

        auto status = gl::GLint{};
        gl::glGetShaderiv(shader, gl::GL_COMPILE_STATUS, &status);
        if (status != 1) {
            auto max_len = gl::GLint{};
            auto log_len = gl::GLint{};

            gl::glGetShaderiv(shader, gl::GL_INFO_LOG_LENGTH, &max_len);
            auto log = std::basic_string<gl::GLchar>(static_cast<std::size_t>(max_len), '\0');
            gl::glGetShaderInfoLog(shader, max_len, &log_len, log.data());
            fmt::println(stderr, "[Shader] Shader compilation of type {} failed:\n{}\n", name, log);
        }
    }

    void shader_link_info(gl::GLuint program)
    {
        auto status = gl::GLint{};
        gl::glGetProgramiv(program, gl::GL_LINK_STATUS, &status);
        if (status != 1) {
            auto max_len = gl::GLint{};
            auto log_len = gl::GLint{};

            gl::glGetProgramiv(program, gl::GL_INFO_LOG_LENGTH, &max_len);
            auto log = std::basic_string<gl::GLchar>(static_cast<std::size_t>(max_len), '\0');
            gl::glGetProgramInfoLog(program, max_len, &log_len, log.data());
            fmt::println(stderr, "[Shader] Program linking failed: {}", log);
        }
    }

    gl::GLuint prepare_shader(std::string_view shader_source, ShaderStage stage)
    {
        gl::GLenum type;
        switch (stage) {
        case ShaderStage::Vertex: type = gl::GL_VERTEX_SHADER; break;
        case ShaderStage::Fragment: type = gl::GL_FRAGMENT_SHADER; break;
        }

        // compile vertex shader
        auto  shader_id         = gl::glCreateShader(type);
        auto* shader_source_ptr = shader_source.data();
        auto  len               = static_cast<int>(shader_source.size());
        gl::glShaderSource(shader_id, 1, &shader_source_ptr, &len);
        gl::glCompileShader(shader_id);
        shader_compile_info(shader_id, stage);

        return shader_id;
    }
};

#endif /* end of include guard: SHADER_HPP_CM510QXM */
