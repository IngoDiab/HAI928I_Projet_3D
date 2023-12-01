#version 430

in vec3 normale;

out vec4 FragColor;

void main()
{
    FragColor = vec4(normale, 1.);
}
