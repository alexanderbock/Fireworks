#version 430 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoords;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec2 UV;

void main() {
   gl_Position = P * V * M * vec4(Position, 1.0);
   UV = TexCoords;
}