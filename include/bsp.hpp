#ifndef BSP_H
#define BSP_H

#include "ray.hpp"
#include "hit.hpp"
#include "utils.hpp"
#include "triangle.hpp"
#include <vector>
#include <map>
#include <iostream>

using namespace std;

class BSPNode {
public:
    vector<Triangle *> patches;
    Vector3f konta, makria;
    BSPNode *lc, *rc;
    BSPNode(const Vector3f &konta, const Vector3f &makria): patches(0), konta(konta), makria(makria), lc(nullptr), rc(nullptr) {} 
    bool contain(Triangle *tria) {
        Vector3f mikro(tria->konta), megalo(tria->makria);
        return (mikro.x() < makria.x() || (mikro.x() == makria.x() && mikro.x() == megalo.x())) 
        && (megalo.x() > konta.x() || (megalo.x() == konta.x() && mikro.x() == megalo.x())) 
        && (mikro.y() < makria.y() || (mikro.y() == makria.y() && mikro.y() == megalo.y())) 
        && (megalo.y() > konta.y() || (megalo.y() == konta.y() && mikro.y() == megalo.y())) 
        && (mikro.z() < makria.z() || (mikro.z() == makria.z() && mikro.z() == megalo.z())) 
        && (megalo.z() > konta.z() || (megalo.z() == konta.z() && mikro.z() == megalo.z()));
    }
};

class BSP {
public:
    BSP(): root(nullptr), maxPatches(8), threshold(24) {}
    void construct(vector<Triangle *> patches) {
        Vector3f konta(1e100), makria(-1e100);
        for (auto tria: patches) {
            konta = Utils::min(konta, tria->konta);
            makria = Utils::max(makria, tria->makria);
        }
        root = construct(0, 0, patches, konta, makria);
    }
    bool intersect(const Ray &ray, Hit &hit, float tmin) {
        return intersect(root, ray, hit, tmin);
    }
    ~BSP() {
        if (root) {
            delete root;
        }
    }
protected:
    BSPNode *root;
    int maxPatches, threshold;
    BSPNode* construct(int depth, int dim, vector<Triangle *> &patches, Vector3f konta, Vector3f makria) {
        BSPNode *node = new BSPNode(konta, makria);
        Vector3f aristeraMakria, dexiaKonta;
        switch (dim) {
            case 0:
                aristeraMakria = Vector3f((node->konta.x() + node->makria.x()) / 2, node->makria.y(), node->makria.z());
                dexiaKonta = Vector3f((node->konta.x() + node->makria.x()) / 2, node->konta.y(), node->konta.z());
                break;
            case 1:
                aristeraMakria = Vector3f(node->makria.x(), (node->konta.y() + node->makria.y()) / 2, node->makria.z());
                dexiaKonta = Vector3f(node->konta.x(), (node->konta.y() + node->makria.y()) / 2, node->konta.z());
                break;
            default:
                aristeraMakria = Vector3f(node->makria.x(), node->makria.y(), (node->konta.z() + node->makria.z()) / 2);
                dexiaKonta = Vector3f(node->konta.x(), node->konta.y(), (node->konta.z() + node->makria.z()) / 2);
                break;
        }
        for (auto tria: patches) {
            if (node->contain(tria)) {
                node->patches.emplace_back(tria);
            }
        }
        if ((int)node->patches.size() > maxPatches && depth < threshold) {
            node->lc = construct(depth + 1, (dim + 1) % 3, node->patches, konta, aristeraMakria);
            node->rc = construct(depth + 1, (dim + 1) % 3, node->patches, dexiaKonta, makria);
            vector<Triangle *> aristeraPatches(node->lc->patches), dexiaPatches(node->rc->patches);
            map<Triangle*, int> multiplicity;
            for (auto tria: aristeraPatches) {
                ++multiplicity[tria];
            }
            for (auto tria: dexiaPatches) {
                ++multiplicity[tria];
            }
            node->lc->patches.clear();
            node->rc->patches.clear();
            node->patches.clear();
            for (auto tria: aristeraPatches) {
                if (multiplicity[tria] > 1) {
                    node->patches.emplace_back(tria);
                } else {
                    node->lc->patches.emplace_back(tria);
                }
            }
            for (auto tria: dexiaPatches) {
                if (multiplicity[tria] <= 1) {
                    node->rc->patches.emplace_back(tria);
                }
            }
        }
        return node;
    }
    float AABBIntersect(BSPNode *node, const Ray &ray) {
        Vector3f origin(ray.getOrigin()), direction(ray.getDirection());
        if (node->konta <= origin && origin <= node->makria) {
            return -1e100;
        }
        float t = -1e100;
        if (direction.x() != 0) {
            t = max(t, min((node->konta.x() - origin.x()) / direction.x(), (node->makria.x() - origin.x()) / direction.x()));
        }
        if (direction.y() != 0) {
            t = max(t, min((node->konta.y() - origin.y()) / direction.y(), (node->makria.y() - origin.y()) / direction.y()));
        }
        if (direction.z() != 0) {
            t = max(t, min((node->konta.z() - origin.z()) / direction.z(), (node->makria.z() - origin.z()) / direction.z()));
        }
        if (t < -1e-6) {
            return 1e100;
        }
        Vector3f point(ray.pointAtParameter(t));
        return (node->konta <= point && point && node->makria) ? t : 1e100;
    }
    bool intersect(BSPNode *node, const Ray &ray, Hit &hit, float tmin) {
        bool isIntersect = false;
        for (auto tria: node->patches) {
            isIntersect = isIntersect || tria->intersect(ray, hit, tmin);
        }
        float aristeraT = node->lc ? AABBIntersect(node->lc, ray) : 1e100;
        float dexiaT = node->lc ? AABBIntersect(node->rc, ray) : 1e100;
        if (aristeraT < dexiaT) {
            if (hit.getT() <= aristeraT) {
                return isIntersect;
            }
            if (node->lc) {
                isIntersect = isIntersect || intersect(node->lc, ray, hit, tmin);
            }
            if (hit.getT() <= dexiaT) {
                return isIntersect;
            }
            if (node->rc) {
                isIntersect = isIntersect || intersect(node->rc, ray, hit, tmin);
            }
        } else {
            if (hit.getT() <= dexiaT) {
                return isIntersect;
            }
            if (node->rc) {
                isIntersect = isIntersect || intersect(node->rc, ray, hit, tmin);
            }
            if (hit.getT() <= aristeraT) {
                return isIntersect;
            }
            if (node->lc) {
                isIntersect = isIntersect || intersect(node->lc, ray, hit, tmin);
            }
        }
        return isIntersect;
    }
};

#endif