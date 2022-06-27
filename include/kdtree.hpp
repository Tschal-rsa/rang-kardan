#ifndef KDTREE_H
#define KDTREE_H

#include "image.hpp"
#include "utils.hpp"
#include "constant.hpp"
#include <algorithm>
#include <iostream>
#include <mutex>

using namespace std;

class KDTreeNode {
public:
    Vector3f konta, makria;
    KDTreeNode *lc, *rc;
    int lo, hi;
    float maxSquaredRadius;
    KDTreeNode(): konta(1e100), makria(-1e100), lc(nullptr), rc(nullptr), lo(-1), hi(-1), maxSquaredRadius(0) {}
};

class KDTree {
public:
    KDTree(Image &image): size(image.Width() * image.Height()) {
        pixels = new Pixel*[size];
        for (int i = 0; i < size; ++i) {
            pixels[i] = image(i);
        }
    }
    void construct() {
        root = construct(0, size);
    }
    void destroy() {
        for (int i = 0; i < size; ++i) {
            pixels[i]->update();
        }
        destroy(root);
        root = nullptr;
    }
    void update(const Vector3f &position, const Vector3f &accumulate) {
        update(root, position, accumulate);
    }
    ~KDTree() {
        if (root) {
            destroy(root);
        }
        if (pixels) {
            for (int i = 0; i < size; ++i) {
                pixels[i] = nullptr;
            }
            delete[] pixels;
        }
    }
protected:
    KDTreeNode* construct(int lo, int hi) {
        KDTreeNode *node = new KDTreeNode;
        for (int i = lo; i < hi; ++i) {
            node->konta = Utils::min(node->konta, pixels[i]->hitPoint);
            node->makria = Utils::max(node->makria, pixels[i]->hitPoint);
            node->maxSquaredRadius = node->maxSquaredRadius < pixels[i]->squaredRadius ? pixels[i]->squaredRadius : node->maxSquaredRadius;
        }
        if (hi - lo <= Constant::kdmax) {
            node->lo = lo;
            node->hi = hi;
            return node;
        }
        int mi = lo + hi >> 1;
        Vector3f scale = node->makria - node->konta;
        if (scale.x() > scale.y() && scale.x() > scale.z()) {
            nth_element(pixels + lo, pixels + mi, pixels + hi, [](Pixel *a, Pixel *b) {
                return a->hitPoint.x() < b->hitPoint.x();
            });
        } else if (scale.y() > scale.z()) {
            nth_element(pixels + lo, pixels + mi, pixels + hi, [](Pixel *a, Pixel *b) {
                return a->hitPoint.y() < b->hitPoint.y();
            });
        } else {
            nth_element(pixels + lo, pixels + mi, pixels + hi, [](Pixel *a, Pixel *b) {
                return a->hitPoint.z() < b->hitPoint.z();
            });
        }
        node->lc = construct(lo, mi);
        node->rc = construct(mi, hi);
        return node;
    }
    void destroy(KDTreeNode *node) {
        if (node->hi < 0) {
            destroy(node->lc);
            destroy(node->rc);
        }
        delete node;
    }
    void update(KDTreeNode *node, const Vector3f &position, const Vector3f &accumulate) {
        Vector3f apoKonta(node->konta - position);
        Vector3f apoMakria(position - node->makria);
        // cerr << apoKonta.x() << "\t" << apoMakria.x() << endl;
        float squaredDistance = Utils::relu(apoKonta).squaredLength() + Utils::relu(apoMakria).squaredLength();
        // cerr << squaredDistance << endl;
        if (squaredDistance > node->maxSquaredRadius) {
            return;
        }
        if (node->hi < 0) {
            update(node->lc, position, accumulate);
            update(node->rc, position, accumulate);
        } else {
            for (int i = node->lo; i < node->hi; ++i) {
                if ((position - pixels[i]->hitPoint).squaredLength() <= pixels[i]->squaredRadius) {
                    lock_guard<mutex> guard(kdlock);
                    ++pixels[i]->incPhotons;
                    // if (isnan(pixels[i]->flux.x())) {
                    //     cerr << "Flux NaN!" << endl;
                    //     cerr << pixels[i]->flux.x() << " " << pixels[i]->flux.y() << " " << pixels[i]->flux.z() << endl;
                    // }
                    pixels[i]->flux += pixels[i]->accumulate * accumulate;
                    // if (isnan(pixels[i]->flux.x())) {
                    //     cerr << "Flux NaN again!" << endl;
                    //     cerr << pixels[i]->accumulate.x() << " " << pixels[i]->accumulate.y() << " " << pixels[i]->accumulate.z() << endl;
                    //     cerr << accumulate.x() << " " << accumulate.y() << " " << accumulate.z() << endl;
                    //     cerr << pixels[i]->flux.x() << " " << pixels[i]->flux.y() << " " << pixels[i]->flux.z() << endl;
                    // }
                    // float shrink = (pixels[i]->numPhotons + 1) * Constant::sppmAlpha / (pixels[i]->numPhotons * Constant::sppmAlpha + 1);
                    // ++pixels[i]->numPhotons;
                    // pixels[i]->squaredRadius *= shrink;
                    // pixels[i]->flux = (pixels[i]->flux + pixels[i]->accumulate * accumulate) * shrink;
                }
            }
        }
    }
    KDTreeNode *root;
    Pixel **pixels;
    mutex kdlock;
    int size;
};

#endif