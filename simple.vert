#version 330
// Copyright[2015] <Brian Mc George>
// MCGBRI004

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in int sampler_index;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;
out vec3 norm;
out vec3 light_position_rotating;
out vec3 light_position_static;
out vec3 light_position_rotating_tangentspace;
out vec3 light_position_static_tangentspace;
out vec3 eye;
out vec3 eye_tangentspace;
out vec2 uv_fragment;
out vec3 tangent0;
out vec3 normal0;
out mat3 TBN;
flat out int sample_index;
uniform vec3 rotating_light;
uniform vec3 static_light;
uniform mat4 MVP;
uniform mat4 MVN;
uniform mat4 model;
uniform mat4 view;
uniform mat3 MV;

void main( void ) {
  gl_Position = MVP * vertex;
  norm = (MVN * vec4(normal.xyz, 0.0)).xyz;
  eye = (vec4(0, 0, 0, 1) - (view * model * vec4(vertex.xyz, 1.0))).xyz;
  light_position_rotating = (view * vec4(rotating_light.xyz, 1.0)).xyz + eye.xyz;
  light_position_static = (view * vec4(static_light.xyz, 1.0)).xyz + eye.xyz;
  uv_fragment = uv;
  sample_index = sampler_index;

  // Normal mapping
  //vec4 vertex_normal_v4 = MVN * (vec4(normal, 1.0));
  //vec4 vertex_tangent_v4 =  (vec4(tangent, 1.0));
  //vec4 vertex_bitangent_v4 = (vec4(bitangent, 1.0));
  vec3 vertex_normal_v4 = MV * normal;
  vec3 vertex_tangent_v4 =  MV *  tangent;
  vec3 vertex_bitangent_v4 = MV * bitangent;
  vec3 vertex_normal_cameraspace = vertex_normal_v4.xyz;
  vec3 vertex_tangent_cameraspace =  vertex_tangent_v4.xyz;
  vec3 vertex_bitangent_cameraspace = vertex_bitangent_v4.xyz;
  TBN = mat3(vertex_tangent_cameraspace, vertex_bitangent_cameraspace, vertex_normal_cameraspace);
  light_position_static_tangentspace = TBN * light_position_static;
  light_position_rotating_tangentspace = TBN * light_position_rotating;
  eye_tangentspace = TBN * eye;
  tangent0 = ((view * model) * vec4(tangent, 0.0)).xyz;
  normal0 = ((view * model) * vec4(normal, 0.0)).xyz;
}
