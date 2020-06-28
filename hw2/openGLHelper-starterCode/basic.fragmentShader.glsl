#version 150

in vec3 viewPosition;
in vec3 viewNormal;

out vec4 c;

uniform vec4 La; // light ambient
uniform vec4 Ld; // light diffuse
uniform vec4 Ls; // light specular
uniform vec3 viewLightDirection;

uniform vec4 ka; // light ambient
uniform vec4 kd; // light diffuse
uniform vec4 ks; // light specular
uniform float alpha;

void main()
{
  // camera is at (0,0,0) after the modelview transformation
  vec3 eyedir = normalize(vec3(0,0,0)-viewPosition);
  // reflected light direction
  vec3 reflectDir = -reflect(viewLightDirection,viewNormal);
  //Phong lighting
  float d = max(dot(viewLightDirection,viewNormal),0.0f);
  float s = max(dot(reflectDir,eyedir),0.0f);
  //compute the final color
  c = ka*La + kd*Ld*d + ks*Ls*pow(s,alpha);
}

