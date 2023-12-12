#version 430

in vec3 normale;

uniform float density;

out vec4 FragColor;

void main()
{
    //FragColor = vec4(normale, 1);
    FragColor = vec4(vec3(density/10.f), 1);
}
