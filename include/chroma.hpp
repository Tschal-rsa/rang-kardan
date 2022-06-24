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
#include "kdtree.hpp"
#include "light.hpp"
#include <omp.h>
#include "utils.hpp"

using namespace std;

class Chroma {
public:
    Chroma(SceneParser &sceneParser, Image &image): kdtree(image), camera(sceneParser.getCamera()), backgroundColor(sceneParser.getBackgroundColor()), baseGroup(sceneParser.getGroup()), image(image), threshold(10), tmin(1e-3) {}
    void photonTrace(Ray beam, Vector3f accumulate) {
        for (int depth = 0; depth < threshold; ++depth) {
            Hit hit;
            if (!baseGroup->intersect(beam, hit, tmin)) {
                return;
            }
            accumulate *= hit.getColor();
            Vector3f hitPoint = beam.pointAtParameter(hit.getT());
            float dotProduct = Vector3f::dot(beam.getDirection(), hit.getNormal());
            bool into = dotProduct < 0;
            Vector3f reflectDirection = beam.getDirection() - 2 * dotProduct * hit.getNormal();

            if (hit.getMaterial()->getSpecular() > 0) {
                beam.set(hitPoint, reflectDirection);
                continue;
            }

            if (hit.getMaterial()->getDiffuse() > 0) {
                kdtree.update(hitPoint, accumulate, beam.getDirection());
                Vector3f diffuseReflectDirection = Utils::sampleReflectedRay(hit.getNormal());
                // Absorb rate = 0.3
                if (Utils::randomEngine() <= 0.7) {
                    beam.set(hitPoint, diffuseReflectDirection);
                    continue;
                }
                return;
            }

            if (hit.getMaterial()->getRefract() > 0) {
                float refr = into ? hit.getMaterial()->getRefr() : 1 / hit.getMaterial()->getRefr();
                float incidentAngleCosine = -dotProduct, squaredRefractAngleCosine = 1 - (1 - Utils::square(incidentAngleCosine)) / Utils::square(refr);
                if (squaredRefractAngleCosine > 0) {
                    float refractAngleCosine = sqrt(squaredRefractAngleCosine);
                    // Schlick's approximation for Fresnel term.
                    float R0 = Utils::square((1 - refr) / (1 + refr));
                    float outerAngleCosine = into ? incidentAngleCosine : refractAngleCosine;
                    float R = R0 + (1 - R0) * pow(1 - outerAngleCosine, 5);
                    // R represents the reflect rate.
                    if (Utils::randomEngine() <= R) {
                        // Reflection
                        beam.set(hitPoint, reflectDirection);
                        continue;
                    } else {
                        // Refraction
                        Vector3f refractDirection = beam.getDirection() / refr + hit.getNormal() * (incidentAngleCosine / refr - refractAngleCosine);
                        beam.set(hitPoint, refractDirection);
                        continue;
                    }
                } else { // Total reflection
                    beam.set(hitPoint, reflectDirection);
                    continue;
                }
            }
        }
    }
    void rayTrace(Pixel &pixel, Ray ray, Vector3f accumulate) {
        for (int depth = 0; depth < threshold; ++depth) {
            Hit hit;
            if (!baseGroup->intersect(ray, hit, tmin)) {
                pixel.phos += pixel.accumulate * backgroundColor;
                return;
            }
            accumulate *= hit.getColor();
            Vector3f hitPoint = ray.pointAtParameter(hit.getT());
            float dotProduct = Vector3f::dot(ray.getDirection(), hit.getNormal());
            bool into = dotProduct < 0;
            Vector3f reflectDirection = ray.getDirection() - 2 * dotProduct * hit.getNormal();

            if (hit.getMaterial()->getSpecular() > 0) {
                ray.set(hitPoint, reflectDirection);
                continue;
            }

            if (hit.getMaterial()->getDiffuse() > 0) {
                pixel.hitPoint = hitPoint;
                pixel.accumulate = accumulate;
                pixel.phos += Utils::clamp(accumulate * hit.getMaterial()->getPhos());
                pixel.normal = hit.getNormal();
                return;
            }

            if (hit.getMaterial()->getRefract() > 0) {
                float refr = into ? hit.getMaterial()->getRefr() : 1 / hit.getMaterial()->getRefr();
                float incidentAngleCosine = -dotProduct, squaredRefractAngleCosine = 1 - (1 - Utils::square(incidentAngleCosine)) / Utils::square(refr);
                if (squaredRefractAngleCosine > 0) {
                    float refractAngleCosine = sqrt(squaredRefractAngleCosine);
                    // Schlick's approximation for Fresnel term.
                    float R0 = Utils::square((1 - refr) / (1 + refr));
                    float outerAngleCosine = into ? incidentAngleCosine : refractAngleCosine;
                    float R = R0 + (1 - R0) * pow(1 - outerAngleCosine, 5);
                    // R represents the reflect rate.
                    if (Utils::randomEngine() <= R) {
                        // Reflection
                        ray.set(hitPoint, reflectDirection);
                        continue;
                    } else {
                        // Refraction
                        Vector3f refractDirection = ray.getDirection() / refr + hit.getNormal() * (incidentAngleCosine / refr - refractAngleCosine);
                        ray.set(hitPoint, refractDirection);
                        continue;
                    }
                } else { // Total reflection
                    ray.set(hitPoint, reflectDirection);
                    continue;
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
        for (int epoch = 1; epoch <= epochs; ++epoch) {
            fprintf(stderr, "Round %d/%d\n", epoch, epochs);
            // Ray tracing pass
#pragma omp parallel for schedule(dynamic, 1)
            for (int x = 0; x < image.Width(); ++x) {
                for (int y = 0; y < image.Height(); ++y) {
                    Pixel &pixel = image(x, y);
                    Ray ray = camera->generateDistributedRay(Vector2f(x, y));
                    Vector3f accumulate(1);
                    rayTrace(pixel, ray, accumulate);
                }
            }
            // Photon tracing pass
            kdtree.construct();
#pragma omp parallel for schedule(dynamic, 1)
            for (int i = 0; i < numPhotons; ++i) {
                Vector3f color;
                Ray beam = generateBeam(color);
                photonTrace(beam, color);
            }
            kdtree.destroy();
            // Save checkpoint
            if (epoch % checkpoint == 0) {
                generateImage(epoch, numPhotons);
                char filename[100];
                sprintf(filename, "checkpoints/checkpoint-%d.bmp", epoch);
                image.SaveBMP(filename);
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
    int threshold;
    float tmin;
};

#endif