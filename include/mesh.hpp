#ifndef MESH_H
#define MESH_H

#include <vector>
#include "bvh.hpp"
#include "object3d.hpp"
#include "triangle.hpp"
#include "utils.hpp"
#include "Vector2f.h"
#include "Vector3f.h"


class Mesh : public Object3D {

public:
    Mesh(const char *filename, Material *m);

    struct TriangleIndex {
        TriangleIndex(int r = -1) {
            x[0] = r; x[1] = r; x[2] = r;
        }
        int &operator[](const int i) { return x[i]; }
        // By Computer Graphics convention, counterclockwise winding is front face
        int x[3]{};
    };

    bool intersect(const Ray &r, Hit &h, float tmin) override;

private:
    std::vector<Triangle *> patches;
    BVH tree;
    // Normal can be used for light estimation
};

#endif
