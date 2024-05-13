//========================================================================
// Offscreen rendering example
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

#include <exception>
#include <stdlib.h>
#include <stdio.h>

#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>
#include <linmath.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

// clang-format off
static const struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.6f, 0.f, 0.f, 1.f }
};
// clang-format on

static const char* vertex_shader_text = R"glsl(
    #version 110

    uniform mat4 MVP;
    attribute vec3 vCol;
    attribute vec2 vPos;
    varying vec3 color;

    void main()
    {
        gl_Position = MVP * vec4(vPos, 0.0, 1.0);
        color = vCol;
    }
)glsl";

static const char* fragment_shader_text = R"glsl(
    #version 110

    varying vec3 color;

    void main()
    {
        gl_FragColor = vec4(color, 1.0);
    }
)glsl";

int main()
{
    try {
        auto api = glfw_cpp::Api::OpenGL{
            .m_major  = 2,
            .m_minor  = 0,
            .m_loader = [](auto, auto proc) { gladLoadGLLoader((GLADloadproc)proc); },
        };
        auto logger = [](auto level, auto msg) {
            if ((int)level >= (int)glfw_cpp::Instance::LogLevel::ERROR) {
                fprintf(stderr, "glfw-cpp error: %s\n", msg.c_str());
            }
        };

        using H = glfw_cpp::WindowHint;
        H hint  = { .m_flags = H::DEFAULT & ~H::VISIBLE };

        // TODO: implement
        // glfwInitHint(GLFW_COCOA_MENUBAR, GLFW_FALSE);

        auto glfw   = glfw_cpp::init(api, logger);
        auto wm     = glfw->createWindowManager();
        auto window = wm.createWindow(hint, "Simple offscreen example", 800, 600);

        window.bind();

        // NOTE: OpenGL error checks have been omitted for brevity

        GLuint vertex_buffer;
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        GLuint vertex_shader;
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
        glCompileShader(vertex_shader);

        GLuint fragment_shader;
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
        glCompileShader(fragment_shader);

        GLuint program;
        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        GLint mvp_location  = glGetUniformLocation(program, "MVP");
        GLint vpos_location = glGetAttribLocation(program, "vPos");
        GLint vcol_location = glGetAttribLocation(program, "vCol");

        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);
        glEnableVertexAttribArray(vcol_location);
        glVertexAttribPointer(
            vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)(sizeof(float) * 2)
        );

        auto [width, height] = window.properties().m_framebufferSize;
        float ratio          = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4 mvp;
        mat4x4_ortho(mvp, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glFinish();

        auto buffer = std::make_unique<unsigned char[]>(4 * width * height);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.get());

        // Write image Y-flipped because OpenGL
        stbi_write_png(
            "offscreen.png", width, height, 4, buffer.get() + (width * 4 * (height - 1)), -width * 4
        );

    } catch (std::exception& e) {
        fprintf(stderr, "Exception occurred: %s\n", e.what());
        return 1;
    }
}
