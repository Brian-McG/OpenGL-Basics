#version 330
out vec4 out_col;
uniform vec4 fcolor;
uniform vec4 ambprod;
uniform vec4 diffprod;
uniform vec4 specprod;
uniform float shine;
in vec3 norm;
in vec3 light_position;
in vec3 edge;

void main(void)
{
	vec3 NN = normalize(norm);
	vec3 EE = normalize(edge);
	vec3 LL = normalize(light_position);
	vec4 amb, diff, spec;
	vec3 H = normalize(LL + EE);
	float kd = max(dot(LL,NN), 0.0);
	float ks = pow(max(dot(NN,H),0.0), shine);
	amb = ambprod;
	diff = kd * diffprod;
	spec = ks * specprod;
	out_col = vec4((amb + diff + spec).rgb, 1.0);
}
