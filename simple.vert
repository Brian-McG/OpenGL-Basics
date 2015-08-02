#version 330
in vec4 vertex;
in vec3 normal;
out vec3 norm;
out vec3 light_position;
out vec3 edge;
uniform vec3 light;
uniform mat4 MVP;
uniform mat4 model;

void main( void ) {
    if(light[0] == 0 && light[1] == 0 && light[2] == 0)
    {

    }
    else
    {
      gl_Position = MVP * vertex;
      norm = (model * vec4(normal.xyz,1.0)).xyz;
      edge = -(model * vertex).xyz;
      light_position = light.xyz + edge.xyz;

    }

}
