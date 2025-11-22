/*****************************************************************************
 * Title:   GLBoing
 * Desc:    Tribute to Amiga Boing.
 * Author:  Jim Brooks  <gfx@jimbrooks.org>
 *          Original Amiga authors were R.J. Mical and Dale Luck.
 *          GLFW conversion by Marcus Geelnard
 *          glfw-cpp conversion by Muhammad Rizal N <mrizaln2000@gmail.com>
 *
 * Notes:   - 360' = 2*PI [radian]
 *
 *          - Distances between objects are created by doing a relative
 *            Z translations.
 *
 *          - Although OpenGL enticingly supports alpha-blending,
 *            the shadow of the original Boing didn't affect the color
 *            of the grid.
 *
 *          - [Marcus] Changed timing scheme from interval driven to frame-
 *            time based animation steps (which results in much smoother
 *            movement)
 *
 * History of Amiga Boing:
 *
 * Boing was demonstrated on the prototype Amiga (codenamed "Lorraine") in
 * 1985. According to legend, it was written ad-hoc in one night by
 * R. J. Mical and Dale Luck. Because the bouncing ball animation was so fast
 * and smooth, attendees did not believe the Amiga prototype was really doing
 * the rendering. Suspecting a trick, they began looking around the booth for
 * a hidden computer or VCR.
 *****************************************************************************/

#include <exception>
#include <iostream>

#include <glad/glad.h>
#include <glfw_cpp/glfw_cpp.hpp>
#include <linmath.h>

/*****************************************************************************
 * Various declarations and macros
 *****************************************************************************/

/* Prototypes */
void init(void);
void display(void);
void reshape(GLFWwindow* window, int w, int h);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double x, double y);
void drawBoingBall(void);
void bounceBall(double dt);
void drawBoingBallBand(GLfloat long_lo, GLfloat long_hi);
void drawGrid(void);

#define RADIUS         70.f
#define STEP_LONGITUDE 22.5f    // 22.5 makes 8 bands like original Boing
#define STEP_LATITUDE  22.5f

#define DIST_BALL (RADIUS * 2.f + RADIUS * 0.1f)

#define VIEW_SCENE_DIST (DIST_BALL * 3.f + 200.f)    // distance from viewer to middle of boing area
#define GRID_SIZE       (RADIUS * 4.5f)              // length (width) of grid
#define BOUNCE_HEIGHT   (RADIUS * 2.1f)
#define BOUNCE_WIDTH    (RADIUS * 2.1f)

#define SHADOW_OFFSET_X -20.f
#define SHADOW_OFFSET_Y 10.f
#define SHADOW_OFFSET_Z 0.f

#define WALL_L_OFFSET 0.f
#define WALL_R_OFFSET 5.f

/* Animation speed (50.0 mimics the original GLUT demo speed) */
#define ANIMATION_SPEED 50.f

/* Maximum allowed delta time per physics iteration */
#define MAX_DELTA_T 0.02f

/* Draw ball, or its shadow */
enum class DrawBall
{
    DRAW_BALL,
    DRAW_BALL_SHADOW
};

/* Vertex type */
struct Vertex
{
    float x;
    float y;
    float z;
};

/* Global vars */
int      windowed_xpos, windowed_ypos, windowed_width, windowed_height;
int      width, height;
GLfloat  deg_rot_y     = 0.f;
GLfloat  deg_rot_y_inc = 2.f;
int      override_pos  = false;
GLfloat  cursor_x      = 0.f;
GLfloat  cursor_y      = 0.f;
GLfloat  ball_x        = -RADIUS;
GLfloat  ball_y        = -RADIUS;
GLfloat  ball_x_inc    = 1.f;
GLfloat  ball_y_inc    = 2.f;
DrawBall drawBallHow;
double   t;
double   t_old = 0.f;
double   dt;

/* Random number generator */
#ifndef RAND_MAX
#define RAND_MAX 4095
#endif

/*****************************************************************************
 * Truncate a degree.
 *****************************************************************************/
GLfloat truncateDeg(GLfloat deg)
{
    if (deg >= 360.f) {
        return (deg - 360.f);
    } else {
        return deg;
    }
}

/*****************************************************************************
 * Convert a degree (360-based) into a radian.
 * 360' = 2 * PI
 *****************************************************************************/
double deg2rad(double deg)
{
    return deg / 360 * (2 * M_PI);
}

/*****************************************************************************
 * 360' sin().
 *****************************************************************************/
double sinDeg(double deg)
{
    return sin(deg2rad(deg));
}

/*****************************************************************************
 * 360' cos().
 *****************************************************************************/
double cosDog(double deg)
{
    return cos(deg2rad(deg));
}

/*****************************************************************************
 * Compute a cross product (for a normal vector).
 *
 * c = a x b
 *****************************************************************************/
void crossProduct(Vertex a, Vertex b, Vertex c, Vertex* n)
{
    GLfloat u1, u2, u3;
    GLfloat v1, v2, v3;

    u1 = b.x - a.x;
    u2 = b.y - a.y;
    u3 = b.y - a.z;

    v1 = c.x - a.x;
    v2 = c.y - a.y;
    v3 = c.z - a.z;

    n->x = u2 * v3 - v2 * u3;
    n->y = u3 * v1 - v3 * u1;
    n->z = u1 * v2 - v1 * u2;
}

#define BOING_DEBUG 0

/*****************************************************************************
 * init()
 *****************************************************************************/
void init(void)
{
    /*
     * Clear background.
     */
    glClearColor(0.55f, 0.55f, 0.55f, 0.f);

    glShadeModel(GL_FLAT);
}

/*****************************************************************************
 * display()
 *****************************************************************************/
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();

    drawBallHow = DrawBall::DRAW_BALL_SHADOW;
    drawBoingBall();

    drawGrid();

    drawBallHow = DrawBall::DRAW_BALL;
    drawBoingBall();

    glPopMatrix();
    glFlush();
}

/*****************************************************************************
 * reshape()
 *****************************************************************************/
void reshape(int w, int h)
{
    mat4x4 projection, view;

    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    glMatrixMode(GL_PROJECTION);
    mat4x4_perspective(
        projection, 2.f * (float)atan2(RADIUS, 200.f), (float)w / (float)h, 1.f, VIEW_SCENE_DIST
    );
    glLoadMatrixf((const GLfloat*)projection);

    glMatrixMode(GL_MODELVIEW);
    {
        vec3 eye    = { 0.f, 0.f, VIEW_SCENE_DIST };
        vec3 center = { 0.f, 0.f, 0.f };
        vec3 up     = { 0.f, -1.f, 0.f };
        mat4x4_look_at(view, eye, center, up);
    }
    glLoadMatrixf((const GLfloat*)view);
}

void handleKeyEvent(glfw_cpp::Window& window, const glfw_cpp::event::KeyPressed& event)
{
    using K = glfw_cpp::KeyCode;
    using M = glfw_cpp::ModifierKey;
    using S = glfw_cpp::KeyState;

    auto& [key, _, state, mods] = event;

    if (state != S::Press) {
        return;
    }

    if (key == K::Escape && mods.none()) {
        window.request_close();
    }

    // should be fullscreen and off (but I haven't implemented adding monitor)
    // TODO: change to fullscreen toggle
    if ((key == K::Enter && mods.test(M::Alt)) || (key == K::F11 && mods.test(M::Alt))) {
        if (window.properties().attribute.maximized) {
            window.restore();
            std::cout << "restore\n";
        } else {
            window.maximize();
            std::cout << "maximize\n";
        }
    }
}

static void setBallPos(GLfloat x, GLfloat y)
{
    ball_x = ((float)width / 2) - x;
    ball_y = y - ((float)height / 2);
}

void handleMouseButtonEvent(const glfw_cpp::event::ButtonPressed& event)
{
    using B = glfw_cpp::MouseButton;
    using S = glfw_cpp::MouseButtonState;

    auto& [button, action, mods] = event;

    if (button != B::Left) {
        return;
    }

    if (action == S::Press) {
        override_pos = true;
        setBallPos(cursor_x, cursor_y);
    } else {
        override_pos = false;
    }
}

void handleCursorEvent(const glfw_cpp::event::CursorMoved& event)
{
    auto& [x, y, dx, dy] = event;

    cursor_x = (float)x;
    cursor_y = (float)y;

    if (override_pos) {
        setBallPos(cursor_x, cursor_y);
    }
}

/*****************************************************************************
 * Draw the Boing ball.
 *
 * The Boing ball is sphere in which each facet is a rectangle.
 * Facet colors alternate between red and white.
 * The ball is built by stacking latitudinal circles.  Each circle is composed
 * of a widely-separated set of points, so that each facet is noticeably large.
 *****************************************************************************/
void drawBoingBall(void)
{
    GLfloat lon_deg; /* degree of longitude */
    double  dt_total, dt2;

    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);

    /*
     * Another relative Z translation to separate objects.
     */
    glTranslatef(0.0, 0.0, DIST_BALL);

    /* Update ball position and rotation (iterate if necessary) */
    dt_total = dt;
    while (dt_total > 0.0) {
        dt2       = dt_total > MAX_DELTA_T ? MAX_DELTA_T : dt_total;
        dt_total -= dt2;
        bounceBall(dt2);
        deg_rot_y = truncateDeg(deg_rot_y + deg_rot_y_inc * ((float)dt2 * ANIMATION_SPEED));
    }

    /* Set ball position */
    glTranslatef(ball_x, ball_y, 0.0);

    /*
     * Offset the shadow.
     */
    if (drawBallHow == DrawBall::DRAW_BALL_SHADOW) {
        glTranslatef(SHADOW_OFFSET_X, SHADOW_OFFSET_Y, SHADOW_OFFSET_Z);
    }

    /*
     * Tilt the ball.
     */
    glRotatef(-20.0, 0.0, 0.0, 1.0);

    /*
     * Continually rotate ball around Y axis.
     */
    glRotatef(deg_rot_y, 0.0, 1.0, 0.0);

    /*
     * Set OpenGL state for Boing ball.
     */
    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    /*
     * Build a faceted latitude slice of the Boing ball,
     * stepping same-sized vertical bands of the sphere.
     */
    for (lon_deg = 0; lon_deg < 180; lon_deg += STEP_LONGITUDE) {
        /*
         * Draw a latitude circle at this longitude.
         */
        drawBoingBallBand(lon_deg, lon_deg + STEP_LONGITUDE);
    }

    glPopMatrix();

    return;
}

/*****************************************************************************
 * Bounce the ball.
 *****************************************************************************/
void bounceBall(double delta_t)
{
    GLfloat sign;
    GLfloat deg;

    if (override_pos) {
        return;
    }

    /* Bounce on walls */
    if (ball_x > (BOUNCE_WIDTH / 2 + WALL_R_OFFSET)) {
        ball_x_inc    = -0.5f - 0.75f * (GLfloat)rand() / (GLfloat)RAND_MAX;
        deg_rot_y_inc = -deg_rot_y_inc;
    }
    if (ball_x < -(BOUNCE_HEIGHT / 2 + WALL_L_OFFSET)) {
        ball_x_inc    = 0.5f + 0.75f * (GLfloat)rand() / (GLfloat)RAND_MAX;
        deg_rot_y_inc = -deg_rot_y_inc;
    }

    /* Bounce on floor / roof */
    if (ball_y > BOUNCE_HEIGHT / 2) {
        ball_y_inc = -0.75f - 1.f * (GLfloat)rand() / (GLfloat)RAND_MAX;
    }
    if (ball_y < -BOUNCE_HEIGHT / 2 * 0.85) {
        ball_y_inc = 0.75f + 1.f * (GLfloat)rand() / (GLfloat)RAND_MAX;
    }

    /* Update ball position */
    ball_x += ball_x_inc * ((float)delta_t * ANIMATION_SPEED);
    ball_y += ball_y_inc * ((float)delta_t * ANIMATION_SPEED);

    /*
     * Simulate the effects of gravity on Y movement.
     */
    if (ball_y_inc < 0) {
        sign = -1.0;
    } else {
        sign = 1.0;
    }

    deg = (ball_y + BOUNCE_HEIGHT / 2) * 90 / BOUNCE_HEIGHT;
    if (deg > 80) {
        deg = 80;
    }
    if (deg < 10) {
        deg = 10;
    }

    ball_y_inc = sign * 4.f * (float)sinDeg(deg);
}

/*****************************************************************************
 * Draw a faceted latitude band of the Boing ball.
 *
 * Parms:   long_lo, long_hi
 *          Low and high longitudes of slice, resp.
 *****************************************************************************/
void drawBoingBallBand(GLfloat long_lo, GLfloat long_hi)
{
    Vertex     vert_ne; /* "ne" means south-east, so on */
    Vertex     vert_nw;
    Vertex     vert_sw;
    Vertex     vert_se;
    Vertex     vert_norm;
    GLfloat    lat_deg;
    static int colorToggle = 0;

    /*
     * Iterate through the points of a latitude circle.
     * A latitude circle is a 2D set of X,Z points.
     */
    for (lat_deg = 0; lat_deg <= (360 - STEP_LATITUDE); lat_deg += STEP_LATITUDE) {
        /*
         * Color this polygon with red or white.
         */
        if (colorToggle) {
            glColor3f(0.8f, 0.1f, 0.1f);
        } else {
            glColor3f(0.95f, 0.95f, 0.95f);
        }
#if 0
      if ( lat_deg >= 180 )
         if ( colorToggle )
            glColor3f( 0.1f, 0.8f, 0.1f );
         else
            glColor3f( 0.5f, 0.5f, 0.95f );
#endif
        colorToggle = !colorToggle;

        /*
         * Change color if drawing shadow.
         */
        if (drawBallHow == DrawBall::DRAW_BALL_SHADOW) {
            glColor3f(0.35f, 0.35f, 0.35f);
        }

        /*
         * Assign each Y.
         */
        vert_ne.y = vert_nw.y = (float)cosDog(long_hi) * RADIUS;
        vert_sw.y = vert_se.y = (float)cosDog(long_lo) * RADIUS;

        /*
         * Assign each X,Z with sin,cos values scaled by latitude radius indexed by longitude.
         * Eg, long=0 and long=180 are at the poles, so zero scale is sin(longitude),
         * while long=90 (sin(90)=1) is at equator.
         */
        vert_ne.x = (float)cosDog(lat_deg) * (RADIUS * (float)sinDeg(long_lo + STEP_LONGITUDE));
        vert_se.x = (float)cosDog(lat_deg) * (RADIUS * (float)sinDeg(long_lo));
        vert_nw.x = (float)cosDog(lat_deg + STEP_LATITUDE)
                  * (RADIUS * (float)sinDeg(long_lo + STEP_LONGITUDE));
        vert_sw.x = (float)cosDog(lat_deg + STEP_LATITUDE) * (RADIUS * (float)sinDeg(long_lo));

        vert_ne.z = (float)sinDeg(lat_deg) * (RADIUS * (float)sinDeg(long_lo + STEP_LONGITUDE));
        vert_se.z = (float)sinDeg(lat_deg) * (RADIUS * (float)sinDeg(long_lo));
        vert_nw.z = (float)sinDeg(lat_deg + STEP_LATITUDE)
                  * (RADIUS * (float)sinDeg(long_lo + STEP_LONGITUDE));
        vert_sw.z = (float)sinDeg(lat_deg + STEP_LATITUDE) * (RADIUS * (float)sinDeg(long_lo));

        /*
         * Draw the facet.
         */
        glBegin(GL_POLYGON);

        crossProduct(vert_ne, vert_nw, vert_sw, &vert_norm);
        glNormal3f(vert_norm.x, vert_norm.y, vert_norm.z);

        glVertex3f(vert_ne.x, vert_ne.y, vert_ne.z);
        glVertex3f(vert_nw.x, vert_nw.y, vert_nw.z);
        glVertex3f(vert_sw.x, vert_sw.y, vert_sw.z);
        glVertex3f(vert_se.x, vert_se.y, vert_se.z);

        glEnd();

#if BOING_DEBUG
        printf("----------------------------------------------------------- \n");
        printf("lat = %f  long_lo = %f  long_hi = %f \n", lat_deg, long_lo, long_hi);
        printf("vert_ne  x = %.8f  y = %.8f  z = %.8f \n", vert_ne.x, vert_ne.y, vert_ne.z);
        printf("vert_nw  x = %.8f  y = %.8f  z = %.8f \n", vert_nw.x, vert_nw.y, vert_nw.z);
        printf("vert_se  x = %.8f  y = %.8f  z = %.8f \n", vert_se.x, vert_se.y, vert_se.z);
        printf("vert_sw  x = %.8f  y = %.8f  z = %.8f \n", vert_sw.x, vert_sw.y, vert_sw.z);
#endif
    }

    /*
     * Toggle color so that next band will opposite red/white colors than this one.
     */
    colorToggle = !colorToggle;

    /*
     * This circular band is done.
     */
    return;
}

/*****************************************************************************
 * Draw the purple grid of lines, behind the Boing ball.
 * When the Workbench is dropped to the bottom, Boing shows 12 rows.
 *****************************************************************************/
void drawGrid(void)
{
    int           row, col;
    const int     rowTotal  = 12;       /* must be divisible by 2 */
    const int     colTotal  = rowTotal; /* must be same as rowTotal */
    const GLfloat widthLine = 2.0;      /* should be divisible by 2 */
    const GLfloat sizeCell  = GRID_SIZE / rowTotal;
    const GLfloat z_offset  = -40.0;
    GLfloat       xl, xr;
    GLfloat       yt, yb;

    glPushMatrix();
    glDisable(GL_CULL_FACE);

    /*
     * Another relative Z translation to separate objects.
     */
    glTranslatef(0.0, 0.0, DIST_BALL);

    /*
     * Draw vertical lines (as skinny 3D rectangles).
     */
    for (col = 0; col <= colTotal; col++) {
        /*
         * Compute co-ords of line.
         */
        xl = -GRID_SIZE / 2 + (float)col * sizeCell;
        xr = xl + widthLine;

        yt = GRID_SIZE / 2;
        yb = -GRID_SIZE / 2 - widthLine;

        glBegin(GL_POLYGON);

        glColor3f(0.6f, 0.1f, 0.6f); /* purple */

        glVertex3f(xr, yt, z_offset); /* NE */
        glVertex3f(xl, yt, z_offset); /* NW */
        glVertex3f(xl, yb, z_offset); /* SW */
        glVertex3f(xr, yb, z_offset); /* SE */

        glEnd();
    }

    /*
     * Draw horizontal lines (as skinny 3D rectangles).
     */
    for (row = 0; row <= rowTotal; row++) {
        /*
         * Compute co-ords of line.
         */
        yt = GRID_SIZE / 2 - (float)row * sizeCell;
        yb = yt - widthLine;

        xl = -GRID_SIZE / 2;
        xr = GRID_SIZE / 2 + widthLine;

        glBegin(GL_POLYGON);

        glColor3f(0.6f, 0.1f, 0.6f); /* purple */

        glVertex3f(xr, yt, z_offset); /* NE */
        glVertex3f(xl, yt, z_offset); /* NW */
        glVertex3f(xl, yb, z_offset); /* SW */
        glVertex3f(xr, yb, z_offset); /* SE */

        glEnd();
    }

    glPopMatrix();

    return;
}

/*======================================================================*
 * main()
 *======================================================================*/

int main(void)
try {
    auto glfw = glfw_cpp::init(
        glfw_cpp::api::OpenGL{
            // default OpenGL
            .loader = [](auto, auto proc) { gladLoadGLLoader((GLADloadproc)proc); },
        },
        [](auto level, auto msg) { std::cout << std::format("glfw-cpp [{}]: {}\n", to_string(level), msg); }
    );

    auto window = glfw->create_window({}, "Boing | classic Amiga demo (glfw-cpp)", 400, 400);

    window.lock_current_aspect_ratio();

    reshape(window.properties().dimensions.width, window.properties().dimensions.height);
    init();

    window.run([&](const glfw_cpp::EventQueue& events) {
        t     = glfw_cpp::get_time();
        dt    = t - t_old;
        t_old = t;

        namespace ev = glfw_cpp::event;
        events.visit(ev::Overload{
            [&](const ev::FramebufferResized& e) { reshape(e.width, e.height); },
            [&](const ev::KeyPressed& e) { handleKeyEvent(window, e); },
            [&](const ev::CursorMoved& e) { handleCursorEvent(e); },
            [&](const ev::ButtonPressed& e) { handleMouseButtonEvent(e); },
            [](auto&) {},
        });

        display();

        glfw->poll_events();
    });
} catch (std::exception& e) {
    std::cerr << "Exception occurred: " << e.what() << '\n';
    return 1;
}
