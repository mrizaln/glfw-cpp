#ifndef PLANE_HPP_GR5KWQFD
#define PLANE_HPP_GR5KWQFD

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>

#include <array>
#include <cstddef>
#include <span>

class Plane
{
public:
    inline static constexpr std::size_t num_of_vertices = 6;

    Plane(float side_len = 1.0F)
        : m_side_len{ side_len }
    {
        auto vertices = Vertices{};

        for (auto i = std::size_t{ 0 }; i < num_of_vertices; ++i) {
            vertices[i] = {
                .position  = s_planeVertices[i] * m_side_len / 2.0F,
                .normal    = s_planeNormals[i],
                .tex_coord = s_planeTexCoords[i],
            };
        }

        set_buffers(vertices);
    }

    Plane(const Plane&)            = delete;
    Plane& operator=(const Plane&) = delete;
    Plane(Plane&&)                 = delete;
    Plane& operator=(Plane&&)      = delete;
    ~Plane() { delete_buffers(); }

    void draw() const
    {
        gl::glBindVertexArray(m_vao);
        gl::glDrawArrays(gl::GL_TRIANGLES, 0, static_cast<gl::GLsizei>(num_of_vertices));
        gl::glBindVertexArray(0);
    }

private:
    using Triple = glm::vec3;
    using Pair   = glm::vec2;

    struct VertexData
    {
        Triple position;
        Triple normal;
        Pair   tex_coord;
    };

    using Vertices = std::array<VertexData, num_of_vertices>;

    // clang-format off
    inline static constexpr std::array<Triple, num_of_vertices> s_planeVertices{ {
        {  1.0F, 0.0F,  1.0F },
        { -1.0F, 0.0F, -1.0F },
        { -1.0F, 0.0F,  1.0F },

        {  1.0F, 0.0F,  1.0F },
        {  1.0F, 0.0F, -1.0F },
        { -1.0F, 0.0F, -1.0F },
    } };
    // clang-format on

    // clang-format off
    inline static constexpr std::array<Triple, num_of_vertices> s_planeNormals{ {
        { 0.0F, 1.0F, 0.0F },
        { 0.0F, 1.0F, 0.0F },
        { 0.0F, 1.0F, 0.0F },

        { 0.0F, 1.0F, 0.0F },
        { 0.0F, 1.0F, 0.0F },
        { 0.0F, 1.0F, 0.0F },
    } };
    // clang-format on

    // clang-format off
    inline static constexpr std::array<Pair, num_of_vertices> s_planeTexCoords{ {
        { 1.0F, 1.0F },
        { 0.0F, 0.0F },
        { 0.0F, 1.0F },

        { 1.0F, 1.0F },
        { 1.0F, 0.0F },
        { 0.0F, 0.0F },
    } };
    // clang-format on

    float      m_side_len;
    gl::GLuint m_vao;
    gl::GLuint m_vbo;

    void set_buffers(std::span<VertexData, num_of_vertices> vertices)
    {
        constexpr auto stride = static_cast<gl::GLsizei>(sizeof(VertexData));

        gl::glGenVertexArrays(1, &m_vao);
        gl::glGenBuffers(1, &m_vbo);

        gl::glBindVertexArray(m_vao);
        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, m_vbo);
        gl::glBufferData(
            gl::GL_ARRAY_BUFFER,
            static_cast<gl::GLsizeiptr>(vertices.size() * stride),
            &vertices.front(),
            gl::GL_STATIC_DRAW
        );

        gl::glVertexAttribPointer(
            0,
            decltype(VertexData::position)::length(),
            gl::GL_FLOAT,
            gl::GL_FALSE,
            stride,
            (void*)offsetof(VertexData, position)
        );
        gl::glVertexAttribPointer(
            1,
            decltype(VertexData::normal)::length(),
            gl::GL_FLOAT,
            gl::GL_FALSE,
            stride,
            (void*)offsetof(VertexData, normal)
        );
        gl::glVertexAttribPointer(
            2,
            decltype(VertexData::tex_coord)::length(),
            gl::GL_FLOAT,
            gl::GL_FALSE,
            stride,
            (void*)offsetof(VertexData, tex_coord)
        );
        gl::glEnableVertexAttribArray(0);
        gl::glEnableVertexAttribArray(1);
        gl::glEnableVertexAttribArray(2);

        gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);
        gl::glBindVertexArray(0);
    }

    void delete_buffers()
    {
        gl::glDeleteVertexArrays(1, &m_vao);
        gl::glDeleteBuffers(1, &m_vbo);
    }
};

#endif /* end of include guard: PLANE_HPP_GR5KWQFD */
