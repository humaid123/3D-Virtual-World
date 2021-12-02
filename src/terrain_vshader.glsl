R"(
#version 330 core

in vec3 vposition;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 viewPos; // camera position, we translate the grid using it...

// clip plane
uniform vec3 clipPlaneNormal;
uniform float clipPlaneHeight;

out vec2 uv;
out vec3 fragPos;
out vec3 normal;
out float height;
out float slope;
out vec3 distanceFromCamera;


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

// adapted from texturing and modeling a procedural approach
float hybridMultiFractal(vec2 point) {
    // set up
    float H = 0.25;
    float offset =  0.7;
    float lacunarity = 2;
    float octaves = 8;
    float frequency = 0.55;// 0.45; //0.7;

    float value = 1.0;
    float signal = 0.0;
    float rmd = 0.0;
    float pwHL = pow(lacunarity, -H);
    float pwr = pwHL;
    float weight = 0.;

    // get zeroth octave of function
    value = pwr*(Perlin2D(point * frequency)+offset);
    weight = value;
    point *= lacunarity;
    pwr *= pwHL;

    // start from 1 and start weighing the signals
    for (int i=1; i<octaves; i++) {
        weight = weight > 1.0f ? 1.0f : weight;

        signal = pwr*(Perlin2D(point * frequency)+offset);
        
        value += weight*signal;
        weight *= signal;
        pwr *= pwHL;
        point *= lacunarity;
    }

    return value;
}

void main() {

    vec3 position = vposition + (vec3(viewPos.x, viewPos.y, 0)); // displace the position so that we get an infinite world

    // we texture based on the perturbed position now so that the texturing scale with the infinite world
    // we get the tex coordinate from the perturbed position normalised in [0, 1], so we bring to 0 to f_width 
    // and we divide by f_width
    uv = (position.xy + 20/2) / 20; 

    // we tile the textures to improve the resolution => instead of [0, 1], we use [0, 40]
    // openGL with GL_REPEAT will just mod it for us
    int num_tiles = 40; 
    uv = (uv * num_tiles); // make opengl repeat the texture so we get higher resolution 
    
    // Calculate height
    float h = hybridMultiFractal(position.xy);
    height = h;

    // find the normal by finding two vectors on the plane next to the point and cross producting them
    vec3 A = vec3(position.x + 1.0f, position.y       , hybridMultiFractal(position.xy + (1, 0))    );
    vec3 B = vec3(position.x - 1.0f, position.y       , hybridMultiFractal(position.xy + (-1, 0))   );
    vec3 C = vec3(position.x       , position.y + 1.0f, hybridMultiFractal(position.xy + (0, 1))    );
    vec3 D = vec3(position.x       , position.y - 1.0f, hybridMultiFractal(position.xy + (0, -1))   );
    vec3 n = normalize( cross(A - B , C - D) );
    normal = n;

    // the up vector is (0, 0, 1) so the dot of the top and the normal vector is normal.z
    // we take acos to get the actual gradient
    slope = acos(normal.z);

    // give new disturbed position
    fragPos = position.xyz + vec3(0,0,h);
    gl_Position = P*V*M*vec4(fragPos, 1.0f);

    // set distance from camera to scale the visibility
    distanceFromCamera = fragPos - viewPos;


    // we also add a clipping plane for the water rendering...
    // set up clipping planes
    // every vertex now needs to give a distance from it to the clip plane
    // if that distance is positive, the vertex is rendered
    // if that distance is negative, the vertex is not rendered
    // opengl will interpolate the distance for points in the middle of vertices to clip them
    
    // to define a plane, we can just use the plane equations
    // we can declare a z = k plane with a normal of (0, -1, 0) and a height
    vec4 plane = vec4(clipPlaneNormal, clipPlaneHeight); // we can represent a vector with just this
    
    // to find the distance from a vertex to a plane, you just need to 
    // do the dot product of the vertex with the plane vector
    // need to compare with NON view-space coordinates, i.e, pre-transformation as plane is defined pre-transformation
    float distance_of_this_vertex = dot(plane, vec4(fragPos, 1.0)); 
    gl_ClipDistance[0] = distance_of_this_vertex;
}
)"