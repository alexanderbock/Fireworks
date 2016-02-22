#version 410 core

const vec2 corners[4] = vec2[4](
	vec2(0.0, 1.0),
	vec2(0.0, 0.0),
	vec2(1.0, 1.0),
	vec2(1.0, 0.0)
);

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

layout(location = 0) in vec3[] pos;
layout(location = 1) in vec3[] vCol;

out vec3 gCol;
out vec2 gTex;

uniform mat4 P;

uniform float spriteSize;

void main(){

	float mag = 1.0;// vCol[0].a;
	vec4 newPos[4];
	for(int i = 0; i < 4; i++){
		vec4 p = gl_in[0].gl_Position;
		p.xy += mag * spriteSize * (corners[i] - vec2(0.5));
		newPos[i] = P * p;
	}

	
	for(int i = 0; i < 4; i++){
		gl_Position =  newPos[i];
		gCol = vCol[0];
		gTex = corners[i];
		EmitVertex();
	}
	
	EndPrimitive();
}