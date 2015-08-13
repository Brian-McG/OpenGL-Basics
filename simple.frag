#version 330
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
uniform sampler2D normal_texture_sampler;

void main(void)
{
 	  //vec3 NN = normalize(norm);
 		vec3 NN = normalize((2.0 * texture2D(normal_texture_sampler, vec2(uv_fragment.x, uv_fragment.y))).rgb - 1.0);
		//vec3 EE = normalize(eye);
		vec3 EE = normalize(eye_tangentspace);

		// Rotating light
		vec3 LRN = normalize(light_position_rotating);
		//vec3 LRN = normalize(light_position_rotating_tangentspace);
		vec4 amb_rotating, diff_rotating, spec_rotating;
		vec3 HR = normalize(LRN + EE);
		float kd_rotating = max(dot(LRN,NN), 0.0);
		float ks_rotating = pow(max(dot(NN,HR),0.0), shine_rotating_light);
		amb_rotating = ambprod_rotating_light;
		diff_rotating = kd_rotating * diffprod_rotating_light;
		spec_rotating = ks_rotating * specprod_rotating_light;

		// Static light
		//vec3 LSN = normalize(light_position_static);
		vec3 LSN = normalize(light_position_static_tangentspace);
		vec4 amb_static, diff_static, spec_static;
		vec3 HS = normalize(LSN + EE);

		float kd_static = max(dot(LSN,NN), 0.0);
		float ks_static = pow(max(dot(NN,HS),0.0), shine_static_light);
		amb_static = ambprod_static_light;
		diff_static = kd_static * diffprod_static_light;
		spec_static = ks_static * specprod_static_light;

		// Draw correct texture
		if (sample_index == 1) {
			vec3 texture = (texture2D(texture_sampler_f16s, uv_fragment).rgb);
			vec3 static_lighting = amb_static.rgb + (texture * diff_static.rgb) + (texture * spec_static.rgb);
		//	vec3 rotating_lighting = amb_rotating.rgb + (specprod_rotating_light.rgb * ks_rotating) + (diffprod_rotating_light.rgb * kd_rotating);
		//	if(ks_static < 0.001) {
		//		out_col = vec4(1.0, 0.0, 0.0, 1.0);
		//	} else {
				out_col = vec4(static_lighting /*+ rotating_lighting*/, 1.0);
		//	}
		} else if(sample_index == 2) {
		  vec3 texture = (texture2D(texture_sampler_f16t, uv_fragment).rgb);
			vec3 static_lighting = amb_static.rgb + /* (texture * diff_static.rgb) +*/ (texture * spec_static.rgb);
		//	vec3 rotating_lighting = amb_rotating.rgb + (specprod_rotating_light.rgb * ks_rotating) + (diffprod_rotating_light.rgb * kd_rotating);
			out_col = vec4(static_lighting /*+ rotating_lighting*/, 1.0);
		}
}
