#version 330
// Output color
out vec4 out_col;
out vec4 out_col2;

// Rotating light color
uniform vec4 ambprod_rotating_light;
uniform vec4 diffprod_rotating_light;
uniform vec4 specprod_rotating_light;
uniform float shine_rotating_light;
in vec3 light_position_rotating;  // Camera space camera position

// Static light color
uniform vec4 ambprod_static_light;
uniform vec4 diffprod_static_light;
uniform vec4 specprod_static_light;
uniform float shine_static_light;
in vec3 light_position_static;  // Camera space camera position

in vec3 norm;		// Camera space normal
in vec3 eye;		// Camera space eye to vertex

void main(void)
{
	vec3 NN = normalize(norm);
	vec3 EE = normalize(eye);

	// Rotating light
	vec3 LRN = normalize(light_position_rotating);
	vec4 amb_rotating, diff_rotating, spec_rotating;
	vec3 HR = normalize(LRN + EE);
	float kd_rotating = max(dot(LRN,NN), 0.0);
	float ks_rotating = pow(max(dot(NN,HR),0.0), shine_rotating_light);
	amb_rotating = ambprod_rotating_light;
	diff_rotating = kd_rotating * diffprod_rotating_light;
	spec_rotating = ks_rotating * specprod_rotating_light;

	// Static light
	vec3 LSN = normalize(light_position_static);
	vec4 amb_static, diff_static, spec_static;
	vec3 HS = normalize(LSN + EE);
	float kd_static = max(dot(LSN,NN), 0.0);
	float ks_static = pow(max(dot(NN,HS),0.0), shine_static_light);
	amb_static = ambprod_static_light;
	diff_static = kd_static * diffprod_static_light;
	spec_static = ks_static * specprod_static_light;

	out_col = vec4((amb_rotating + diff_rotating + spec_rotating).rgb + (amb_static + diff_static + spec_static).rgb, 1.0);
}
