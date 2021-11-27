R"(
#version 330 core

out vec4 FragColor;
in vec4 clipSpaceCoordinates;
in vec2 uv;
in vec3 toCameraPos;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D waterTexture;

void main() {  
    // normalised device coordinates to properly => we get the screen space coordinates Not the (nx, ny) one but the (-1, 1) ones
    vec2 ndc = clipSpaceCoordinates.xy/clipSpaceCoordinates.w;
    // now we change the [-1, 1] coodinates to [0, 1] so that we can use it to sample the texture
    vec2 ndc_uv = ndc / 2.0 + 0.5;
    
    // we invert the coordinates to sample for the reflection
    vec2 reflectionUV = vec2(ndc_uv.x, 1.0-ndc_uv.y);
    vec2 refractionUV = vec2(ndc_uv.x, ndc_uv.y); // no need to change y

    vec4 reflectColour = texture(reflectionTexture, reflectionUV);
    vec4 refractColour = texture(refractionTexture, refractionUV);
    vec4 waterColour = texture(waterTexture, ndc_uv);

    //FragColor = vec4(uv.x, uv.y, 0.0, 1.0f);
    //FragColor = reflectColour;

    // vec3 viewVector = normalize(toCameraPos);
    // float refractiveFactor = dot(viewVector, vec3(0.0, 1.0, 0.0));
    // refractiveFactor = pow(refractiveFactor, 10);
    FragColor = mix(reflectColour, refractColour, 0.5);
    FragColor = mix(FragColor, waterColour, 0.3); // add a blue tint
}
)"