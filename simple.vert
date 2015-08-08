#version 330
in vec4 vertex;
in vec3 normal;
out vec3 norm;
out vec3 light_position_rotating;
out vec3 light_position_static;
out vec3 eye;
uniform vec3 rotating_light;
uniform vec3 static_light;
uniform mat4 MVP;
uniform mat4 MVN;
uniform mat4 model;
uniform mat4 view;


void main( void ) {
  gl_Position = MVP * vertex;
  norm = (MVN * vec4(normal.xyz, 0.0)).xyz;
  eye = (vec4(0, 0, 0, 1) - (view * model * vec4(vertex.xyz, 1.0))).xyz;
  light_position_rotating = (view * vec4(rotating_light.xyz, 1.0)).xyz + eye.xyz;
  light_position_static = (view * vec4(static_light.xyz, 1.0)).xyz + eye.xyz;
}
