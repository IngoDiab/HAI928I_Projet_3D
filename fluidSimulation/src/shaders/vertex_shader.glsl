#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec3 normale;

void main()
{
    gl_Position =  projection_matrix * view_matrix * model_matrix * vec4(vertexPosition,1);

    vec4 normaleV4 = normalize(transpose(inverse(model_matrix)) * vec4(vertexNormal, 1.0));
    normale = normaleV4.xyz;
}

