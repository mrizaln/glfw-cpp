//========================================================================
// Simple multi-window example
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

#include <algorithm>
#include <array>
#include <cstdio>

#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

int main()
{
    auto glfw = glfw_cpp::init(
        glfw_cpp::Api::OpenGL{
            .m_loader = [](auto, auto proc) { gladLoadGLLoader((GLADloadproc)proc); },
        },
        [](auto level, auto msg) {
            if ((int)level >= (int)glfw_cpp::Instance::LogLevel::ERROR) {
                fprintf(stderr, "%s\n", msg.c_str());
            }
        }
    );
    auto wm = glfw->createWindowManager();

    auto [xpos, ypos, width, height] = glfw_cpp::getPrimaryMonitor().workArea();

    std::array<glfw_cpp::Window, 4> windows;
    for (int i = 0; i < 4; i++) {
        const int size = height / 5;
        const struct
        {
            float r, g, b;
        } colors[] = {
            { 0.95f, 0.32f, 0.11f },
            { 0.50f, 0.80f, 0.16f },
            { 0.f, 0.68f, 0.94f },
            { 0.98f, 0.74f, 0.04f },
        };

        auto hint = glfw_cpp::WindowHint{
            .m_flags = glfw_cpp::WindowHint::DEFAULT & ~glfw_cpp::WindowHint::DECORATED,
        };
        if (i > 0) {
            hint.m_flags &= ~glfw_cpp::WindowHint::FOCUS_ON_SHOW;
        }

        windows[i] = wm.createWindow(hint, "Learn Multi-Window Example", size, size);

        // set window pos will be queued inside WindowManager
        windows[i].setWindowPos(xpos + size * (1 + (i & 1)), ypos + size * (1 + (i >> 1)));

        glClearColor(colors[i].r, colors[i].g, colors[i].b, 1.f);
    }

    // process queued tasks
    wm.pollEvents();

    while (wm.hasWindowOpened()) {
        for (int i = 0; i < 4; i++) {
            windows[i].bind();

            glClear(GL_COLOR_BUFFER_BIT);

            if (windows[i].properties().m_keyState.isPressed(glfw_cpp::KeyCode::ESCAPE)) {
                std::ranges::for_each(windows, &glfw_cpp::Window::requestClose);
            }

            windows[i].display();
            windows[i].unbind();
        }

        wm.waitEvents();
    }
}
