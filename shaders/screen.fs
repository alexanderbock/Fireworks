#version 430 core

in vec2 UV;

out vec4 color;

uniform sampler2D prevFrame;

/*
uniform sampler2D frame;
uniform sampler2D frame2;
*/
uniform float scale;
void main()
{
	/*
	vec4 currColor = texture(frame, UV);
	vec4 prevColor = texture(frame2, UV);
	
	color =  currColor * scale + prevColor * 0.99f;
	*/
	
	color = texture(prevFrame, UV) * .9f;
}
