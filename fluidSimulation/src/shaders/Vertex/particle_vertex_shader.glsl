#version 430

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

uniform mat4 model_matrix = mat4(1.0f);
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec3 o_normale;
out vec3 o_positionWorld;

void main()
{
    vec4 o_positionWorldV4 = model_matrix * vec4(vertexPosition,1);
    o_positionWorld = o_positionWorldV4.xyz;
    gl_Position =  projection_matrix * view_matrix * o_positionWorldV4;
    o_normale = vertexNormal;
}

