#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    // Imposta la dimensione del punto. Puoi giocarci con questo valore.
    gl_PointSize = 25.0;
}