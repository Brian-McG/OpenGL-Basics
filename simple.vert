#version 330
in vec4 vertex;
in vec3 normal;
out vec3 norm;
out vec3 light_position;
out vec3 edge;
uniform vec3 light;
uniform mat4 MVP;
uniform mat4 MVN;
uniform mat4 model;
uniform mat4 view;


void main( void ) {
  gl_Position = MVP * vertex;
  norm = (MVN * vec4(normal.xyz,0.0)).xyz;
  edge = (vec4(0,0,0,1) - (view * model * vec4(vertex.xyz,1.0))).xyz;
  light_position = (view * vec4(light.xyz,1.0)).xyz + edge.xyz;
}
