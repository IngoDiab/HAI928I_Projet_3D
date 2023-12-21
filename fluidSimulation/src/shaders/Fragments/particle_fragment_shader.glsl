#version 430

in vec3 o_normale;
in vec3 o_positionWorld;

uniform vec3 cameraPosition;

out vec4 FragColor;

const vec3 LIGHT_POSITION = vec3(0,14,0);
const float LIGHT_POWER = 200;
const vec3 LIGHT_COLOR = vec3(1,1,1);

const vec3 WATER_COLOR = vec3(0.137,0.537,0.855);
const vec3 WATER_SPECULAR = vec3(1,1,1);
const float WATER_SHININESS = 30;

vec3 Phong()
{
    vec3 _lightColor = LIGHT_POWER*LIGHT_COLOR/(length(LIGHT_POSITION - o_positionWorld)*length(LIGHT_POSITION - o_positionWorld));

    vec3 _lightDirection = normalize(LIGHT_POSITION - o_positionWorld);
    float _clampedDotDiffuse = max(dot(_lightDirection, o_normale), 0.0);
    vec3 _diffuse = WATER_COLOR * _clampedDotDiffuse * _lightColor;

    vec3 _viewDirection = normalize(cameraPosition-o_positionWorld);
    vec3 _reflectDirection = reflect(-_lightDirection,o_normale);
    float _clampedDotSpecular = pow(max(dot(_viewDirection, _reflectDirection), 0.0), WATER_SHININESS);
    vec3 _specular = WATER_SPECULAR * _clampedDotSpecular * LIGHT_POWER*LIGHT_COLOR;

    return _diffuse;// + _specular;
}

void main()
{
    vec3 _ambient = WATER_COLOR * 0.1f;
    vec3 _illumination = Phong();
    FragColor = vec4(_ambient+_illumination, 1);

    //FragColor = vec4(density/5.f,density/5.f,density/5.f, 1);

//    if(density < 100) FragColor = vec4(vec3(0,0,1), 0.25);
//    else if(density == 100) FragColor = vec4(vec3(1,1,1), 0.25);
//    else if(density > 100) FragColor = vec4(vec3(1,0,0), 0.25);
}
