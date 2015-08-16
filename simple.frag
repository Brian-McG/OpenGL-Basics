#version 330
// Copyright[2015] <Brian Mc George>
// MCGBRI004

// Output color
out vec4 out_col;

// Rotating light color
uniform vec4 ambprod_rotating_light;
uniform vec4 diffprod_rotating_light;
uniform vec4 specprod_rotating_light;
uniform float shine_rotating_light;
in vec3 light_position_rotating;  // Camera space camera position
in vec3 light_position_rotating_tangentspace;

// Static light color
uniform vec4 ambprod_static_light;
uniform vec4 diffprod_static_light;
uniform vec4 specprod_static_light;
uniform float shine_static_light;
in vec3 light_position_static;  // Camera space camera position
in vec3 light_position_static_tangentspace;
in vec3 norm;		// Camera space normal
in vec3 eye;		// Camera space eye to vertex
in vec3 eye_tangentspace;

// Textures
in vec2 uv_fragment; // Texture coordinate
flat in int sample_index;
uniform sampler2D texture_sampler_f16s;
uniform sampler2D texture_sampler_f16t;

// Normal Map
uniform sampler2D normal_texture_sampler;
uniform bool normal_mapping_active;
in vec3 tangent0;
in vec3 normal0;
in mat3 TBN;

mat3 calculateBumpedNormal() {
  vec3 normal = normalize(normal0);
  vec3 tangent = normalize(tangent0);
  tangent = normalize(tangent - dot(tangent, normal) * normal);
  vec3 bitangent = cross(tangent, normal);
  vec3 bumpMapNormal = texture(normal_texture_sampler, uv_fragment).xyz;
  bumpMapNormal = 2.0 * bumpMapNormal - vec3(1.0, 1.0, 1.0);
  vec3 newNormal;
  mat3 TBN = mat3(tangent, bitangent, normal);
  return TBN;
}

void main(void)
{
  vec3 NN, EE, LRN, LSN;
  float cosTheta;
  vec3 r;
  float cosAlpha;
  if(normal_mapping_active) {
    NN = texture(normal_texture_sampler, uv_fragment).xyz;
    NN = 2.0 * NN - 1;
    NN = TBN * normalize(NN);
    NN = normalize(NN);
    EE = normalize(eye);
    LRN = normalize(light_position_rotating);
    LSN = normalize(light_position_static);
  } else {
    NN = normalize(norm);
    EE = normalize(eye);
    LRN = normalize(light_position_rotating);
    LSN = normalize(light_position_static);
  }

  vec4 amb_rotating, diff_rotating, spec_rotating;
  vec3 HR = normalize(LRN + EE);

  vec4 amb_static, diff_static, spec_static;
  vec3 HS = normalize(LSN + EE);

  if(normal_mapping_active) {
    // Rotating light

    float kd_rotating = max(dot(LRN,NN), 0.0);
    float ks_rotating = pow(max(dot(NN,HR),0.0), shine_rotating_light);
    amb_rotating = ambprod_rotating_light;
    diff_rotating = kd_rotating * diffprod_rotating_light;
    spec_rotating = ks_rotating * specprod_rotating_light;

    // Static light

    float kd_static = max(dot(LSN,NN), 0.0);
    float ks_static = pow(max(dot(NN,HS),0.0), shine_static_light);
    amb_static = ambprod_static_light;
    diff_static = kd_static * diffprod_static_light;
    spec_static = ks_static * specprod_static_light;
  } else {
    // Rotating light

    float kd_rotating = max(dot(LRN,NN), 0.0);
    float ks_rotating = pow(max(dot(NN,HR),0.0), shine_rotating_light);
    amb_rotating = ambprod_rotating_light;
    diff_rotating = kd_rotating * diffprod_rotating_light;
    spec_rotating = ks_rotating * specprod_rotating_light;

    // Static light

    float kd_static = max(dot(LSN,NN), 0.0);
    float ks_static = pow(max(dot(NN,HS),0.0), shine_static_light);
    amb_static = ambprod_static_light;
    diff_static = kd_static * diffprod_static_light;
    spec_static = ks_static * specprod_static_light;
  }

  // Draw correct texture
  vec3 texture;
  if (sample_index == 0 || sample_index == 1) {
    texture = (texture2D(texture_sampler_f16s, uv_fragment).rgb);
  } else if(sample_index == 2) {
    texture = (texture2D(texture_sampler_f16t, uv_fragment).rgb);
  } else {  // Error
    texture = vec3(1.0, 0, 0);
  }
  vec3 rotating_lighting = (texture * amb_rotating.rgb) + (texture * diff_rotating.rgb) + (texture * spec_rotating.rgb);
  vec3 static_lighting = (texture * amb_static.rgb) + (texture * diff_static.rgb) + (texture * spec_static.rgb);
  out_col = vec4(
                  clamp(static_lighting.r + rotating_lighting.r, 0, 1),
                  clamp(static_lighting.g + rotating_lighting.g, 0, 1),
                  clamp(static_lighting.b + rotating_lighting.b, 0, 1),
                  1.0
                );
}
