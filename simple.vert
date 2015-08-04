#version 330
in vec4 vertex;
in vec3 normal;
out vec3 norm;
out vec3 light_position;
out vec3 edge;
uniform vec3 light;
uniform mat4 MVP;
uniform mat4 MVN;
uniform mat4 model;
uniform mat4 view;

void main( void ) {
  vec3
  gl_Position = MVP * vertex;
  norm = (MVN * vec4(normal.xyz,0.0)).xyz;
  edge = vec3(0,0,0) - (view * model * vec4(vertex.xyz,1.0)).xyz;
  light_position = (view * vec4(light.xyz,1.0)).xyz + edge.xyz;
}

// Rotate about a given axis by a given angle
mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}
