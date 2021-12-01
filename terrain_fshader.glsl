R"(
#version 330 core

uniform sampler2D grass;
uniform sampler2D rock;
uniform sampler2D sand;
uniform sampler2D snow;

uniform vec3 viewPos;
uniform float waterHeight;
uniform vec3 skyColor;
uniform vec3 lightPos;

in vec2 uv;
in vec3 fragPos;
in float height;
in vec3 normal;
in float slope; 
in vec3 distanceFromCamera;

out vec4 color;

vec4 mixWithGrassAndSnow(sampler2D base, vec2 uv, float height, float slope) {
    float mixWithGrassLevel = 1.5f;
    float mixWithSnowLevel = mixWithGrassLevel + 2.0f;

    vec4 with_grass = mix(texture(base, uv), texture(grass, uv), slope);
    vec4 with_snow = mix(texture(base, uv), texture(snow, uv), slope);
    if (height < mixWithGrassLevel) {
        vec4 res = mix(with_grass, with_snow, (mixWithGrassLevel - height) / (mixWithGrassLevel));
        res = vec4(res.xyz, 1.0f); // grass has low specular power
        return res;
    } else if (abs(height - mixWithGrassLevel) < 0.1f) {
        vec4 res = mix(with_grass, with_snow, 0.5f);
        return res;
    } else { 
        vec4 res = mix(with_grass, with_snow, (mixWithSnowLevel - height) / (mixWithSnowLevel));
        return res;
    }
}


void main() {
    /*
    what I want is
        sand, rock, snow/rock mixture

        however we define a level 
            water, grass, snow
        such that at each of these level, we mix with these colours based on the slope

        this way we get as if snow/grass was depsited on the surface
    */

    float pureSandLevel = waterHeight + 0.01f; // beach
    float sandLevel = pureSandLevel + 0.2f; //sand with some grass ot snow
    float grassLevel = sandLevel + 0.15f;
    float rockLevel = grassLevel + 1.8f; // rock with some grass or snow
    float snowLevel = rockLevel + 1.0f;

    // go from top to bottom so that the code is simple if-statements after if-statements
    // the top level is a mixture of snow and rock
    vec4 col = mix(texture(snow, uv), texture(rock, uv), slope); // color at the top is the default, we change if height is too low
    vec4 col_below = texture(rock, uv); // used to mix with the level below it
    col = mix(col, col_below, (snowLevel - height) / (snowLevel));

    // mixture of rock and grass => also add snow/more grass based on the slope
    if (height < rockLevel) {
        // we add some changes such as slope/10.0f and mix -0.1f so as to prefer the rock texture over the other textures
        col = mixWithGrassAndSnow(rock, uv, height, slope/10.0f);
        col_below = mixWithGrassAndSnow(grass, uv, height, slope);
        col = mix(col, col_below, (rockLevel - height) / (rockLevel) -0.1f);
    }

    if (height < grassLevel) {
        col = mixWithGrassAndSnow(grass, uv, height, slope);
        col_below = mixWithGrassAndSnow(sand, uv, height, slope);
        col = mix(col, col_below, (grassLevel - height) / (grassLevel));
    }

    if (height < sandLevel) {
        col = mixWithGrassAndSnow(sand, uv, height, slope);
        col_below = texture(sand, uv);
        col = mix(col, col_below, (sandLevel - height) / (sandLevel));
    }

    // we have a beach level near the water
    if (height < pureSandLevel) {
        col = texture(sand, uv);
    }

    // Blinn-Phong
    float ka = 0.05f;
    float kd = 1.2f;
    float ks = 0.7f;
    float p = 0.8f;

    // diffuse lighting
    vec3 lightDir = normalize(lightPos - fragPos);
    float diffuse = kd * max(0.0f, -dot(normal, lightDir));

    // specular lighting
    vec3 viewDirection = viewPos - fragPos;
    vec3 halfway = normalize(lightDir + viewDirection);
    float specular = ks * max(0.0f, pow(dot(normal, halfway), p));

    col = ka*vec4(skyColor, 1.0f) + diffuse*col + specular*col;
  
    // we now mix the color with the skycolor based on the distance from the camera
    // we use the visibility formula to detect how far an object is from the camera
    // 1 => render normaly, 0 => fade into the skycolor
    // visibility takes the distance from the camera and use an exponential decrease so that the fog scaling looks more natural
    // visibility = exp(-pow(distance * density, gradient)) is such a formula
    // density = thickness of the fog, higher value means less visiblr
    // gradient = how quickly the visibility decreases with distance
    float density = 0.1;
    float gradient = 1.5;
    float distance = length(distanceFromCamera.xyz);
    float visibility = exp(-pow(distance * density, gradient));
    visibility = clamp(visibility, 0.0, 1.0);
    color = mix(vec4(skyColor, 1.0f),  col, visibility);
}
)"