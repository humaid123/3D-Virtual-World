R"(
#version 330 core
//uniform sampler2D noiseTex;

in vec3 vposition;
in vec2 vtexcoord;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 viewPos; // camera position, we translate the grid using it...

out vec2 uv;
out vec3 fragPos;
out float waterHeight;
out vec3 normal;
out float height;
out float slope;


// =====================================================================================
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


// =================================================================

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
    float frequency = 0.7;


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
	if (rmd != 0.0) value += (rmd * snoise(point*frequency) * pwr);

	return value;
}

void main() {

    // old =>  vec3 position = vposition;
    vec3 position = vposition + (vec3(viewPos.x, viewPos.y, 0)); // displace the position so that we get an infinite world

    // old - uv = vtexcoord;
    // we get the tex coordinate from the perturbed position normalised in [0, 1], so we bring to 0 to f_width 
    // and we divide by f_width
    uv = vec2(
        (position.x + 5.0/2.0) / 5, 
        (position.y + 5.0/2.0) / 5
    );

    // Earth scene
    float water = 0.7f;
    
    // Calculate height
    // float h = (texture(noiseTex, uv).r + 1.0f) / 2.0f;
    // float h = 2 * cnoise(position.xy);
    float h = hybridMultiFractal(position.xy);
    // h = max(h, water);
    height = h;

    // calculate the normal
    // Calculate surface normal N

    // classic perlin
    // vec3 A = vec3(position.x + 1.0f, position.y       , 2*cnoise(position.xy + (1, 0))    );
    // vec3 B = vec3(position.x - 1.0f, position.y       , 2*cnoise(position.xy + (-1, 0))   );
    // vec3 C = vec3(position.x       , position.y + 1.0f, 2*cnoise(position.xy + (0, 1))    );
    // vec3 D = vec3(position.x       , position.y - 1.0f, 2*cnoise(position.xy + (0, -1))   );


    // hybrid multifractal
    vec3 A = vec3(position.x + 1.0f, position.y       , hybridMultiFractal(position.xy + (1, 0))    );
    vec3 B = vec3(position.x - 1.0f, position.y       , hybridMultiFractal(position.xy + (-1, 0))   );
    vec3 C = vec3(position.x       , position.y + 1.0f, hybridMultiFractal(position.xy + (0, 1))    );
    vec3 D = vec3(position.x       , position.y - 1.0f, hybridMultiFractal(position.xy + (0, -1))   );
    vec3 n = normalize( cross(A - B , C - D) );
    normal = n;

    // Calculate Slopes & Levels
    float denom = 1.0f;
    float s = 0.0f;
    s = max(s, (A.y - position.y) / denom);
    s = max(s, (B.y - position.y) / denom);
    s = max(s, (C.y - position.y) / denom);
    s = max(s, (D.y - position.y) / denom);
    slope = s;

    // Set fragment position
    fragPos = position.xyz + vec3(0,0,h);

    // Set gl_Position
    gl_Position = P*V*M*vec4(fragPos, 1.0f);

    // Set height of water
    waterHeight = water;
}
)"