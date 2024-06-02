/*
 * 3-D gear wheels.  This program is in the public domain.
 *
 * Command line options:
 *    -info      print GL implementation information
 *    -exit      automatically exit after 30 seconds
 *
 *
 * Brian Paul
 *
 *
 * Marcus Geelnard:
 *   - Conversion to GLFW
 *   - Time based rendering (frame rate independent)
 *   - Slightly modified camera that should work better for stereo viewing
 *
 *
 * Camilla Löwy:
 *   - Removed FPS counter (this is not a benchmark)
 *   - Added a few comments
 *   - Enabled vsync
 *
 * Muhammad Rizal Nurromdhoni <mrizaln2000@gmail.com>:
 *   - Convert to glfw-cpp (and C++)
 */

#include <exception>
#include <iostream>
#include <cmath>

#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>

/**

  Draw a gear wheel.  You'll probably want to call this function when
  building a display list since we do a lot of trig here.

  Input:  inner_radius - radius of hole at center
          outer_radius - radius at center of teeth
          width - width of gear teeth - number of teeth
          tooth_depth - depth of tooth

 **/

static void gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width, GLint teeth, GLfloat tooth_depth)
{
    GLint   i;
    GLfloat r0, r1, r2;
    GLfloat angle, da;
    GLfloat u, v, len;

    r0 = inner_radius;
    r1 = outer_radius - tooth_depth / 2.f;
    r2 = outer_radius + tooth_depth / 2.f;

    da = 2.f * (float)M_PI / teeth / 4.f;

    glShadeModel(GL_FLAT);

    glNormal3f(0.f, 0.f, 1.f);

    /* draw front face */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.f * (float)M_PI / teeth;
        glVertex3f(r0 * (float)cos(angle), r0 * (float)sin(angle), width * 0.5f);
        glVertex3f(r1 * (float)cos(angle), r1 * (float)sin(angle), width * 0.5f);
        if (i < teeth) {
            glVertex3f(r0 * (float)cos(angle), r0 * (float)sin(angle), width * 0.5f);
            glVertex3f(r1 * (float)cos(angle + 3 * da), r1 * (float)sin(angle + 3 * da), width * 0.5f);
        }
    }
    glEnd();

    /* draw front sides of teeth */
    glBegin(GL_QUADS);
    da = 2.f * (float)M_PI / teeth / 4.f;
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float)M_PI / teeth;

        glVertex3f(r1 * (float)cos(angle), r1 * (float)sin(angle), width * 0.5f);
        glVertex3f(r2 * (float)cos(angle + da), r2 * (float)sin(angle + da), width * 0.5f);
        glVertex3f(r2 * (float)cos(angle + 2 * da), r2 * (float)sin(angle + 2 * da), width * 0.5f);
        glVertex3f(r1 * (float)cos(angle + 3 * da), r1 * (float)sin(angle + 3 * da), width * 0.5f);
    }
    glEnd();

    glNormal3f(0.0, 0.0, -1.0);

    /* draw back face */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.f * (float)M_PI / teeth;
        glVertex3f(r1 * (float)cos(angle), r1 * (float)sin(angle), -width * 0.5f);
        glVertex3f(r0 * (float)cos(angle), r0 * (float)sin(angle), -width * 0.5f);
        if (i < teeth) {
            glVertex3f(r1 * (float)cos(angle + 3 * da), r1 * (float)sin(angle + 3 * da), -width * 0.5f);
            glVertex3f(r0 * (float)cos(angle), r0 * (float)sin(angle), -width * 0.5f);
        }
    }
    glEnd();

    /* draw back sides of teeth */
    glBegin(GL_QUADS);
    da = 2.f * (float)M_PI / teeth / 4.f;
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float)M_PI / teeth;

        glVertex3f(r1 * (float)cos(angle + 3 * da), r1 * (float)sin(angle + 3 * da), -width * 0.5f);
        glVertex3f(r2 * (float)cos(angle + 2 * da), r2 * (float)sin(angle + 2 * da), -width * 0.5f);
        glVertex3f(r2 * (float)cos(angle + da), r2 * (float)sin(angle + da), -width * 0.5f);
        glVertex3f(r1 * (float)cos(angle), r1 * (float)sin(angle), -width * 0.5f);
    }
    glEnd();

    /* draw outward faces of teeth */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < teeth; i++) {
        angle = i * 2.f * (float)M_PI / teeth;

        glVertex3f(r1 * (float)cos(angle), r1 * (float)sin(angle), width * 0.5f);
        glVertex3f(r1 * (float)cos(angle), r1 * (float)sin(angle), -width * 0.5f);
        u    = r2 * (float)cos(angle + da) - r1 * (float)cos(angle);
        v    = r2 * (float)sin(angle + da) - r1 * (float)sin(angle);
        len  = (float)sqrt(u * u + v * v);
        u   /= len;
        v   /= len;
        glNormal3f(v, -u, 0.0);
        glVertex3f(r2 * (float)cos(angle + da), r2 * (float)sin(angle + da), width * 0.5f);
        glVertex3f(r2 * (float)cos(angle + da), r2 * (float)sin(angle + da), -width * 0.5f);
        glNormal3f((float)cos(angle), (float)sin(angle), 0.f);
        glVertex3f(r2 * (float)cos(angle + 2 * da), r2 * (float)sin(angle + 2 * da), width * 0.5f);
        glVertex3f(r2 * (float)cos(angle + 2 * da), r2 * (float)sin(angle + 2 * da), -width * 0.5f);
        u = r1 * (float)cos(angle + 3 * da) - r2 * (float)cos(angle + 2 * da);
        v = r1 * (float)sin(angle + 3 * da) - r2 * (float)sin(angle + 2 * da);
        glNormal3f(v, -u, 0.f);
        glVertex3f(r1 * (float)cos(angle + 3 * da), r1 * (float)sin(angle + 3 * da), width * 0.5f);
        glVertex3f(r1 * (float)cos(angle + 3 * da), r1 * (float)sin(angle + 3 * da), -width * 0.5f);
        glNormal3f((float)cos(angle), (float)sin(angle), 0.f);
    }

    glVertex3f(r1 * (float)cos(0), r1 * (float)sin(0), width * 0.5f);
    glVertex3f(r1 * (float)cos(0), r1 * (float)sin(0), -width * 0.5f);

    glEnd();

    glShadeModel(GL_SMOOTH);

    /* draw inside radius cylinder */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
        angle = i * 2.f * (float)M_PI / teeth;
        glNormal3f(-(float)cos(angle), -(float)sin(angle), 0.f);
        glVertex3f(r0 * (float)cos(angle), r0 * (float)sin(angle), -width * 0.5f);
        glVertex3f(r0 * (float)cos(angle), r0 * (float)sin(angle), width * 0.5f);
    }
    glEnd();
}

static GLfloat view_rotx = 20.f, view_roty = 30.f, view_rotz = 0.f;
static GLint   gear1, gear2, gear3;
static GLfloat angle = 0.f;

/* OpenGL draw function & timing */
static void draw()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glRotatef(view_rotx, 1.0, 0.0, 0.0);
    glRotatef(view_roty, 0.0, 1.0, 0.0);
    glRotatef(view_rotz, 0.0, 0.0, 1.0);

    glPushMatrix();
    glTranslatef(-3.0, -2.0, 0.0);
    glRotatef(angle, 0.0, 0.0, 1.0);
    glCallList(gear1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3.1f, -2.f, 0.f);
    glRotatef(-2.f * angle - 9.f, 0.f, 0.f, 1.f);
    glCallList(gear2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.1f, 4.2f, 0.f);
    glRotatef(-2.f * angle - 25.f, 0.f, 0.f, 1.f);
    glCallList(gear3);
    glPopMatrix();

    glPopMatrix();
}

/* update animation parameters */
static void animate()
{
    angle = 100.f * (float)glfw_cpp::getTime();
}

/* change view angle, exit upon ESC */
void key(glfw_cpp::Window& window, const glfw_cpp::Event::KeyPressed& event)
{
    using K = glfw_cpp::KeyCode;
    using M = glfw_cpp::ModifierKey;
    using S = glfw_cpp::KeyState;

    auto& [key, _, action, mods] = event;

    if (action == S::RELEASE) {
        return;
    }

    switch (key) {
    case K::Z:
        if (mods.test(M::SHIFT)) {
            view_rotz -= 5.0f;
        } else {
            view_rotz += 5.0f;
        }
        break;
    case K::ESCAPE: window.requestClose();
    case K::UP: view_rotx += 5.0f; break;
    case K::DOWN: view_rotx -= 5.0f; break;
    case K::LEFT: view_roty += 5.0f; break;
    case K::RIGHT: view_roty -= 5.0f; break;
    default: return;
    }
}

/* new window size */
void reshape(int width, int height)
{
    GLfloat h = (GLfloat)height / (GLfloat)width;
    GLfloat xmax, znear, zfar;

    znear = 5.0f;
    zfar  = 30.0f;
    xmax  = znear * 0.5f;

    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-xmax, xmax, -xmax * h, xmax * h, znear, zfar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -20.0);
}

/* program & OpenGL initialization */
static void init()
{
    static GLfloat pos[4]   = { 5.f, 5.f, 10.f, 0.f };
    static GLfloat red[4]   = { 0.8f, 0.1f, 0.f, 1.f };
    static GLfloat green[4] = { 0.f, 0.8f, 0.2f, 1.f };
    static GLfloat blue[4]  = { 0.2f, 0.2f, 1.f, 1.f };

    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    /* make the gears */
    gear1 = glGenLists(1);
    glNewList(gear1, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
    gear(1.f, 4.f, 1.f, 20, 0.7f);
    glEndList();

    gear2 = glGenLists(1);
    glNewList(gear2, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
    gear(0.5f, 2.f, 2.f, 10, 0.7f);
    glEndList();

    gear3 = glGenLists(1);
    glNewList(gear3, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
    gear(1.3f, 2.f, 0.5f, 10, 0.7f);
    glEndList();

    glEnable(GL_NORMALIZE);
}

/* program entry */
int main()
{
    try {
        auto glfw = glfw_cpp::init(
            glfw_cpp::Api::OpenGL{
                // default OpenGL
                .m_loader = [](auto, auto proc) { gladLoadGLLoader((GLADloadproc)proc); },
            },
            [](auto level, auto msg) { std::cerr << std::format("glfw-cpp [{}]: {}\n", (int)level, msg); }
        );

        using H = glfw_cpp::WindowHint;
        H hint  = { .m_flags = H::DEFAULT | H::TRANSPARENT_FRAMEBUFFER, .m_depthBits = 16 };

        auto wm     = glfw->createWindowManager();
        auto window = wm.createWindow(hint, "Gears", 300, 300);

        reshape(window.properties().m_dimension.m_width, window.properties().m_dimension.m_height);
        init();

        window.run([&](auto&& events) {
            using EV = glfw_cpp::Event;
            for (const EV& event : events) {
                if (auto* e = event.getIf<EV::FramebufferResized>()) {
                    reshape(e->m_width, e->m_height);
                } else if (auto* e = event.getIf<EV::KeyPressed>()) {
                    key(window, *e);
                }
            }

            draw();
            animate();

            wm.pollEvents();
        });
    } catch (std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << '\n';
        return 1;
    }
}
