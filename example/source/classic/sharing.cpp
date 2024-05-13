//========================================================================
// Context sharing example
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

#include <array>
#include <cstdio>
#include <cstdlib>

#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>
#include <getopt.h>
#include <linmath.h>

static const char* vertex_shader_text = R"glsl(
    #version 110

    uniform mat4 MVP;
    attribute vec2 vPos;
    varying vec2 texcoord;

    void main()
    {
        gl_Position = MVP * vec4(vPos, 0.0, 1.0);
        texcoord = vPos;
    }
)glsl";

static const char* fragment_shader_text = R"glsl(
    #version 110

    uniform sampler2D texture;
    uniform vec3 color;
    varying vec2 texcoord;

    void main()
    {
        gl_FragColor = vec4(color * texture2D(texture, texcoord).rgb, 1.0);
    }
)glsl";

static const vec2 vertices[4] = {
    // clang-format off
    { 0.f, 0.f },
    { 1.f, 0.f },
    { 1.f, 1.f },
    { 0.f, 1.f }
    // clang-format on
};

int main()
{
    GLuint texture, program, vertex_buffer;
    GLint  mvp_location, vpos_location, color_location, texture_location;

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

    auto glfw = glfw_cpp::init(api, logger);
    auto wm   = glfw->createWindowManager();

    std::array<glfw_cpp::Window, 2> windows;

    windows[0] = wm.createWindow({}, "First", 400, 400);

    // Create the OpenGL objects inside the first context, created above
    // All objects will be shared with the second context, created below
    {
        int    x, y;
        char   pixels[16 * 16];
        GLuint vertex_shader, fragment_shader;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        srand((unsigned int)glfw_cpp::getTimerValue());

        for (y = 0; y < 16; y++) {
            for (x = 0; x < 16; x++) {
                pixels[y * 16 + x] = rand() % 256;
            }
        }

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_LUMINANCE, 16, 16, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
        glCompileShader(vertex_shader);

        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
        glCompileShader(fragment_shader);

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        mvp_location     = glGetUniformLocation(program, "MVP");
        color_location   = glGetUniformLocation(program, "color");
        texture_location = glGetUniformLocation(program, "texture");
        vpos_location    = glGetAttribLocation(program, "vPos");

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    glUseProgram(program);
    glUniform1i(texture_location, 0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);

    windows[1] = wm.createWindow({ .m_share = &windows[0] }, "Second", 400, 400);

    // Only enable vsync for the first of the windows to be swapped to
    // avoid waiting out the interval for each window
    windows[1].setVsync(false);

    // Place the second window to the right of the first
    {
        // TODO: implement this
        // int left, right;
        // glfwGetWindowFrameSize(windows[0].handle(), &left, NULL, &right, NULL);

        auto& [_1, pos, dim, _2, _3, _4, _5, _6] = windows[0].properties();
        windows[1].setWindowPos(pos.m_x + dim.m_width, pos.m_y);
    }

    // While objects are shared, the global context state is not and will
    // need to be set up for each context

    glUseProgram(program);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);

    while (wm.hasWindowOpened()) {
        const vec3 colors[] = { { 0.8f, 0.4f, 1.f }, { 0.3f, 0.4f, 1.f } };

        for (std::size_t i = 0; i < 2; i++) {
            windows[i].bind();

            for (auto& event : windows[i].poll()) {
                using EV = glfw_cpp::Event;
                using KC = glfw_cpp::KeyCode;
                using KS = glfw_cpp::KeyState;

                if (auto* e = event.getIf<EV::KeyPressed>()) {
                    if (e->m_key == KC::ESCAPE && e->m_state == KS::PRESS) {
                        // close both
                        windows[0].requestClose();
                        windows[1].requestClose();
                    }
                } else {
                    static int c = 0;
                    fprintf(stderr, "other event going on: %d\n", c++);
                }
            }

            auto [width, height] = windows[i].properties().m_framebufferSize;
            glViewport(0, 0, width, height);

            mat4x4 mvp;
            mat4x4_ortho(mvp, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f);
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
            glUniform3fv(color_location, 1, colors[i]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            windows[i].display();
            windows[i].unbind();
        }

        wm.waitEvents();
    }
}
