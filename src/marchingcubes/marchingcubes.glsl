#version 460

#include "src/marchingcubes/marchingTables.glsl"

layout (std430, binding = 0) buffer Verts {
    vec4[] vertices;
};

layout (std430, binding = 1) buffer Tris {
    uvec4[] triangles;
};

layout(std430, binding = 2) buffer Counter {
    uint vertexCount;
    uint triangleCount;
};

uniform vec3  _Origine;
uniform float _CubeSize;
uniform float _IsoLevel;
uniform uvec3 _RegionSize;

float field(vec3 v) {
    vec3 center = vec3(10, 10, 10);
    return length(center - v) - 5;
}

vec3 interpolate_vertices(vec3 a, vec3 b, float vA, float vB) {

    float denom = (vB - vA);
    if (abs(denom) < 1e-6)
        return a;

    float t = (_IsoLevel - vA) / denom;
    return a + t * (b - a);
}

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

void main() {
    vec3 id = gl_GlobalInvocationID;

    // get the corners
    vec3 corners[8] = {
        _Origine + (id + vec3(0, 0, 0)) * _CubeSize,
        _Origine + (id + vec3(1, 0, 0)) * _CubeSize,
        _Origine + (id + vec3(1, 1, 0)) * _CubeSize,
        _Origine + (id + vec3(0, 1, 0)) * _CubeSize,
        _Origine + (id + vec3(0, 0, 1)) * _CubeSize,
        _Origine + (id + vec3(1, 0, 1)) * _CubeSize,
        _Origine + (id + vec3(1, 1, 1)) * _CubeSize,
        _Origine + (id + vec3(0, 1, 1)) * _CubeSize,
    };

    float values[8];
    for (int i = 0; i < 8; i++) {
        values[i] = field(corners[i]);
    }

    // figure out the configuration
    uint cubeIndex = 0;
    for (uint i = 0; i < 8; i++) {
        cubeIndex |= (uint(values[i] < _IsoLevel) << i); 
    }

    if (cubeIndex == 0 || cubeIndex == 255)
        return;

    for (int i = 0; triangulationTable[cubeIndex][i] != -1; i += 3) {
        int a0 = edgeCornerATable[triangulationTable[cubeIndex][i + 0]];
        int b0 = edgeCornerBTable[triangulationTable[cubeIndex][i + 0]];
        
        int a1 = edgeCornerATable[triangulationTable[cubeIndex][i + 1]]; 
        int b1 = edgeCornerBTable[triangulationTable[cubeIndex][i + 1]]; 
        
        int a2 = edgeCornerATable[triangulationTable[cubeIndex][i + 2]]; 
        int b2 = edgeCornerBTable[triangulationTable[cubeIndex][i + 2]];

        vec3 a = interpolate_vertices(corners[a0], corners[b0], values[a0], values[b0]);
        vec3 b = interpolate_vertices(corners[a1], corners[b1], values[a1], values[b1]);
        vec3 c = interpolate_vertices(corners[a2], corners[b2], values[a2], values[b2]);

        // push the triangle
        uint baseVertIndex = atomicAdd(vertexCount, 3);
        uint baseTriIndex  = atomicAdd(triangleCount, 1);
        vertices[baseVertIndex + 0] = vec4(a, 1.0);
        vertices[baseVertIndex + 1] = vec4(b, 1.0);
        vertices[baseVertIndex + 2] = vec4(c, 1.0);
        
        triangles[baseTriIndex] = uvec4(baseVertIndex, baseVertIndex + 1, baseVertIndex + 2, 0.0);
    }
}