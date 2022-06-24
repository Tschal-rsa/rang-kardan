#ifndef KDTREE_H
#define KDTREE_H

#include "image.hpp"
#include "utils.hpp"
#include <algorithm>
#include <iostream>

using namespace std;

class KDTreeNode {
public:
    Vector3f konta, makria;
    Pixel *pixel;
    KDTreeNode *lc, *rc;
    float maxSquaredRadius;
    KDTreeNode(): konta(1e100), makria(-1e100), lc(nullptr), rc(nullptr), maxSquaredRadius(0) {}
};

class KDTree {
public:
    KDTree(Image &image): alpha(0.7), size(image.Width() * image.Height()) {
        pixels = new Pixel*[size];
        for (int i = 0; i < size; ++i) {
            pixels[i] = image(i);
        }
    }
    void construct() {
        root = construct(0, size - 1, 0);
    }
    void destroy() {
        destroy(root);
    }
    void update(const Vector3f &position, const Vector3f &accumulate, const Vector3f &beamDirection) {
        update(root, position, accumulate, beamDirection);
    }
    ~KDTree() {
        if (root) {
            destroy(root);
        }
        if (pixels) {
            delete[] pixels;
        }
    }
protected:
    KDTreeNode* construct(int lo, int hi, int dim) {
        KDTreeNode *node = new KDTreeNode;
        for (int i = lo; i <= hi; ++i) {
            node->konta = Utils::min(node->konta, pixels[i]->hitPoint);
            node->makria = Utils::max(node->makria, pixels[i]->hitPoint);
            node->maxSquaredRadius = node->maxSquaredRadius < pixels[i]->squaredRadius ? pixels[i]->squaredRadius : node->maxSquaredRadius;
        }
        int mi = lo + hi >> 1;
        switch (dim) {
            case 0:
                nth_element(pixels + lo, pixels + mi, pixels + (hi + 1), [](Pixel *a, Pixel *b) {
                    return a->hitPoint.x() < b->hitPoint.x();
                });
                break;
            case 1:
                nth_element(pixels + lo, pixels + mi, pixels + (hi + 1), [](Pixel *a, Pixel *b) {
                    return a->hitPoint.y() < b->hitPoint.y();
                });
                break;
            default:
                nth_element(pixels + lo, pixels + mi, pixels + (hi + 1), [](Pixel *a, Pixel *b) {
                    return a->hitPoint.z() < b->hitPoint.z();
                });
                break;
        }
        node->pixel = pixels[mi];
        // cerr << node->pixel->hitPoint.x() << endl;
        node->lc = lo < mi ? construct(lo, mi - 1, (dim + 1) % 3) : nullptr;
        node->rc = mi < hi ? construct(mi + 1, hi, (dim + 1) % 3) : nullptr;
        return node;
    }
    void destroy(KDTreeNode *node) {
        if (node->lc) {
            destroy(node->lc);
        }
        if (node->rc) {
            destroy(node->rc);
        }
        delete node;
    }
    void update(KDTreeNode *node, const Vector3f &position, const Vector3f &accumulate, const Vector3f &beamDirection) {
        if (!node) {
            return;
        }
        Vector3f apoKonta(node->konta - position);
        Vector3f apoMakria(position - node->makria);
        // cerr << apoKonta.x() << "\t" << apoMakria.x() << endl;
        float squaredDistance = Utils::relu(apoKonta).squaredLength() + Utils::relu(apoMakria).squaredLength();
        // cerr << squaredDistance << endl;
        if (squaredDistance > node->maxSquaredRadius) {
            return;
        }
        if ((position - node->pixel->hitPoint).squaredLength() <= node->pixel->squaredRadius) {
            Pixel *pixel = node->pixel;
            float shrink = (pixel->numPhotons * alpha + alpha) / (pixel->numPhotons * alpha + 1);
            Vector3f reflectDirection(beamDirection - 2 * Vector3f::dot(beamDirection, pixel->normal) * pixel->normal);
            ++pixel->numPhotons;
            pixel->squaredRadius *= shrink;
            // cerr << shrink << "\t" << pixel->squaredRadius << endl;
            pixel->flux = (pixel->flux + pixel->accumulate * accumulate) * shrink;
        }
        if (node->lc) {
            update(node->lc, position, accumulate, beamDirection);
        }
        if (node->rc) {
            update(node->rc, position, accumulate, beamDirection);
        }
        node->maxSquaredRadius = node->pixel->squaredRadius;
        if (node->lc && node->maxSquaredRadius < node->lc->maxSquaredRadius) {
            node->maxSquaredRadius = node->lc->maxSquaredRadius;
        }
        if (node->rc && node->maxSquaredRadius < node->rc->maxSquaredRadius) {
            node->maxSquaredRadius = node->rc->maxSquaredRadius;
        }
    }
    KDTreeNode *root;
    Pixel **pixels;
    float alpha;
    int size;
};

#endif