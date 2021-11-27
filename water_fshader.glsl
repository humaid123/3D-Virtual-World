R"(
#version 330 core

out vec4 FragColor;
in vec4 clipSpaceCoordinates;
in vec2 uv;
in vec3 toCameraPos;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D waterTexture;
uniform float time;

// simple Box blur for reflection distortion
// https://www.shadertoy.com/view/llGSz3
vec4 blur(sampler2D sampler, vec2 uv) {
   const float kernel = 10.0;
   const float weight = 1.0;

   vec2 resolution = vec2(1280, 720); // resolution of image
   vec3 sum = vec3(0);
   float pixelSize = 1.0 / resolution.x;

   vec3 accumulation = vec3(0);
   vec3 weightSum = vec3(0);
   for (float i = -kernel; i<=kernel; i++) {
        accumulation += texture(sampler, uv + vec2(i * pixelSize, 0.0)).xyz * weight;
        weightSum += weight;
   }
   sum = accumulation / weightSum;
   return vec4(sum, 1.0);
}

void main() {  

    // we get the water color with a displacement
    //vec2 waterUV = texture(waterTexture, vec2(uv.x + (1 + sin(time/10)), uv.y)).rg * 0.01f;
    //waterUV = uv + vec2(waterUV.x, waterUV.y + (1 + sin(time/10)));
    vec2 waterUV = uv + vec2(0.1 * (1 + sin(time/5)), 0.2 * (1 + sin(time/10)));
    //waterUV = clamp(waterUV, 0.001, 0.999);
    vec4 waterColour = texture(waterTexture, waterUV);

    // normalised device coordinates to properly => we get the screen space coordinates Not the (nx, ny) one but the (-1, 1) ones
    vec2 ndc = clipSpaceCoordinates.xy/clipSpaceCoordinates.w;
    // now we change the [-1, 1] coodinates to [0, 1] so that we can use it to sample the texture
    vec2 ndc_uv = ndc / 2.0 + 0.5;
    
    // we invert the coordinates to sample for the reflection
    vec2 reflectionUV = vec2(ndc_uv.x, 1.0-ndc_uv.y);
    //reflectionUV += 0.1 * vec2(sin((1-ndc_uv.y)*time/10), cos((ndc_uv.x)*time/10));
    //reflectionUV = clamp(reflectionUV, 0.001, 0.999);
    
    vec2 refractionUV = vec2(ndc_uv.x, ndc_uv.y); // no need to change y

    // vec4 reflectColour = texture(reflectionTexture, reflectionUV);
    vec4 reflectColour = blur(reflectionTexture, reflectionUV);
    
    // vec4 refractColour = texture(refractionTexture, refractionUV);
    vec4 refractColour = blur(refractionTexture, refractionUV);

    FragColor = mix(reflectColour, refractColour, 0.3); // more reflection, the smaller the number
    FragColor = mix(FragColor, waterColour, 0.4); // add a blue tint
}
)"