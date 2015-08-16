#version 330
// Copyright[2015] <Brian Mc George>
// MCGBRI004
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in int sampler_index;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;
uniform vec3 rotating_light_1;
uniform vec3 rotating_light_2;
uniform mat4 MVP;
uniform mat3 MV;
uniform mat4 MVN;
uniform mat4 model;
uniform mat4 view;
out vec3 norm;
out vec3 light_position_rotating_1;
out vec3 light_position_rotating_2;
out vec3 eye;
out vec2 uv_fragment;
out mat3 TBN;
flat out int sample_index;

void main( void ) {
  gl_Position = MVP * vertex;
  norm = (MVN * vec4(normal.xyz, 0.0)).xyz;
  eye = (vec4(0, 0, 0, 1) - (view * model * vec4(vertex.xyz, 1.0))).xyz;
  light_position_rotating_1 = (view * vec4(rotating_light_1.xyz, 1.0)).xyz + eye.xyz;
  light_position_rotating_2 = (view * vec4(rotating_light_2.xyz, 1.0)).xyz + eye.xyz;
  uv_fragment = uv;
  sample_index = sampler_index;

  // Normal mapping
  vec3 vertex_normal_cameraspace = MV * normal;
  vec3 vertex_tangent_cameraspace =  MV *  tangent;
  vec3 vertex_bitangent_cameraspace = MV * bitangent;
  TBN = mat3(vertex_tangent_cameraspace, vertex_bitangent_cameraspace, vertex_normal_cameraspace);
}
