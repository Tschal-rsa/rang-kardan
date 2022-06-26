#ifndef BVH_H
#define BVH_H

#include "triangle.hpp"
#include "utils.hpp"
#include "constant.hpp"
#include <algorithm>
#include <vector>

using namespace std;

class BVHNode {
public:
    Vector3f konta, makria;
    BVHNode *lc, *rc;
    int lo, hi;
    BVHNode(): konta(1e100), makria(-1e100), lc(nullptr), rc(nullptr), lo(-1), hi(-1) {}
    bool intersect(const Ray &ray) {
        Vector3f origin = ray.getOrigin(), direction = ray.getDirection();
        Vector3f tKonta = (konta - origin) / direction, tMakria = (makria - origin) / direction;
        float tEnter = Utils::max(Utils::min(tKonta, tMakria)), tExit = Utils::min(Utils::max(tKonta, tMakria));
        return tEnter < tExit && tExit >= 0;
    }
};

class BVH {
public:
    BVH(): root(nullptr), tria(nullptr) {}
    void construct(vector<Triangle *> &patches) {
        size = patches.size();
        tria = new Triangle*[size];
        for (int i = 0; i < size; ++i) {
            tria[i] = patches[i];
        }
        root = construct(0, size);
    }
    bool intersect(const Ray &ray, Hit &hit, float tmin) {
        return intersect(root, ray, hit, tmin);
    }
    ~BVH() {
        if (root) {
            destroy(root);
        }
        if (tria) {
            for (int i = 0; i < size; ++i) {
                tria[i] = nullptr;
            }
            delete[] tria;
        }
    }
protected:
    BVHNode *root;
    Triangle **tria;
    int size;
    BVHNode* construct(int lo, int hi) {
        BVHNode *node = new BVHNode;
        for (int i = lo; i < hi; ++i) {
            node->konta = Utils::min(node->konta, tria[i]->konta);
            node->makria = Utils::max(node->makria, tria[i]->makria);
        }
        if (hi - lo <= Constant::bvhmax) {
            node->lo = lo;
            node->hi = hi;
            return node;
        }
        int mi = lo + hi >> 1;
        Vector3f scale = node->makria - node->konta;
        if (scale.x() > scale.y() && scale.x() > scale.z()) {
            nth_element(tria + lo, tria + mi, tria + hi, [](Triangle *a, Triangle *b) {
                return a->getCenter().x() < b->getCenter().x();
            });
        } else if (scale.y() > scale.z()) {
            nth_element(tria + lo, tria + mi, tria + hi, [](Triangle *a, Triangle *b) {
                return a->getCenter().y() < b->getCenter().y();
            });
        } else {
            nth_element(tria + lo, tria + mi, tria + hi, [](Triangle *a, Triangle *b) {
                return a->getCenter().z() < b->getCenter().z();
            });
        }
        node->lc = construct(lo, mi);
        node->rc = construct(mi, hi);
        return node;
    }
    void destroy(BVHNode *node) {
        if (node->hi < 0) {
            destroy(node->lc);
            destroy(node->rc);
        }
        delete node;
    }
    bool intersect(BVHNode *node, const Ray &ray, Hit &hit, float tmin) {
        if (!node->intersect(ray)) {
            return false;
        }
        bool isIntersect = false;
        if (node->hi < 0) {
            isIntersect = intersect(node->lc, ray, hit, tmin) || isIntersect;
            isIntersect = intersect(node->rc, ray, hit, tmin) || isIntersect;
        } else {
            for (int i = node->lo; i < node->hi; ++i) {
                isIntersect = tria[i]->intersect(ray, hit, tmin) || isIntersect;
            }
        }
        return isIntersect;
    }
};

#endif