#version 430 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;

out vec3 vCol;

uniform mat4 M;
uniform mat4 V;

void main(){
	gl_Position = V * M * vec4(pos, 1);	
	vCol = col;
}