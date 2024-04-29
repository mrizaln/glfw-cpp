#version 330 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 a_texCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(a_pos.xzy, 1.0);
    TexCoords   = a_texCoords;
}
