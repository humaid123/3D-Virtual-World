R"(
#version 330 core
in vec3 vposition;

out vec3 texCoords;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 viewPos;

void main()
{
    vec4 pos = projection * view * vec4(100*(vposition), 1.0f);
    // Having z equal w will always result in a depth of 1.0f
    gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
    // We want to flip the z axis due to the different coordinate systems (left hand vs right hand)
    //texCoords = vec3(vposition.x, vposition.y, vposition.z);
    texCoords =  vec3(vposition.x, -vposition.z, -vposition.y);
}   
)"