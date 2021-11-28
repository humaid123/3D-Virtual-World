R"(
#version 330 core

in vec2 position;
in vec3 vposition;
in vec2 vtexcoord;

uniform vec3 viewPos;
uniform mat4 V;
uniform mat4 P;

void main() {
	vec2 positionXY = vposition.xy + viewPos.xy;
	float h = 0.7f;
	gl_Position = P * V * vec4(positionXY, h, 1.0f);
}
)"