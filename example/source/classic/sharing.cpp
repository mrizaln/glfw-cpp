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

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>

#include <glfw_cpp/glfw_cpp.hpp>

#include <getopt.h>

#include <linmath.h>

#include <array>
#include <cstdio>
#include <cstdlib>

using namespace gl;    // from <glbinding/gl/gl.h>

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

    auto glfw = glfw_cpp::init({});

    glfw->set_error_callback([](auto code, auto msg) {
        fprintf(stderr, "glfw-cpp [%20s]: %s\n", to_string(code).data(), msg.data());
    });

    glfw->apply_hints({
        .api = glfw_cpp::api::OpenGL{
            .version_major = 2,
            .version_minor = 0,
        },
    });

    auto windows = std::array<glfw_cpp::Window, 2>{};

    windows[0] = glfw->create_window(400, 400, "First | Sharing (glfw-cpp)");

    glfw_cpp::make_current(windows[0].handle());
    glbinding::initialize(0, glfw_cpp::get_proc_address, true);

    // Create the OpenGL objects inside the first context, created above
    // All objects will be shared with the second context, created below
    {
        int    x, y;
        char   pixels[16 * 16];
        GLuint vertex_shader, fragment_shader;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        srand((unsigned int)glfw_cpp::get_timer_value());

        for (y = 0; y < 16; y++) {
            for (x = 0; x < 16; x++) {
                pixels[y * 16 + x] = (char)(rand() % 256);
            }
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 16, 16, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);
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

    windows[1] = glfw->create_window(400, 400, "Second | Sharing (glfw-cpp)", nullptr, windows[0].handle());

    glfw_cpp::make_current(windows[1].handle());
    glbinding::initialize(1, glfw_cpp::get_proc_address, true);

    // Only enable vsync for the first of the windows to be swapped to
    // avoid waiting out the interval for each window
    windows[1].set_vsync(false);

    // Place the second window to the right of the first
    if (glfw->platform() != glfw_cpp::hint::Platform::Wayland) {
        // TODO: implement this
        // int left, right;
        // glfwGetWindowFrameSize(windows[0].handle(), &left, NULL, &right, NULL);

        auto& [title, pos, dim, frame, cursor, mouse, keys, mon] = windows[0].properties();
        windows[1].set_window_pos(pos.x + dim.width, pos.y);
    }

    // While objects are shared, the global context state is not and will
    // need to be set up for each context

    glUseProgram(program);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);

    while (glfw->has_window_opened()) {
        const vec3 colors[] = { { 0.8f, 0.4f, 1.f }, { 0.3f, 0.4f, 1.f } };

        for (std::size_t i = 0; i < 2; i++) {
            if (windows[i].should_close()) {
                if (windows[i].attributes().visible) {
                    windows[i].hide();
                }
                continue;
            }

            glfw_cpp::make_current(windows[i].handle());
            glbinding::useContext(i);

            for (const auto& event : windows[i].swap_events()) {
                namespace ev = glfw_cpp::event;
                using KC     = glfw_cpp::KeyCode;
                using KS     = glfw_cpp::KeyState;

                if (auto* e = event.get_if<ev::KeyPressed>()) {
                    if (e->key == KC::Escape && e->state == KS::Press) {
                        // close both
                        windows[0].request_close();
                        windows[1].request_close();
                    }
                }
            }

            auto [width, height] = windows[i].properties().framebuffer_size;
            glViewport(0, 0, width, height);

            mat4x4 mvp;
            mat4x4_ortho(mvp, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f);
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
            glUniform3fv(color_location, 1, colors[i]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            windows[i].swap_buffers();
        }

        glfw->wait_events();
    }
}
