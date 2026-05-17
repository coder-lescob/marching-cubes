/**
 * noise functions from https://github.com/czinn/perlin
 */

float rawnoise(int n) {
    n = (n << 13) ^ n;
    return (1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
}

float noise1d(int x, int octave, int seed) {
    return rawnoise(x * 1619 + octave * 3463 + seed * 13397);
}

float noise2d(int x, int y, int octave, int seed) {
    return rawnoise(x * 1619 + y * 31337 + octave * 3463 + seed * 13397);
}

float noise3d(int x, int y, int z, int octave, int seed) {
    return rawnoise(x * 1919 + y * 31337 + z * 7669 + octave * 3463 + seed * 13397);
}

float interpolate(float a, float b, float x) {
    float f = (1 - cos(x * 3.141593)) * 0.5;

    return a * (1 - f) + b * f;
}

float smooth1d(float x, int octave, int seed) {
    int intx = int(x);
    float fracx = x - intx;

    float v1 = noise1d(intx, octave, seed);
    float v2 = noise1d(intx + 1, octave, seed);

    return interpolate(v1, v2, fracx);
}

float smooth2d(float x, float y, int octave, int seed) {
    int intx = int(x);
    float fracx = x - intx;
    int inty = int(y);
    float fracy = y - inty;
    
    float v1 = noise2d(intx, inty, octave, seed);
    float v2 = noise2d(intx + 1, inty, octave, seed);
    float v3 = noise2d(intx, inty + 1, octave, seed);
    float v4 = noise2d(intx + 1, inty + 1, octave, seed);
    
    float i1 = interpolate(v1, v2, fracx);
    float i2 = interpolate(v3, v4, fracx);
    
    return interpolate(i1, i2, fracy);
}

float smooth3d(float x, float y, float z, int octave, int seed) {
    int intx = int(x);
    float fracx = x - intx;
    int inty = int(y);
    float fracy = y - inty;
    int intz = int(z);
    float fracz = z - intz;


    float v1 = noise3d(intx, inty, intz, octave, seed);
    float v2 = noise3d(intx + 1, inty, intz, octave, seed);
    float v3 = noise3d(intx, inty + 1, intz, octave, seed);
    float v4 = noise3d(intx + 1, inty + 1, intz, octave, seed);
    float v5 = noise3d(intx, inty, intz + 1, octave, seed);
    float v6 = noise3d(intx + 1, inty, intz + 1, octave, seed);
    float v7 = noise3d(intx, inty + 1, intz + 1, octave, seed);
    float v8 = noise3d(intx + 1, inty + 1, intz + 1, octave, seed);

    float i1 = interpolate(v1, v2, fracx);
    float i2 = interpolate(v3, v4, fracx);
    float i3 = interpolate(v5, v6, fracx);
    float i4 = interpolate(v7, v8, fracx);
    
    float j1 = interpolate(i1, i2, fracy);
    float j2 = interpolate(i3, i4, fracy);

    return interpolate(j1, j2, fracz);
}

float pnoise1d(float x, float persistence, int octaves, int seed) {
   float total = 0.0;
   float frequency = 1.0;
   float amplitude = 1.0;
   int i = 0;
   
   for(i = 0; i < octaves; i++) {
       total += smooth1d(x * frequency, i, seed) * amplitude;
       frequency /= 2;
       amplitude *= persistence;
   } 

   return total;
}

float pnoise2d(float x, float y, float persistence, int octaves, int seed) {
   float total = 0.0;
   float frequency = 1.0;
   float amplitude = 1.0;
   int i = 0;
   
   for(i = 0; i < octaves; i++) {
       total += smooth2d(x * frequency, y * frequency, i, seed) * amplitude;
       frequency /= 2;
       amplitude *= persistence;
   } 

   return total;
}

float pnoise3d(float x, float y, float z, float persistence, int octaves, int seed) {
   float total = 0.0;
   float frequency = 1.0;
   float amplitude = 1.0;
   int i = 0;
   
   for(i = 0; i < octaves; i++) {
       total += smooth3d(x * frequency, y * frequency, z * frequency, i, seed) * amplitude;
       frequency /= 2;
       amplitude *= persistence;
   } 

   return total;
}
