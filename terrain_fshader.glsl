R"(
#version 330 core

// Uniforms
uniform sampler2D noiseTex;
uniform sampler2D grass;
uniform sampler2D rock;
uniform sampler2D sand;
uniform sampler2D snow;
uniform sampler2D water;
uniform sampler2D lunar;

uniform float waveMotion;
uniform vec3 viewPos;

// In
in vec2 uv;
in vec3 fragPos;
in float waterHeight;

// Out
out vec4 color;

void main() {

    // Directional light source
    //vec3 lightPos = vec3(1.0f, 1.0f, 3.0f);
    vec3 lightPos = vec3(1.0f, 1.0f, 1.0f);

    // Texture size in pixels
    ivec2 size = textureSize(noiseTex, 0);

    // Calculate distorted wave positions
    vec2 layeredWaterCoordinates = texture(water, vec2(uv.x + waveMotion, uv.y)).rg * 0.01f;
    layeredWaterCoordinates = uv + vec2(layeredWaterCoordinates.x, layeredWaterCoordinates.y + waveMotion);

    // Calculate surface normal N
    vec3 A = vec3(uv.x + 1.0f / size.x, uv.y, textureOffset(noiseTex, uv, ivec2(1, 0)));
    vec3 B = vec3(uv.x - 1.0f / size.x, uv.y, textureOffset(noiseTex, uv, ivec2(-1, 0)));
    vec3 C = vec3(uv.x, uv.y + 1.0f / size.y, textureOffset(noiseTex, uv, ivec2(0, 1)));
    vec3 D = vec3(uv.x, uv.y - 1.0f / size.y, textureOffset(noiseTex, uv, ivec2(0, -1)));
    vec3 normal = normalize( cross(normalize(A - B), normalize(C - D)) );

    // Adjust Slopes & Levels
    float denom = 1.0f / size.x;
    float slope = 0.0f;
    slope = max(slope, (A.y - fragPos.y) / denom);
    slope = max(slope, (B.y - fragPos.y) / denom);
    slope = max(slope, (C.y - fragPos.y) / denom);
    slope = max(slope, (D.y - fragPos.y) / denom);

    // Slopes & Levels - Earth (fBm2DTexture)
    float tempWaterHeight = waterHeight;
    float sandLevel = waterHeight + 0.02f;
    float grassLevel = sandLevel + 0.07f;
    float snowLevel = 1.0f;
    float sandSlope = 0.7f;
    float snowSlope = 0.85f;
    vec4 col = texture(sand, uv);
    vec2 tempAdj = vec2(0.1f, 0.1f);

    // Texture according to height and slope
    float height = (texture(noiseTex, uv).r + 1.0f) / 2.0f;
    float specularPower = (height / 1.0f) * 10.0f;

    // Texture with water if height lies below waterHeight
    if (height < tempWaterHeight) {
        col = texture(water, layeredWaterCoordinates);
        specularPower = 200.0f;
    }

    // Texture with sand
    if (height > tempWaterHeight && height < sandLevel && slope < sandSlope) {
        col = texture(sand, uv);
        specularPower = 1.0f;
    }

    // Texture with grass
    if (height > sandLevel && height < grassLevel) {
        //col = texture(grass, uv);
        specularPower = 1.0f;

        // Calculate blend factor
        float halfDistance = (grassLevel - sandLevel) / 2.0f;

        if (height < (sandLevel + halfDistance)) {
            // Blend with sand
            float pos = height - sandLevel;
            float posScaled = pos / halfDistance;
            col = (texture(grass, uv) * (posScaled)) + (texture(sand, uv) * (1 - posScaled));
        } else {
            // Blend with rocks
            float pos = grassLevel - height;
            float posScaled = pos / halfDistance;
            col = (texture(grass, uv) * (posScaled)) + (texture(rock, uv) * (1 - posScaled));
        }
    }

    // Texture with rock
    if (height > grassLevel) {
        col = texture(rock, uv);
        specularPower = 1.0f;

        // Calculate blend factor
        float quarterDistance = (snowLevel - grassLevel) / 4.0f;

        if (height > (snowLevel - quarterDistance)) {

            // Blend with snow
            float pos = height - (snowLevel - quarterDistance);
            float posScaled = pos / quarterDistance;
            col = (texture(snow, uv) * (posScaled)) + (texture(rock, uv) * (1 - posScaled));
        }
    }

    // Texture with snow
    if (height > snowLevel && slope < snowSlope) {
        col = texture(snow, uv);
        specularPower = 100.0f;
    }

    // Calculate ambient lighting factor
    float ambient = 0.05f;
    float diffuse_coefficient = 0.2f;
    float specular_coefficient = 0.2f;

    // Calculate diffuse lighting factor
    vec3 lightDir = normalize(lightPos - fragPos);
    float diffuse = diffuse_coefficient * max(0.0f, -dot(normal, lightDir));

    // Calculate specular lighting factor
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