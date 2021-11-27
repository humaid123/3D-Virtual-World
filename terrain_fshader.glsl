R"(
#version 330 core

// Uniforms
//uniform sampler2D noiseTex;
uniform sampler2D grass;
uniform sampler2D rock;
uniform sampler2D sand;
uniform sampler2D snow;
uniform sampler2D lunar;

uniform vec3 viewPos;
uniform float waterHeight;


// In
in vec2 uv;
in vec3 fragPos;
in float height;
in vec3 normal;
in float slope; 

// Out
out vec4 color;

void main() {

    // Directional light source
    vec3 lightPos = vec3(1.0f, 1.0f, 4.0f);

    float sandLevel = waterHeight + 0.07f;
    float grassLevel = sandLevel + 0.3f;
    float rockLevel = grassLevel + 0.5f;
    float lunarLevel = rockLevel + 0.6f;
    float snowLevel = lunarLevel + 0.4f;
    float snowSlope = 0.05f; // slope is the dot product between the normal and the up vector, if the value is small => we get a perpendicular so the slope is too big
      
    vec4 col;
    float specularPower;

    if (height < sandLevel) {
        col = texture(sand, uv);
        specularPower = 1.0f;
    }

    // Texture with sand, then grass, then rock and blend with each others
    if (height >= sandLevel && height <= rockLevel) {
        col = texture(grass, uv);
        specularPower = 1.0f;

        // Calculate blend factor
        float halfDistance = (rockLevel - sandLevel) / 2.0f;

        if (height < (sandLevel + halfDistance)) {
            // Blend with sand
            float pos = height - sandLevel;
            float posScaled = pos / halfDistance;
            col = (texture(grass, uv) * (posScaled)) + (texture(sand, uv) * (1 - posScaled));
        } else {
            // Blend with rocks
            float pos = rockLevel - height;
            float posScaled = pos / halfDistance;
            col = (texture(grass, uv) * (posScaled)) + (texture(rock, uv) * (1 - posScaled));
        }
    }

    // Texture with grass, then rock,  then lunar
    if (height >= grassLevel && height <= lunarLevel) {
        col = texture(rock, uv);
        specularPower = 1.0f;

        // Calculate blend factor
        float halfDistance = (lunarLevel - grassLevel) / 2.0f;

        if (height < (grassLevel + halfDistance)) {
            // Blend with grass
            float pos = height - grassLevel;
            float posScaled = pos / halfDistance;
            col = (texture(rock, uv) * (posScaled)) + (texture(grass, uv) * (1 - posScaled));
        } else {
            // Blend with lunar
            float pos = lunarLevel - height;
            float posScaled = pos / halfDistance;
            col = (texture(rock, uv) * (posScaled)) + (texture(lunar, uv) * (1 - posScaled));
        }
    }

    // texture with lunar and snow
    if (height >= lunarLevel) {
        col = texture(lunar, uv);
        specularPower = 1.0f;

        // Calculate blend factor
        float quarterDistance = (snowLevel - lunarLevel) / 4.0f;

        if (height > (snowLevel - quarterDistance)) {
            // Blend with snow
            float pos = height - (snowLevel - quarterDistance);
            float posScaled = pos / quarterDistance;
            col = (texture(snow, uv) * (posScaled)) + (texture(lunar, uv) * (1 - posScaled));
        }
    }

    // Texture with snow
    if (height >= snowLevel) {
        if (slope < snowSlope) {
            // the dot product is too small, so the slope is too sharp for snow
            col = texture(rock, uv);
            specularPower = 1.0f;
        } else {
            col = texture(snow, uv);
            specularPower = 100.0f;
        }
    }

    // coefficients
    float ambient = 0.05f;
    float diffuse_coefficient = 0.2f;
    float specular_coefficient = 0.2f;

    // diffuse lighting
    vec3 lightDir = normalize(lightPos - fragPos);
    float diffuse = diffuse_coefficient * max(0.0f, -dot(normal, lightDir));

    // specular lighting
    vec3 view_direction = viewPos - fragPos;
    vec3 halfway = normalize(lightDir + view_direction);
    float specular = specular_coefficient * max(0.0f, pow(dot(normal, halfway), specularPower));


    if (diffuse > 0.0f) {
        col += (ambient + diffuse + specular);
    } else {
        col += (ambient + diffuse);
    }

    color = vec4(col);
}
)"