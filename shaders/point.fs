#version 430 core

in vec3 gCol;
in vec2 gTex;

uniform sampler2D Tex;

uniform float scale;

//out vec4 color;

out vec4 color;
out vec3 normal;

void main(){
	
	float alpha = texture(Tex, gTex).a ;
	color = vec4(gCol, alpha);
	if(color.a < 0.01)
	discard;

}