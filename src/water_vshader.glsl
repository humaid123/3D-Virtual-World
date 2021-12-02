R"(
#version 330 core
//uniform sampler2D noiseTex;

in vec3 vposition;
in vec2 vtexcoord;
out vec4 clipSpaceCoordinates;
out vec2 uv;
out vec3 toCameraPos;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 viewPos; // camera position, we translate the grid using it...


void main() {
    // Set gl_Position
    vec4 position = P*V*M*vec4(vposition + (vec3(viewPos.x, viewPos.y, 0)), 1.0f);

    clipSpaceCoordinates = position;
    gl_Position = position;
    uv = vtexcoord;
    toCameraPos = viewPos - position.xyz;
}
)"