#ifndef GROUP_H
#define GROUP_H

#include "bvh.hpp"
#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include "constant.hpp"
#include <iostream>
#include <vector>


// TODO: Implement Group - add data structure to store a list of Object*
class Group : public Object3D {

public:

    Group(): objects(0), illuminants(0), tree() {}

    explicit Group (int num_objects): objects(0), illuminants(0), tree() {}

    ~Group() override {}

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        bool groupIntersect = tree.intersect(r, h, tmin);
        // bool groupIntersect = false;
        // for (Object3D* obj : objects) {
        //     groupIntersect = obj->intersect(r, h, tmin) || groupIntersect;
        // }
        for (Object3D* obj : uncensored) {
            groupIntersect = obj->intersect(r, h, tmin) || groupIntersect;
        }
        return groupIntersect;
    }

    void addObject(int index, Object3D *obj) {
        if (obj->getMaterial() && obj->getMaterial()->getPhos() != Vector3f::ZERO) {
            illuminants.push_back(obj);
        }
        if (obj->bounded()) {
            objects.push_back(obj);
        } else {
            uncensored.push_back(obj);
        }
    }

    void activate() {
        tree.construct(objects);
    }

    Ray generateBeam(Vector3f &color, int idx) {
        Object3D *illuminant = illuminants[idx];
        color = illuminant->getMaterial()->getPhos() * Constant::strongPhos;
        return illuminant->generateBeam();
    }

    int getGroupSize() {
        return objects.size() + uncensored.size();
    }

    int getIlluminantSize() {
        return illuminants.size();
    }

private:
    std::vector<Object3D*> objects, illuminants, uncensored;
    BVH tree;
};

#endif
	
