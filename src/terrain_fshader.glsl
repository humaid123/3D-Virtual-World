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

// prevent lines between two levels by blending several levels together
vec4 mixBetweenLevels(vec4 col_middle, float height, vec4 col_lower, float height_lower, vec4 col_higher, float height_higher) {
    float halfDistance = (height_higher - height_lower) / 2.0f;
    if (height < (height_lower + halfDistance)) {
        float pos = height - height_lower;
        float posScaled = pos/halfDistance;
        return col_middle * posScaled + col_lower * (1 - posScaled);
    } else {
        float pos = height_higher - height;
        float posScaled = pos/halfDistance;
        return col_middle * posScaled + col_higher * (1 - posScaled);
    }
}

// mix with grass or snow based on level so that grass/snow appears deposited on the surface
vec4 mixWithGrassAndSnow(sampler2D base, vec2 uv, float height, float slope) {
    float mixWithGrassLevel = 1.5f;
    float mixWithSnowLevel = mixWithGrassLevel + 2.0f;

    vec4 with_grass = mix(texture(base, uv), texture(grass, uv), slope);
    vec4 with_snow = mix(texture(base, uv), texture(snow, uv), slope);
    vec4 col = texture(base, uv);

    // we have to mix between levels here again as otherwise we get a line here also...
    return mixBetweenLevels(col, height, with_grass, mixWithGrassLevel,  with_snow, mixWithSnowLevel);
}


void main() {

    /*
    the shading is as such:
        we have a pure snow level, followed by a rock, grass, sand and pure sand level
        To make the rock, grass, sand levels in the middle 'impure', I mix them with the snow and the 
        grass texture based on a seperate heights - see mixWithGrassAndSnowFunction => this looks like deposits 
        on the terrain.
    */

    float pureSandLevel = waterHeight + 0.01f; // beach around the water => pure sand
    float sandLevel = pureSandLevel + 0.15f;   // sand with some grass or snow
    float grassLevel = sandLevel + 0.25f;      // grass with some grass or snow (preferably snow for some 'snowy grass')
    float rockLevel = grassLevel + 1.6f;       // rock with some grass or snow
    float snowLevel = rockLevel + 0.8f;        // pure snow

    vec4 col;

    vec4 pureSnowCol = texture(snow, uv);
    vec4 rockCol = mixWithGrassAndSnow(rock, uv, height, slope/10.0f);
    vec4 grassCol = mixWithGrassAndSnow(grass, uv, height, slope);
    vec4 sandCol = mixWithGrassAndSnow(sand, uv, height, slope);
    vec4 pureSandCol = texture(sand, uv);

    // Blinn-Phong constants are placed here so we can update them at each level
    float ka = 0.05f, kd = 1.2f, ks = 0.7f, p = 0.8f;
    
    if (height > snowLevel) {
        col = pureSnowCol;
        kd = 1.0f;
    } else if (height >= grassLevel && height <= snowLevel) {
        // rock but blend with grass below and snow above
        col = mixBetweenLevels(rockCol, height, grassCol, grassLevel, pureSnowCol, snowLevel);
    } else if (height >= sandLevel && height <= rockLevel) {
        // texture with grass but blend with rock above and blend with sand below
        col = mixBetweenLevels(grassCol, height, sandCol, sandLevel, rockCol, rockLevel);
    } else if (height >= pureSandLevel && height <= grassLevel) {
        // texture with sand but blend with grass above and blend with pureSand below
        col = mixBetweenLevels(sandCol, height, pureSandCol, pureSandLevel, grassCol, grassLevel);
    } else { 
        // height <= pureSandLevel
        col = pureSandCol;
    }

    // Blinn-Phong calculation
    vec3 lightDir = normalize(lightPos - fragPos);
    float diffuse = kd * max(0.0f, -dot(normal, lightDir));

    vec3 viewDirection = viewPos - fragPos;
    vec3 halfway = normalize(lightDir + viewDirection);
    float specular = ks * max(0.0f, pow(dot(normal, halfway), p));

    // I use the skyColor as the ambient color to make the fog look better.
    col = ka*vec4(skyColor, 1.0f) + diffuse*col + specular*col;

    // visibility calculation => add fog so that we can hide 'render distance'  
    // we mix the color with the skycolor based on the distance from the camera
    // we use a visibility formula to detect how far an object is from the camera
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