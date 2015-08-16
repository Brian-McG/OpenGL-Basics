#version 330
// Copyright[2015] <Brian Mc George>
// MCGBRI004

// Output color
out vec4 out_col;

// Rotating light 1
uniform vec4 ambprod_rotating_light_1;
uniform vec4 diffprod_rotating_light_1;
uniform vec4 specprod_rotating_light_1;
uniform float shine_rotating_light_1;
in vec3 light_position_rotating_1;

// rotating light 2
uniform vec4 ambprod_rotating_light_2;
uniform vec4 diffprod_rotating_light_2;
uniform vec4 specprod_rotating_light_2;
uniform float shine_rotating_light_2;
in vec3 light_position_rotating_2;

// rotating light 3
uniform vec4 ambprod_rotating_light_3;
uniform vec4 diffprod_rotating_light_3;
uniform vec4 specprod_rotating_light_3;
uniform float shine_rotating_light_3;
in vec3 light_position_rotating_3;

in vec3 norm;		// Camera space normal
in vec3 eye;		// Camera space eye to vertex

// Textures
in vec2 uv_fragment; // Texture coordinate
flat in int sample_index;
uniform sampler2D texture_sampler_f16s;
uniform sampler2D texture_sampler_f16t;

// Normal Map
uniform sampler2D normal_texture_sampler;
uniform bool normal_mapping_active;
in mat3 TBN;

void main(void)
{
  vec3 NN, EE, LRN_1, LRN_2, LRN_3;
  if (normal_mapping_active) {
    NN = texture(normal_texture_sampler, uv_fragment).xyz;
    NN = 2.0 * NN - 1;
    NN = TBN * normalize(NN);
    NN = normalize(NN);
  } else {
    NN = normalize(norm);
  }
  EE = normalize(eye);
  LRN_1 = normalize(light_position_rotating_1);
  LRN_2 = normalize(light_position_rotating_2);
  LRN_3 = normalize(light_position_rotating_3);

  // Rotating light 1
  vec4 amb_rotating_1, diff_rotating_1, spec_rotating_1;
  vec3 HR_1 = normalize(LRN_1 + EE);
  float kd_rotating_1 = max(dot(LRN_1,NN), 0.0);
  float ks_rotating_1 = pow(max(dot(NN,HR_1), 0.0), shine_rotating_light_1);
  amb_rotating_1 = ambprod_rotating_light_1;
  diff_rotating_1 = kd_rotating_1 * diffprod_rotating_light_1;
  spec_rotating_1 = ks_rotating_1 * specprod_rotating_light_1;

  // Rotating light 2
  vec4 amb_rotating_2, diff_rotating_2, spec_rotating_2;
  vec3 HR_2 = normalize(LRN_2 + EE);
  float kd_rotating_2 = max(dot(LRN_2,NN), 0.0);
  float ks_rotating_2 = pow(max(dot(NN,HR_2), 0.0), shine_rotating_light_2);
  amb_rotating_2 = ambprod_rotating_light_2;
  diff_rotating_2 = kd_rotating_2 * diffprod_rotating_light_2;
  spec_rotating_2 = ks_rotating_2 * specprod_rotating_light_2;

  // Rotating light 3
  vec4 amb_rotating_3, diff_rotating_3, spec_rotating_3;
  vec3 HR_3 = normalize(LRN_3 + EE);
  float kd_rotating_3 = max(dot(LRN_3,NN), 0.0);
  float ks_rotating_3 = pow(max(dot(NN,HR_3), 0.0), shine_rotating_light_3);
  amb_rotating_3 = ambprod_rotating_light_3;
  diff_rotating_3 = kd_rotating_3 * diffprod_rotating_light_3;
  spec_rotating_3 = ks_rotating_3 * specprod_rotating_light_3;

  // Draw correct texture
  vec3 texture;
  if (sample_index == 0 || sample_index == 1) {
    texture = (texture2D(texture_sampler_f16s, uv_fragment).rgb);
  } else if(sample_index == 2) {
    texture = (texture2D(texture_sampler_f16t, uv_fragment).rgb);
  } else {  // Error
    texture = vec3(1.0, 0, 0);
  }
  vec3 rotating_lighting_1 = (texture * amb_rotating_1.rgb) + (texture * diff_rotating_1.rgb) + (texture * spec_rotating_1.rgb);
  vec3 rotating_lighting_2 = (texture * amb_rotating_2.rgb) + (texture * diff_rotating_2.rgb) + (texture * spec_rotating_2.rgb);
  vec3 rotating_lighting_3 = (texture * amb_rotating_3.rgb) + (texture * diff_rotating_3.rgb) + (texture * spec_rotating_3.rgb);
  out_col = vec4(
                  clamp(rotating_lighting_1.r + rotating_lighting_2.r + rotating_lighting_3.r, 0, 1),
                  clamp(rotating_lighting_1.g + rotating_lighting_2.g + rotating_lighting_3.g, 0, 1),
                  clamp(rotating_lighting_1.b + rotating_lighting_2.b + rotating_lighting_3.b, 0, 1),
                  1.0
                );
}
