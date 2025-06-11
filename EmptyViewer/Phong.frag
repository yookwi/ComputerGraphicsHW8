#version 330 core
out vec4 FragColor;

in vec3 wPosition;
in vec3 wNormal;
in vec3 wColor;

uniform vec3 cameraPos;
uniform vec4 lightPos;
uniform vec3 la;
uniform float p;

uniform vec3 ac;
uniform vec3 dc;
uniform vec3 sc;

vec4 Shading(vec3 wPosition, vec3 wNormal, vec3 wColor);

void main() {
	FragColor = Shading(wPosition, wNormal, wColor);
}

vec4 Shading(vec3 wPosition, vec3 wNormal, vec3 wColor){
	vec3 Normal = normalize(wNormal);

	vec3 ka = vec3(1, 1, 1);
	vec3 kd = vec3(1, 1, 1);
	vec3 ks = vec3(0, 0, 0);
	float r = 2.2;
	
	vec3 lay;
	if(lightPos.w == 0){
		lay = -lightPos.xyz;
	}
	else{
		lay = vec3(lightPos) - wPosition;
	}
	lay = normalize(lay);
	vec3 view = cameraPos - wPosition;
	view = normalize(view);
	vec3 h = view + lay;
	h = normalize(h);
	vec3 light = ka * la * ac + kd * dc * max(0.0, dot(Normal, lay)) + ks * sc * pow(max(0.0, dot(Normal, h)), p);
	light = pow(light, vec3(1.0 / r));
	return vec4(light, 1.0f);
}