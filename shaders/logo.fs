#version 430 core

in vec2 UV;

out vec4 color;

uniform sampler2D Tex;

void main() {
    vec4 c = texture(Tex, UV);
    if (length(c.rgb) > 0.01) 
	    color = vec4(texture(Tex, UV).rgb, 1.0);
    else
        color = vec4(0.0);
}
