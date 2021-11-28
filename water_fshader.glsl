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

vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v){
    const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);
    vec2 i1;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod(i, 289.0);
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
    + i.x + vec3(0.0, i1.x, 1.0 ));
    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}


void main() {  

    // we get the water color with a displacement
    vec2 waterUV1 = uv + vec2(0.1 * (1 + sin(time/5)), 0.2 * (1 + sin(time/10)));
    vec2 waterUV2 = uv + vec2(0.05 * (1 + sin(time/2)), 0.1 * (1 + sin(time/5)));
    vec2 waterUV3 = uv + vec2(0.03 * (1 + sin(time)), 0.05 * (1 + sin(time/2)));
    vec4 waterColour = 0.5 * texture(waterTexture, waterUV1) + 0.3 * texture(waterTexture, waterUV2) + 0.2 * texture(waterTexture, waterUV3);

    // normalised device coordinates to properly => we get the screen space coordinates Not the (nx, ny) one but the (-1, 1) ones
    vec2 ndc = clipSpaceCoordinates.xy/clipSpaceCoordinates.w;
    // now we change the [-1, 1] coodinates to [0, 1] so that we can use it to sample the texture
    vec2 ndc_uv = ndc / 2.0 + 0.5;
    
    // we invert the coordinates to sample for the reflection
    vec2 reflectionUV = vec2(ndc_uv.x, 1.0-ndc_uv.y);    
    vec2 refractionUV = vec2(ndc_uv.x, ndc_uv.y); // no need to change y

    vec4 reflectColour = texture(reflectionTexture, vec2(
        reflectionUV.x + 0.01*snoise(vec2(reflectionUV.x, time/2)), 
        reflectionUV.y + 0.02*snoise(vec2(time/5, reflectionUV.y))
    ));
    
    vec4 refractColour = texture(refractionTexture, refractionUV);

    FragColor = mix(reflectColour, refractColour, 0.3); // more reflection, the smaller the number
    FragColor = mix(FragColor, waterColour, 0.5); // add displacement based on the layered periodic coordinates
    FragColor = mix(FragColor, vec4(0.0, 0.1, 0.2, 1.0), 0.2); // add a bluish color
}
)"