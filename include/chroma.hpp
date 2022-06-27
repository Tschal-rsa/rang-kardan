#ifndef CHROMA_H
#define CHROMA_H

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <iostream>
#include <string>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "kdtree.hpp"
#include "light.hpp"
#include <omp.h>
#include "constant.hpp"
#include "utils.hpp"

using namespace std;

class Chroma {
public:
    Chroma(SceneParser &sceneParser, Image &image): kdtree(image), camera(sceneParser.getCamera()), backgroundColor(sceneParser.getBackgroundColor()), baseGroup(sceneParser.getGroup()), image(image) {
        baseGroup->activate();
    }
    Vector3f radiance(const Ray &r, int depth)
    {
        // float t;   // distance to intersection
        // int id = 0; // id of intersected object
        Hit hit;
        if (!baseGroup->intersect(r, hit, 1e-2))
            return Vector3f::ZERO;                // if miss, return black
        Material *material = hit.getMaterial();
        // const Sphere &obj = spheres[id]; // the hit object
        Vector3f x = r.pointAtParameter(hit.getT()), n = hit.getNormal(), nl = Vector3f::dot(n, r.getDirection()) < 0 ? n : -n, f = material->getDiffuseColor();
        float p = Utils::max(f); // max refl
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
        int w = camera->getWidth(), h = camera->getHeight(), samps = 10; // # samples
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
                            r += radiance(camera->generateDistributedRay(Vector2f(x, y)), 0);
                        } // Camera rays are pushed ^^^^^ forward to start in interior
                        c[i] += Utils::clamp(r / samps);
                    }
                image.SetPixel(x, y, Utils::gammaCorrect(c[i] * 0.25));
            }
        }
        delete[] c;
    }
    void photonTrace(Ray beam, Vector3f accumulate) {
        for (int depth = 0; depth < Constant::traceThreshold; ++depth) {
            if (depth > Constant::russianRoulette) {
                float maxAccumulate = Utils::max(accumulate);
                if (Utils::randomEngine() < maxAccumulate) {
                    accumulate *= (1 / maxAccumulate);
                } else {
                    return;
                }
            }
            Hit hit;
            if (!baseGroup->intersect(beam, hit, Constant::tmin)) {
                return;
            }
            accumulate *= hit.getColor();
            float dotProduct = Vector3f::dot(beam.getDirection(), hit.getNormal());
            bool into = dotProduct < 0;
            Vector3f hitPoint = beam.pointAtParameter(hit.getT());

            float erabu = Utils::randomEngine();
            float genkai = hit.getMaterial()->getDiffuse();
            if (erabu < genkai) {
                kdtree.update(hitPoint, accumulate);
                Vector3f diffuseReflectDirection = Utils::sampleReflectedRay((into ? 1 : -1) * hit.getNormal());
                beam.set(hitPoint, diffuseReflectDirection);
                continue;
            }

            Vector3f reflectDirection = beam.getDirection() - 2 * dotProduct * hit.getNormal();

            genkai += hit.getMaterial()->getSpecular();
            if (erabu < genkai) {
                beam.set(hitPoint, reflectDirection);
                continue;
            }

            genkai += hit.getMaterial()->getRefract();
            if (erabu < genkai) {
                float refr = into ? hit.getMaterial()->getRefr() : 1 / hit.getMaterial()->getRefr();
                float incidentAngleCosine = into ? -dotProduct : dotProduct, squaredRefractAngleCosine = 1 - (1 - Utils::square(incidentAngleCosine)) / Utils::square(refr);
                if (squaredRefractAngleCosine > 0) {
                    float refractAngleCosine = sqrt(squaredRefractAngleCosine);
                    // Schlick's approximation for Fresnel term.
                    float R0 = Utils::square((refr - 1) / (refr + 1));
                    float outerAngleCosine = into ? incidentAngleCosine : refractAngleCosine;
                    float R = R0 + (1 - R0) * pow(1 - outerAngleCosine, 5);
                    // R represents the reflect rate.
                    if (Utils::randomEngine() <= R) {
                        // Reflection
                        beam.set(hitPoint, reflectDirection);
                    } else {
                        // Refraction
                        Vector3f refractDirection = (beam.getDirection() / refr + hit.getNormal() * ((into ? 1 : -1) * (incidentAngleCosine / refr - refractAngleCosine))).normalized();
                        beam.set(hitPoint, refractDirection);
                    }
                } else { // Total reflection
                    beam.set(hitPoint, reflectDirection);
                }
            }
        }
    }
    void rayTrace(Pixel &pixel, Ray ray) {
        Vector3f accumulate(1);
        for (int depth = 0; depth < Constant::traceThreshold; ++depth) {
            Hit hit;
            if (!baseGroup->intersect(ray, hit, Constant::tmin)) {
                pixel.phos += pixel.accumulate * backgroundColor;
                pixel.hitPoint = ray.pointAtParameter(1e100);
                return;
            }
            Vector3f hitPoint = ray.pointAtParameter(hit.getT());

            float erabu = Utils::randomEngine();
            float genkai = hit.getMaterial()->getDiffuse();
            if (erabu < genkai) {
                pixel.hitPoint = hitPoint;
                pixel.accumulate = accumulate;
                pixel.phos += Utils::clamp(accumulate * hit.getMaterial()->getPhos());
                pixel.normal = hit.getNormal();
                return;
            }

            float dotProduct = Vector3f::dot(ray.getDirection(), hit.getNormal());
            Vector3f reflectDirection = ray.getDirection() - 2 * dotProduct * hit.getNormal();

            accumulate *= hit.getColor();
            genkai += hit.getMaterial()->getSpecular();
            if (erabu < genkai) {
                ray.set(hitPoint, reflectDirection);
                continue;
            }

            genkai += hit.getMaterial()->getRefract();
            if (erabu < genkai) {
                bool into = dotProduct < 0;
                float refr = into ? hit.getMaterial()->getRefr() : 1 / hit.getMaterial()->getRefr();
                float incidentAngleCosine = into ? -dotProduct : dotProduct, squaredRefractAngleCosine = 1 - (1 - Utils::square(incidentAngleCosine)) / Utils::square(refr);
                if (squaredRefractAngleCosine > 0) {
                    float refractAngleCosine = sqrt(squaredRefractAngleCosine);
                    // Schlick's approximation for Fresnel term.
                    float R0 = Utils::square((refr - 1) / (refr + 1));
                    float outerAngleCosine = into ? incidentAngleCosine : refractAngleCosine;
                    float R = R0 + (1 - R0) * pow(1 - outerAngleCosine, 5);
                    // R represents the reflect rate.
                    if (Utils::randomEngine() <= R) {
                        // Reflection
                        ray.set(hitPoint, reflectDirection);
                    } else {
                        // Refraction
                        Vector3f refractDirection = (ray.getDirection() / refr + hit.getNormal() * ((into ? 1 : -1) * (incidentAngleCosine / refr - refractAngleCosine))).normalized();
                        ray.set(hitPoint, refractDirection);
                    }
                } else { // Total reflection
                    ray.set(hitPoint, reflectDirection);
                }
            }
        }
    }
    void generateImage(int epoch, int numPhotons) {
        for (int x = 0; x < image.Width(); ++x) {
            for (int y = 0; y < image.Height(); ++y) {
                Pixel &pixel = image(x, y);
                Vector3f color = (pixel.flux / (M_PI * pixel.squaredRadius * numPhotons) + pixel.phos) / epoch;
                pixel.color = Utils::clamp(Utils::gammaCorrect(color));
            }
        }
    }
    Ray generateBeam(Vector3f &color) {
        static int counter = 0;
        // Select an illuminant to generate a beam.
        counter = (counter + 1) % baseGroup->getIlluminantSize();
        return baseGroup->generateBeam(color, counter);
    }
    void render(int epochs, int numPhotons, int checkpoint) {
        clock_t apocalypse = clock();
        for (int epoch = 1; epoch <= epochs; ++epoch) {
            fprintf(stderr, "Round %d/%d\n", epoch, epochs);
            // Ray tracing pass
#pragma omp parallel for schedule(dynamic, 1)
            for (int x = 0; x < image.Width(); ++x) {
                fprintf(stderr, "\rRay tracing pass %d%%", x * 100 / image.Width());
                for (int y = 0; y < image.Height(); ++y) {
                    Pixel &pixel = image(x, y);
                    Ray ray = camera->generateDistributedRay(Vector2f(x, y));
                    rayTrace(pixel, ray);
                }
            }
            fprintf(stderr, "\rRay tracing pass 100%%\n");
            // Photon tracing pass
            kdtree.construct();
#pragma omp parallel for schedule(dynamic, 1)
            for (int i = 0; i < numPhotons; ++i) {
                if ((i & 0x3fff) == 0) {
                    fprintf(stderr, "\rPhoton tracing pass %d%%", i * 100 / numPhotons);
                }
                Vector3f color;
                Ray beam = generateBeam(color);
                photonTrace(beam, color);
            }
            kdtree.destroy();
            fprintf(stderr, "\rPhoton tracing pass 100%%\n");
            // Save checkpoint
            if (epoch % checkpoint == 0) {
                generateImage(epoch, numPhotons);
                char filename[100];
                sprintf(filename, "checkpoints/checkpoint-%d.bmp", epoch);
                image.SaveBMP(filename);
                fprintf(stderr, "Total time: %.3fs\n", float(clock() - apocalypse) / CLOCKS_PER_SEC);
            }
        }
        generateImage(epochs, numPhotons);
    }
    Image* getImage() {
        return &image;
    }
    ~Chroma() {
        camera = nullptr;
        baseGroup = nullptr;
    }
protected:
    KDTree kdtree;
    Camera *camera;
    Vector3f backgroundColor;
    Group *baseGroup;
    Image &image;
};

#endif