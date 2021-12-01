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

//	Classic Perlin 2D Noise 
//	by Stefan Gustavson
//
vec2 fade(vec2 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}
vec4 permute(vec4 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float cnoise(vec2 P){
    vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation
    vec4 ix = Pi.xzxz;
    vec4 iy = Pi.yyww;
    vec4 fx = Pf.xzxz;
    vec4 fy = Pf.yyww;
    vec4 i = permute(permute(ix) + iy);
    vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024...
    vec4 gy = abs(gx) - 0.5;
    vec4 tx = floor(gx + 0.5);
    gx = gx - tx;
    vec2 g00 = vec2(gx.x,gy.x);
    vec2 g10 = vec2(gx.y,gy.y);
    vec2 g01 = vec2(gx.z,gy.z);
    vec2 g11 = vec2(gx.w,gy.w);
    vec4 norm = 1.79284291400159 - 0.85373472095314 * 
        vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
    g00 *= norm.x;
    g01 *= norm.y;
    g10 *= norm.z;
    g11 *= norm.w;
    float n00 = dot(g00, vec2(fx.x, fy.x));
    float n10 = dot(g10, vec2(fx.y, fy.y));
    float n01 = dot(g01, vec2(fx.z, fy.z));
    float n11 = dot(g11, vec2(fx.w, fy.w));
    vec2 fade_xy = fade(Pf.xy);
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
    return 2.3 * n_xy;
}


// hybrid multifractal from https://www.shadertoy.com/view/4sXXW2
float hybridMultiFractal(vec2 point) {
    // set up
    float H = 0.25;
    float offset =  0.7;
    float lacunarity = 2;
    float octaves = 8;
    float frequency = 0.45; //0.7;


	float value = 1.0;
	float signal = 0.0;
	float rmd = 0.0;
	float pwHL = pow(lacunarity, -H);
	float pwr = pwHL;
	float weight = 0.;

	/* get first octave of function */
	// built with simplex => value = pwr*(snoise(point * frequency)+offset);
    // built with classic Perlin noise
    value = pwr*(cnoise(point * frequency)+offset);
	weight = value;
	point *= lacunarity;
	pwr *= pwHL;

	/* spectral construction inner loop, where the fractal is built */
	for (int i=1; i<65535; i++)
	{
		weight = weight>1. ? 1. : weight;

		// built with simplex => signal = pwr * (snoise(point*frequency) + offset);
        // built with classic Perlin noise
        signal = pwr * (cnoise(point*frequency) + offset);
		
        
        value += weight*signal;
		weight *= signal;
		pwr *= pwHL;
		point *= lacunarity;
		if (i==int(octaves)-1) break;
	}

	/* take care of remainder in ``octaves''  */
	rmd = octaves - floor(octaves);
	if (rmd != 0.0) value += (rmd * cnoise(point*frequency) * pwr);

	return value;
}

void main() {

    vec3 position = vposition + (vec3(viewPos.x, viewPos.y, 0)); // displace the position so that we get an infinite world

    // we texture based on the perturbed position now so that the texturing scale with the infinite world
    // we get the tex coordinate from the perturbed position normalised in [0, 1], so we bring to 0 to f_width 
    // and we divide by f_width
    uv = (position.xy + 20/2) / 20; 
    // we tile the textures to improve the resolution
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