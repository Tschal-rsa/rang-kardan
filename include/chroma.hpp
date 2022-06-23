#ifndef CHROMA_H
#define CHROMA_H

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <string>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include <omp.h>
#include "utils.hpp"

using namespace std;

class Chroma {
public:
    Chroma(SceneParser *sceneParser): camera(sceneParser->getCamera()), baseGroup(sceneParser->getGroup()), image(sceneParser->getCamera()->getWidth(), sceneParser->getCamera()->getHeight()) {}
    Vector3f radiance(const Ray &r, int depth)
    {
        // float t;   // distance to intersection
        // int id = 0; // id of intersected object
        Hit hit;
        if (!baseGroup->intersect(r, hit, 1e-3))
            return Vector3f::ZERO;                // if miss, return black
        Material *material = hit.getMaterial();
        // const Sphere &obj = spheres[id]; // the hit object
        Vector3f x = r.pointAtParameter(hit.getT()), n = hit.getNormal(), nl = Vector3f::dot(n, r.getDirection()) < 0 ? n : -n, f = material->getDiffuseColor();
        float p = f.x() > f.y() && f.x() > f.z() ? f.x() : f.y() > f.z() ? f.y()
                                                            : f.z(); // max refl
        if (++depth > 5) {
            if (Utils::randomEngine() < p)
                f = f * (1 / p);
            else
                return material->getPhos(); // R.R.
        }
        if (material->getDiffuse() > 0)
        { // Ideal DIFFUSE reflection
            float r1 = 2 * M_PI * Utils::randomEngine(), r2 = Utils::randomEngine(), r2s = sqrt(r2);
            Vector3f w = nl, u = Vector3f::cross((fabs(w.x()) > .1 ? Vector3f::UP : Vector3f::RIGHT), w).normalized(), v = Vector3f::cross(w, u);
            Vector3f d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalized();
            return material->getPhos() + f * radiance(Ray(x, d), depth);
        }
        else if (material->getSpecular() > 0) // Ideal SPECULAR reflection
            return material->getPhos() + f * radiance(Ray(x, r.getDirection() - n * 2 * Vector3f::dot(n, r.getDirection())), depth);
        Ray reflRay(x, r.getDirection() - n * 2 * Vector3f::dot(n, r.getDirection())); // Ideal dielectric REFRACTION
        bool into = Vector3f::dot(n, nl) > 0;                // Ray from outside going in?
        float nc = 1, nt = material->getRefr(), nnt = into ? nc / nt : nt / nc, ddn = Vector3f::dot(r.getDirection(), nl), cos2t;
        if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0) // Total internal reflection
            return material->getPhos() + f * radiance(reflRay, depth);
        Vector3f tdir = (r.getDirection() * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).normalized();
        float a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : Vector3f::dot(tdir, n));
        float Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);
        return material->getPhos() + f * (depth > 2 ? (Utils::randomEngine() < P ? // Russian roulette
                                            radiance(reflRay, depth) * RP
                                                        : radiance(Ray(x, tdir), depth) * TP)
                                        : radiance(reflRay, depth) * Re + radiance(Ray(x, tdir), depth) * Tr);
    }
    void render() {
        int w = camera->getWidth(), h = camera->getHeight(), samps = 50; // # samples
        Ray cam(Vector3f(50, 40.8, 295.6), Vector3f(0, 0, -1).normalized());        // cam pos, dir
        Vector3f cx = Vector3f(w * .5135 / h, 0, 0), cy = Vector3f::cross(cx, cam.getDirection()).normalized() * .5135, r, *c = new Vector3f[w * h];
#pragma omp parallel for schedule(dynamic, 1) private(r) // OpenMP
        for (int y = 0; y < h; y++)
        { // Loop over image rows
            fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps * 4, 100. * y / (h - 1));
            for (unsigned short x = 0; x < w; x++) // Loop cols
            {
                int i = (h - y - 1) * w + x;
                for (int sy = 0; sy < 2; sy++)       // 2x2 subpixel rows
                    for (int sx = 0; sx < 2; sx++, r = Vector3f::ZERO)
                    { // 2x2 subpixel cols
                        for (int s = 0; s < samps; s++)
                        {
                            // float r1 = 2 * Utils::randomEngine(), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                            // float r2 = 2 * Utils::randomEngine(), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                            // Vector3f d = cx * (((sx + .5 + dx) / 2 + x) / w - .5) +
                            //         cy * (((sy + .5 + dy) / 2 + y) / h - .5) + cam.getDirection();
                            // r += radiance(Ray(cam.getOrigin() + d * 140, d.normalized()), 0);
                            r += radiance(camera->generateAverageRay(Vector2f(x, y)), 0);
                        } // Camera rays are pushed ^^^^^ forward to start in interior
                        c[i] += Utils::clamp(r / samps);
                    }
                image.SetPixel(x, y, Utils::gammaCorrect(c[i] * 0.25));
            }
        }
        delete[] c;
    }
    Image* getImage() {
        return &image;
    }
    ~Chroma() {
        camera = nullptr;
        baseGroup = nullptr;
    }
protected:
    Camera *camera;
    Group *baseGroup;
    Image image;
};

#endif