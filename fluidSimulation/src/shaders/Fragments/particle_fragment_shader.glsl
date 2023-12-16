#version 430

in vec3 normale;

uniform float density;
uniform float pressure;

out vec4 FragColor;

void main()
{
    //FragColor = vec4(normale, 1);

    //FragColor = vec4(density/5.f,density/5.f,density/5.f, 1);

    if(density < 100) FragColor = vec4(vec3(0,0,1), 1);
    else if(density == 100) FragColor = vec4(vec3(1,1,1), 1);
    else if(density > 100) FragColor = vec4(vec3(1,0,0), 1);
}
