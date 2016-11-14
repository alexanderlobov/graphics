#version 130

attribute vec3 coord;
uniform mat4 mvp;
varying vec3 texcoords;

void main()
{
    texcoords = coord;
    gl_Position = mvp * vec4(coord, 1.0);
}
