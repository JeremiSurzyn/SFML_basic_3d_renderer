#include "primitives.h"
#include <algorithm>
#include <vector>
#include <cmath>
#include <memory>
#include <map>
#include <utility>

Mesh createCube(float size, sf::Color color, const Transform& transform) {
    std::vector<std::unique_ptr<Triangle>> triangles;
    Vector3f vertices[8] = {
        {-size/2, -size/2, -size/2},
        {size/2, -size/2, -size/2},
        {size/2, size/2, -size/2},
        {-size/2, size/2, -size/2},
        {-size/2, -size/2, size/2},
        {size/2, -size/2, size/2},
        {size/2, size/2, size/2},
        {-size/2, size/2, size/2}
    };
    int faces[12][3] = {
        {0,2,1}, {0,3,2},
        {4,5,6}, {4,6,7},
        {0,4,7}, {0,7,3},
        {1,6,5}, {1,2,6},
        {3,7,6}, {3,6,2},
        {0,5,4}, {0,1,5}
    };
    for (auto& face : faces) {
        auto tri = std::make_unique<Triangle>(vertices[face[0]], vertices[face[1]], vertices[face[2]], color);
        triangles.push_back(std::move(tri));
    }
    return Mesh(std::move(triangles), color, transform);
}

Mesh createIcosahedron(float radius, sf::Color color, const Transform& transform) {
    std::vector<std::unique_ptr<Triangle>> triangles;
    const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;
    Vector3f verts[12] = {
        {0, 1, phi}, {0, 1, -phi}, {0, -1, phi}, {0, -1, -phi},
        {1, phi, 0}, {1, -phi, 0}, {-1, phi, 0}, {-1, -phi, 0},
        {phi, 0, 1}, {-phi, 0, 1}, {phi, 0, -1}, {-phi, 0, -1}
    };
    for (auto& v : verts) {
        v = v.normalized() * radius;
    }
    int faces[20][3] = {
        {4,6,0},{4,1,6},{7,5,2},{7,3,5},
        {10,8,5},{4,8,10},{7,9,11},{9,6,11},
        {2,5,8},{2,9,7},{9,0,6},{0,8,4},{0,2,8},{0,9,2},
        {11,3,7},{11,6,1},{1,4,10},{10,5,3},{10,3,1},{1,3,11}
    };
    for (auto& face : faces) {
        auto tri = std::make_unique<Triangle>(verts[face[0]], verts[face[1]], verts[face[2]], color);
        triangles.push_back(std::move(tri));
    }
    return Mesh(std::move(triangles), color, transform);
}


static int getMidpoint(int a, int b, std::vector<Vector3f>& verts, std::map<std::pair<int,int>, int>& cache, float radius) {
    if (a > b) std::swap(a,b);
    auto key = std::make_pair(a,b);
    auto it = cache.find(key);
    if (it != cache.end()) return it->second;
    Vector3f va = verts[a];
    Vector3f vb = verts[b];
    Vector3f mid = { (va.x + vb.x) * 0.5f, (va.y + vb.y) * 0.5f, (va.z + vb.z) * 0.5f };
    mid = mid.normalized() * radius;
    int idx = (int)verts.size();
    verts.push_back(mid);
    cache[key] = idx;
    return idx;
}

Mesh createIcosphere(float radius, int subdivisions, sf::Color color, const Transform& transform) {

    const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;
    std::vector<Vector3f> verts = {
        {0, 1, phi}, {0, 1, -phi}, {0, -1, phi}, {0, -1, -phi},
        {1, phi, 0}, {1, -phi, 0}, {-1, phi, 0}, {-1, -phi, 0},
        {phi, 0, 1}, {-phi, 0, 1}, {phi, 0, -1}, {-phi, 0, -1}
    };
    for (auto& v : verts) v = v.normalized() * radius;

    std::vector<std::array<int,3>> faces = {
        {4,6,0},{4,1,6},{7,5,2},{7,3,5},
        {10,8,5},{4,8,10},{7,9,11},{9,6,11},
        {2,5,8},{2,9,7},{9,0,6},{0,8,4},{0,2,8},{0,9,2},
        {11,3,7},{11,6,1},{1,4,10},{10,5,3},{10,3,1},{1,3,11}
    };

    for (int s = 0; s < subdivisions; ++s) {
        std::map<std::pair<int,int>, int> midpointCache;
        std::vector<std::array<int,3>> newFaces;
        for (auto& f : faces) {
            int v0 = f[0];
            int v1 = f[1];
            int v2 = f[2];
            int a = getMidpoint(v0, v1, verts, midpointCache, radius);
            int b = getMidpoint(v1, v2, verts, midpointCache, radius);
            int c = getMidpoint(v2, v0, verts, midpointCache, radius);
            newFaces.push_back({v0, a, c});
            newFaces.push_back({v1, b, a});
            newFaces.push_back({v2, c, b});
            newFaces.push_back({a, b, c});
        }
        faces.swap(newFaces);
    }

    std::vector<std::unique_ptr<Triangle>> triangles;
    triangles.reserve(faces.size());
    for (auto& f : faces) {
        Vector3f v0 = verts[f[0]].normalized() * radius;
        Vector3f v1 = verts[f[1]].normalized() * radius;
        Vector3f v2 = verts[f[2]].normalized() * radius;
        auto tri = std::make_unique<Triangle>(v0, v1, v2, color);
        triangles.push_back(std::move(tri));
    }
    Mesh mesh(std::move(triangles), color, transform);
    mesh.shadeSmooth = true;
    return mesh;
}
