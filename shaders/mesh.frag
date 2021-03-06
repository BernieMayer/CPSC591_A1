#version 430 core

out vec4 color;

in vec3 N;
in vec3 L;
in vec3 V;

uniform float b;
uniform float y;

uniform float alpha;
uniform float beta;

uniform vec3 kd;


void main(void)
{
    /*
    vec3 ambient = vec3(0.1745, 0.01175, 0.01175) * L;
    vec3 white = vec3(0.7278, 0.6270, 0.627);

    vec3 R = normalize(reflect(-L, N));

    float e = 76.8;
    float specular = pow( max( 0.0, dot(R, V)), e);

    vec3 gold = vec3(0.614, 0.0414, 0.0414);
    */

    vec3 k_blue = vec3(0, 0, b);
    vec3 k_yellow = vec3(y, y, 0);

    vec3 k_cool = k_blue + alpha * kd;

    vec3 k_warm = k_yellow + beta * kd;



    float lightFactor = (1 - dot(L, N))/2;

    vec3 I = lightFactor * k_cool + (1 - lightFactor) * k_warm;
    vec3 R = normalize(reflect(-L, N));
    float e = 76.8;
    float specular = pow( max( 0.0, dot(R, V)), e);
    vec3 white = vec3(1.0, 1.0, 1.0);

    color = vec4( I, 1.0);


    //color = vec4( ambient + diffuse*gold + specular * white, 1.0);
}
