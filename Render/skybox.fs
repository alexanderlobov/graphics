#version 130

varying vec3 texcoords;
uniform samplerCube cubeTexture;
out vec4 frag_color;

void main()
{
    frag_color = texture(cubeTexture, texcoords);
}
