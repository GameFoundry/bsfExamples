///////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
///////////////////////////////////////////////////////////////////////////////

#include "mesh.h"
#include "noise.h"
#include <map>
#include <random>
#include <cassert>

#include "Mesh/BsMesh.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Math/BsVector2.h"
#include "Mesh/BsMeshUtility.h"

typedef bs::UINT32 IndexType;

typedef bs::Vector3 Vertex;
typedef bs::Vector3 Normal;

using bs::Vector2;

namespace demo {

// typedef unsigned short IndexType;

// NOTE: This data could be compressed, but it's not really the bottleneck at the moment
// struct Vertex
// {
//     float x;
//     float y;
//     float z;
// };


// struct Normal
// {
//     float nx;
//     float ny;
//     float nz;
// };

struct Mesh
{
    void clear()
    {
        normals.clear();
        vertices.clear();
        indices.clear();
    }

    std::vector<Vertex> vertices;
    std::vector<Normal> normals;
    std::vector<IndexType> indices;
    std::vector<Vector2> uv;
};


void CreateIcosahedron(Mesh *outMesh);

// 1 face -> 4 faces
void SubdivideInPlace(Mesh *outMesh);

void SpherifyInPlace(Mesh *outMesh, float radius = 1.0f);

void ComputeAvgNormalsInPlace(Mesh *outMesh);

void CreateUVMap(Mesh *outMesh);

// subdivIndexOffset array should be [subdivLevels+2] in size
void CreateGeospheres(Mesh *outMesh, unsigned int subdivLevelCount, unsigned int* outSubdivIndexOffsets);


void CreateIcosahedron(Mesh *outMesh)
{
    static const float a = std::sqrt(2.0f / (5.0f - std::sqrt(5.0f)));
    static const float b = std::sqrt(2.0f / (5.0f + std::sqrt(5.0f)));

    static const size_t num_vertices = 12;
    static const Vertex vertices[num_vertices] = // x, y, z
    {
        {-b,  a,  0},
        { b,  a,  0},
        {-b, -a,  0},
        { b, -a,  0},
        { 0, -b,  a},
        { 0,  b,  a},
        { 0, -b, -a},
        { 0,  b, -a},
        { a,  0, -b},
        { a,  0,  b},
        {-a,  0, -b},
        {-a,  0,  b},
    };

    static const size_t num_triangles = 20;
    static const IndexType indices[num_triangles*3] =
    {
         0,  5, 11,
         0,  1,  5,
         0,  7,  1,
         0, 10,  7,
         0, 11, 10,
         1,  9,  5,
         5,  4, 11,
        11,  2, 10,
        10,  6,  7,
         7,  8,  1,
         3,  4,  9,
         3,  2,  4,
         3,  6,  2,
         3,  8,  6,
         3,  9,  8,
         4,  5,  9,
         2, 11,  4,
         6, 10,  2,
         8,  7,  6,
         9,  1,  8,
    };

    outMesh->clear();
    outMesh->vertices.insert(outMesh->vertices.end(), vertices, vertices + num_vertices);
    outMesh->indices.insert(outMesh->indices.end(), indices, indices + num_triangles*3);
    outMesh->normals.resize(outMesh->vertices.size());
}


// Maps edge (lower index first!) to
struct Edge
{
    Edge(IndexType i0, IndexType i1)
        : v0(i0), v1(i1)
    {
        if (v0 > v1)
            std::swap(v0, v1);
    }
    IndexType v0;
    IndexType v1;

    bool operator<(const Edge &c) const
    {
        return v0 < c.v0 || (v0 == c.v0 && v1 < c.v1);
    }
};

typedef std::map<Edge, IndexType> MidpointMap;

inline IndexType EdgeMidpoint(Mesh *mesh, MidpointMap *midpoints, Edge e)
{
    auto index = midpoints->find(e);
    if (index == midpoints->end())
    {
        auto a = mesh->vertices[e.v0];
        auto b = mesh->vertices[e.v1];

        Vertex m;
        m.x = (a.x + b.x) * 0.5f;
        m.y = (a.y + b.y) * 0.5f;
        m.z = (a.z + b.z) * 0.5f;

        index = midpoints->insert(std::make_pair(e, static_cast<IndexType>(mesh->vertices.size()))).first;
        mesh->vertices.push_back(m);
    }
    return index->second;
}


void SubdivideInPlace(Mesh *outMesh)
{
    MidpointMap midpoints;

    std::vector<IndexType> newIndices;
    newIndices.reserve(outMesh->indices.size() * 4);
    outMesh->vertices.reserve(outMesh->vertices.size() * 2);

    assert(outMesh->indices.size() % 3 == 0); // trilist
    size_t triangles = outMesh->indices.size() / 3;
    for (size_t t = 0; t < triangles; ++t)
    {
        auto t0 = outMesh->indices[t*3+0];
        auto t1 = outMesh->indices[t*3+1];
        auto t2 = outMesh->indices[t*3+2];

        auto m0 = EdgeMidpoint(outMesh, &midpoints, Edge(t0, t1));
        auto m1 = EdgeMidpoint(outMesh, &midpoints, Edge(t1, t2));
        auto m2 = EdgeMidpoint(outMesh, &midpoints, Edge(t2, t0));

        IndexType indices[] = {
            t0, m0, m2,
            m0, t1, m1,
            m0, m1, m2,
            m2, m1, t2,
        };
        newIndices.insert(newIndices.end(), indices, indices + 4*3);
    }

    std::swap(outMesh->indices, newIndices); // Constant time
}


void SpherifyInPlace(Mesh *outMesh, float radius)
{
    for (auto &v : outMesh->vertices) {
        float n = radius / std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
        v.x *= n;
        v.y *= n;
        v.z *= n;
    }
}


void ComputeAvgNormalsInPlace(Mesh *outMesh)
{
    for (auto &v : outMesh->normals) {
        v.x = 0.0f;
        v.y = 0.0f;
        v.z = 0.0f;
    }

    assert(outMesh->indices.size() % 3 == 0); // trilist
    size_t triangles = outMesh->indices.size() / 3;
    for (size_t t = 0; t < triangles; ++t)
    {
        auto v1 = &outMesh->vertices[outMesh->indices[t*3+0]];
        auto v2 = &outMesh->vertices[outMesh->indices[t*3+1]];
        auto v3 = &outMesh->vertices[outMesh->indices[t*3+2]];

        auto n1 = &outMesh->normals[outMesh->indices[t*3+0]];
        auto n2 = &outMesh->normals[outMesh->indices[t*3+1]];
        auto n3 = &outMesh->normals[outMesh->indices[t*3+2]];

        // Two edge vectors u,v
        auto ux = v2->x - v1->x;
        auto uy = v2->y - v1->y;
        auto uz = v2->z - v1->z;
        auto vx = v3->x - v1->x;
        auto vy = v3->y - v1->y;
        auto vz = v3->z - v1->z;

        // cross(u,v)
        float nx = uy*vz - uz*vy;
        float ny = uz*vx - ux*vz;
        float nz = ux*vy - uy*vx;

        // Do not normalize... weight average by contributing face area
        n1->x += nx; n1->y += ny; n1->z += nz;
        n2->x += nx; n2->y += ny; n2->z += nz;
        n3->x += nx; n3->y += ny; n3->z += nz;
    }

    // Normalize
    for (auto &v : outMesh->normals) {
        float n = 1.0f / std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
        v.x *= n;
        v.y *= n;
        v.z *= n;
    }
}


void CreateGeospheres(Mesh *outMesh, unsigned int subdivLevelCount, unsigned int* outSubdivIndexOffsets)
{
    CreateIcosahedron(outMesh);
    outSubdivIndexOffsets[0] = 0;

    std::vector<Vertex> vertices(outMesh->vertices);
    std::vector<Normal> normals(outMesh->normals);
    std::vector<IndexType> indices(outMesh->indices);

    for (unsigned int i = 0; i < subdivLevelCount; ++i) {
        outSubdivIndexOffsets[i+1] = (unsigned int)indices.size();
        SubdivideInPlace(outMesh);

        // Ensure we add the proper offset to the indices from this subdiv level for the combined mesh
        // This avoids also needing to track a base vertex index for each subdiv level
        IndexType vertexOffset = (IndexType)vertices.size();
        vertices.insert(vertices.end(), outMesh->vertices.begin(), outMesh->vertices.end());
        normals.resize(vertices.size());

        for (auto newIndex : outMesh->indices) {
            indices.push_back(newIndex + vertexOffset);
        }
    }
    outSubdivIndexOffsets[subdivLevelCount+1] = (unsigned int)indices.size();

    SpherifyInPlace(outMesh);

    // Put the union of vertices/indices back into the mesh object
    std::swap(outMesh->indices, indices);
    std::swap(outMesh->vertices, vertices);
    std::swap(outMesh->normals, normals);
}


void CreateUVMap(Mesh* outMesh) {
    std::vector<Vector2> uvs(outMesh->vertices.size());
    auto size = outMesh->vertices.size();
    for (unsigned int i = 0; i < size; ++i) {
        uvs[i] = {outMesh->vertices[i].x, outMesh->vertices[i].y};
    }
    std::swap(outMesh->uv, uvs);

    // make tangents as well

}

void CreateAsteroidsFromGeospheres(Mesh *outMesh,
                                   unsigned int subdivLevelCount, unsigned int meshInstanceCount,
                                   unsigned int rngSeed,
                                   unsigned int* outSubdivIndexOffsets, unsigned int* vertexCountPerMesh)
{
    assert(subdivLevelCount <= meshInstanceCount);

    std::mt19937 rng(rngSeed);

    Mesh baseMesh;
    CreateGeospheres(&baseMesh, subdivLevelCount, outSubdivIndexOffsets);

    // Per unique mesh
    *vertexCountPerMesh = (unsigned int)baseMesh.vertices.size();
    std::vector<Vertex> vertices;
    vertices.reserve(meshInstanceCount * baseMesh.vertices.size());
    std::vector<Normal> normals;
    normals.reserve(meshInstanceCount * baseMesh.normals.size());
    // Reuse indices for the different unique meshes

    auto randomNoise = std::uniform_real_distribution<float>(0.0f, 10000.0f);
    auto randomPersistence = std::normal_distribution<float>(0.95f, 0.04f);
    float noiseScale = 0.5f;
    float radiusScale = 0.9f;
    float radiusBias = 0.3f;

    // Create and randomize unique vertices for each mesh instance
    for (unsigned int m = 0; m < meshInstanceCount; ++m) {
        Mesh newMesh(baseMesh);
        NoiseOctaves<4> textureNoise(randomPersistence(rng));
        float noise = randomNoise(rng);

        for (auto &v : newMesh.vertices) {
            float radius = textureNoise(v.x*noiseScale, v.y*noiseScale, v.z*noiseScale, noise);
            radius = radius * radiusScale + radiusBias;
            v.x *= radius;
            v.y *= radius;
            v.z *= radius;
        }
        ComputeAvgNormalsInPlace(&newMesh);

        vertices.insert(vertices.end(), newMesh.vertices.begin(), newMesh.vertices.end());
        normals.insert(normals.end(), newMesh.normals.begin(), newMesh.normals.end());
    }

    // Copy to output
    std::swap(outMesh->indices, baseMesh.indices);
    std::swap(outMesh->vertices, vertices);
    std::swap(outMesh->normals, normals);

    // Quick hack to create UV map.
    CreateUVMap(outMesh);
}

} // namespace demo

namespace bs {

void generateTangents(UINT8* positions, UINT8* normals, UINT8* uv, UINT32* indices,
    UINT32 numVertices, UINT32 numIndices, UINT32 vertexOffset, UINT32 indexOffset, UINT32 vertexStride, UINT8* tangents)
{
    Vector3* tempTangents = bs_stack_alloc<Vector3>(numVertices);
    Vector3* tempBitangents = bs_stack_alloc<Vector3>(numVertices);

    MeshUtility::calculateTangents(
        (Vector3*)(positions + vertexOffset * vertexStride),
        (Vector3*)(normals + vertexOffset * vertexStride),
        (Vector2*)(uv + vertexOffset * vertexStride),
        (UINT8*)(indices + indexOffset),
        numVertices, numIndices, tempTangents, tempBitangents, 4, vertexStride);

    for (UINT32 i = 0; i < (UINT32)numVertices; i++)
    {
        Vector3 normal = *(Vector3*)&normals[(vertexOffset + i) * vertexStride];
        Vector3 tangent = tempTangents[i];
        Vector3 bitangent = tempBitangents[i];

        Vector3 engineBitangent = Vector3::cross(normal, tangent);
        float sign = Vector3::dot(engineBitangent, bitangent);

        Vector4 packedTangent(tangent.x, tangent.y, tangent.z, sign > 0 ? 1.0f : -1.0f);
        memcpy(tangents + (vertexOffset + i) * vertexStride, &packedTangent, sizeof(Vector4));
    }

    bs_stack_free(tempBitangents);
    bs_stack_free(tempTangents);
}

void CalculateTangents(SPtr<MeshData> meshData) {
    const SPtr<VertexDataDesc>& desc = meshData->getVertexDesc();
    UINT32* indexData = meshData->getIndices32();
    UINT8* positionData = meshData->getElementData(VES_POSITION);
    UINT8* normalData = meshData->getElementData(VES_NORMAL);

    UINT32 numVertices = meshData->getNumVertices();
    UINT32 numIndices = meshData->getNumIndices();
    UINT32 vertexStride = desc->getVertexStride();

    unsigned int vertexOffset = 0;
    unsigned int indexOffset = 0;
    UINT8* uvData = meshData->getElementData(VES_TEXCOORD);
    UINT8* tangentData = meshData->getElementData(VES_TANGENT);
    generateTangents(positionData, normalData, uvData, indexData, numVertices, numIndices,
        vertexOffset, indexOffset, vertexStride, tangentData);
}

void makeMeshes(unsigned int meshInstanceCount, unsigned int subdivCount, std::vector<HMesh>& meshes) {


    std::vector<unsigned int> mIndexOffsets(subdivCount + 2);
    unsigned int mSubdivCount(subdivCount);
    unsigned int mVertexCountPerMesh{0};
    demo::Mesh _meshes; // one big mesh with lots of sub-meshes...
    unsigned int rng = 100;
    demo::CreateAsteroidsFromGeospheres(&_meshes, mSubdivCount, meshInstanceCount, rng, mIndexOffsets.data(), &mVertexCountPerMesh);
    assert(_meshes.vertices.size() == _meshes.normals.size());

    // std::cout << "meshInstanceCount " << meshInstanceCount << " " << subdivCount << " " << mIndexOffsets.size() << " " << mVertexCountPerMesh << std::endl;
    // std::cout << "vertices " << _meshes.vertices.size() << " " << _meshes.indices.size() << std::endl;

    // for (auto i : mIndexOffsets) {
    //     std::cout << i << " ";
    // }
    // std::cout << std::endl;


    SPtr<VertexDataDesc> vertexDesc = VertexDataDesc::create();
    vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
    vertexDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
    vertexDesc->addVertElem(VET_FLOAT3, VES_TANGENT);
    vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

    for (unsigned int i = 0; i < meshInstanceCount; ++i) {
        MESH_DESC meshDesc;

        unsigned int j = subdivCount;
        unsigned int vertexOffset = i * mVertexCountPerMesh;
        meshDesc.vertexDesc = vertexDesc;
        meshDesc.numVertices = mVertexCountPerMesh;
        meshDesc.numIndices = mIndexOffsets[j + 1] - mIndexOffsets[j];

        SPtr<MeshData> meshData = MeshData::create(meshDesc.numVertices, meshDesc.numIndices, vertexDesc);
        // Write the vertices
        meshData->setVertexData(VES_POSITION, &_meshes.vertices[vertexOffset], sizeof(Vertex) * meshDesc.numVertices);
        meshData->setVertexData(VES_NORMAL, &_meshes.normals[vertexOffset], sizeof(Vertex) * meshDesc.numVertices);
        meshData->setVertexData(VES_TEXCOORD, &_meshes.uv[vertexOffset], sizeof(Vector2) * meshDesc.numVertices);

        // meshData->setIndexData(VES_NORMAL, _meshes.indices[0], sizeof(IndexType) * meshDesc.numIndices);
        memcpy(meshData->getIndices32(), &_meshes.indices[mIndexOffsets[j]], sizeof(IndexType) * meshDesc.numIndices);

        CalculateTangents(meshData);
        // auto bounds = meshData->calculateBounds();

        // THIS DOESN'T WORK? IS THIS A DOCS ISSUE??
        // bool discard = false;
        // worth investigating....
        // HMesh mesh = Mesh::create(meshDesc);
        // mesh->writeData(meshData, discard);
        // but initializing with meshData does work...
        HMesh mesh = Mesh::create(meshData);

        meshes.push_back(mesh);
    }
    // std::cout << "FINISH " << std::endl;
}

}
