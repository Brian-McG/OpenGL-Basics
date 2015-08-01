#version 330
in vec4 vertex;
uniform vec4 normal;
uniform vec4 light;
out vec3 norm;
out vec3 light_position;
out vec3 edge;
uniform mat4 MVP;

void main( void ) {
  gl_Position = MVP * vertex;
  norm = norm.xyz;
  light_position = light.xyz - vertex.xyz;
  edge = -vertex.xyz;
}
