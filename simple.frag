#version 330
out vec4 out_col;
uniform vec4 fcolor;
uniform vec4 light;
uniform vec4 ambprod;
uniform vec4 diffprod;
uniform vec4 specprod;
uniform float shine;
in vec3 N;
in vec3 L;
in vec3 E;

void main( void )
{
	vec3 NN = normalize(N);
	vec3 EE = normalize(E);
	vec3 LL = normalize(L);
	vec4 amb, diff, spec;
	vec3 H = normalize(LL + EE);
	float kd = max(dot(L,N), 0.0);
	float ks = pow(max(dot(N,H),0.0), shine);
	amb = ambprod;
	diff = kd * diffprod;
	spec = ks * specprod;
	out_col = vec4((amb + diff + spec).xyz, 1.0); //gl_FragColor is supposed to be depricated
}
