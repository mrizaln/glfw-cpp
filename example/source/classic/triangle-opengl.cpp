//========================================================================
// OpenGL triangle example
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//
// Conversion to glfw-cpp (and C++):
//    Muhammad Rizal Nurromdhoni <mrizaln2000@gmail.com>

#include <cstddef>
#include <cstdio>
#include <cstdlib>

#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>
#include <linmath.h>

typedef struct Vertex
{
    vec2 pos;
    vec3 col;
} Vertex;

static const Vertex vertices[3] = {
    // clang-format off
    { { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } },
    { {  0.6f, -0.4f }, { 0.f, 1.f, 0.f } },
    { {   0.f,  0.6f }, { 0.f, 0.f, 1.f } }
    // clang-format on
};

static const char* vertex_shader_text = R"glsl(
    #version 330

    uniform mat4 MVP;
    in vec3 vCol;
    in vec2 vPos;
    out vec3 color;

    void main()
    {
        gl_Position = MVP * vec4(vPos, 0.0, 1.0);
        color = vCol;
    }
)glsl";

static const char* fragment_shader_text = R"glsl(
    #version 330

    in vec3 color;
    out vec4 fragment;

    void main()
    {
        fragment = vec4(color, 1.0);
    }
)glsl";

int main()
{
    auto glfw = glfw_cpp::init(
        glfw_cpp::api::OpenGL{
            .major          = 3,
            .minor          = 3,
            .profile        = glfw_cpp::api::gl::Profile::Core,
            .forward_compat = true,
            .loader         = [](auto, auto proc) { gladLoadGLLoader((GLADloadproc)proc); },
        },
        [](auto level, auto msg) {
            if ((int)level >= (int)glfw_cpp::LogLevel::Error) {
                fprintf(stderr, "%s\n", msg.c_str());
            }
        }
    );

    auto wm     = glfw->create_window_manager();
    auto window = wm->create_window({}, "OpenGL Triangle", 640, 480);

    // NOTE: OpenGL error checks have been omitted for brevity

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    const GLint mvp_location  = glGetUniformLocation(program, "MVP");
    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vcol_location = glGetAttribLocation(program, "vCol");

    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, col));

    window.run([&](const auto& events) {
        namespace ev = glfw_cpp::event;
        using KC     = glfw_cpp::KeyCode;
        using KS     = glfw_cpp::KeyState;

        for (const glfw_cpp::Event& event : events) {
            if (auto* e = event.get_if<ev::KeyPressed>()) {
                if (e->key == KC::Escape && e->state == KS::Press) {
                    window.request_close();
                }
            }
        }

        const auto [width, height] = window.properties().framebuffer_size;
        const float ratio          = (float)width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4 m, p, mvp;
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float)glfw_cpp::get_time());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)&mvp);
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        wm->poll_events();
    });
}
