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

float Perlin2D( vec2 P ) {
    //  https://github.com/BrianSharpe/Wombat/blob/master/Perlin2D.glsl

    // establish our grid cell and unit position
    vec2 Pi = floor(P);
    vec4 Pf_Pfmin1 = P.xyxy - vec4( Pi, Pi + 1.0 );

    // calculate the hash
    vec4 Pt = vec4( Pi.xy, Pi.xy + 1.0 );
    Pt = Pt - floor(Pt * ( 1.0 / 71.0 )) * 71.0;
    Pt += vec2( 26.0, 161.0 ).xyxy;
    Pt *= Pt;
    Pt = Pt.xzxz * Pt.yyww;
    vec4 hash_x = fract( Pt * ( 1.0 / 951.135664 ) );
    vec4 hash_y = fract( Pt * ( 1.0 / 642.949883 ) );

    // calculate the gradient results
    vec4 grad_x = hash_x - 0.49999;
    vec4 grad_y = hash_y - 0.49999;
    vec4 grad_results = inversesqrt( grad_x * grad_x + grad_y * grad_y ) * ( grad_x * Pf_Pfmin1.xzxz + grad_y * Pf_Pfmin1.yyww );

    // Classic Perlin Interpolation
    grad_results *= 1.4142135623730950488016887242097;  // scale things to a strict -1.0->1.0 range  *= 1.0/sqrt(0.5)
    vec2 blend = Pf_Pfmin1.xy * Pf_Pfmin1.xy * Pf_Pfmin1.xy * (Pf_Pfmin1.xy * (Pf_Pfmin1.xy * 6.0 - 15.0) + 10.0);
    vec4 blend2 = vec4( blend, vec2( 1.0 - blend ) );
    return dot( grad_results, blend2.zxzx * blend2.wwyy );
}


void main() {  

    // we get the water color with a displacement
    // by adding several displaced version of the water texture, it appears as if it is moving
    vec2 waterUV1 = uv + vec2(0.2 * (1 + sin(time/5)), 0.2 * (1 + sin(time/10)));
    vec2 waterUV2 = uv + vec2(0.1 * (1 + sin(time/2)), 0.1 * (1 + sin(time/5)));
    vec2 waterUV3 = uv + vec2(0.05 * (1 + sin(time)), 0.05 * (1 + sin(time/2)));
    vec4 waterColor = 0.5 * texture(waterTexture, waterUV1) + 0.3 * texture(waterTexture, waterUV2) + 0.2 * texture(waterTexture, waterUV3);

    // normalised device coordinates to properly => we get the screen space coordinates Not the (nx, ny) one but the (-1, 1) ones
    vec2 ndc = clipSpaceCoordinates.xy/clipSpaceCoordinates.w;
    // now we change the [-1, 1] coodinates to [0, 1] so that we can use it to sample the texture
    vec2 ndc_uv = ndc / 2.0 + 0.5;
    
    // we invert the coordinates to sample for the reflection
    vec2 reflectionUV = vec2(ndc_uv.x, 1.0-ndc_uv.y);   // need to change y as inverted (opengl coordinate system problems)
    vec2 refractionUV = vec2(ndc_uv.x, ndc_uv.y); // no need to change y

    // displace the reflection based on time and a simplex noise
    // vec4 reflectColor = texture(reflectionTexture, vec2(
    //     reflectionUV.x + 0.1*snoise(vec2(reflectionUV.x, time/3)), 
    //     reflectionUV.y + 0.1*snoise(vec2(time/3, reflectionUV.y))
    // ));

    vec4 reflectColor = texture(reflectionTexture, vec2(
        reflectionUV.x + 0.01*Perlin2D(vec2(reflectionUV.x, time/2)), 
        reflectionUV.y + 0.01*Perlin2D(vec2(time/2, reflectionUV.y))
    ));
    
    vec4 refractColor = texture(refractionTexture, refractionUV);

    FragColor = mix(reflectColor, refractColor, 0.3); // more reflection, the smaller the number
    FragColor = mix(FragColor, waterColor, 0.2); // add displacement based on the layered periodic coordinates
    FragColor = mix(FragColor, vec4(0.0, 0.1, 0.2, 1.0), 0.2); // add a bluish color
}
)"