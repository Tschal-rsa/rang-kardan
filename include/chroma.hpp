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
        pixel.phos += pixel.accumulate * backgroundColor;
        pixel.hitPoint = ray.pointAtParameter(1e100);
    }
    void generateImage(int epoch) {
        for (int x = 0; x < image.Width(); ++x) {
            for (int y = 0; y < image.Height(); ++y) {
                Pixel &pixel = image(x, y);
                Vector3f color = (pixel.flux / (M_PI * pixel.squaredRadius * Constant::numPhotons) + pixel.phos) / epoch;
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
    void render(int epochs, int checkpoint, bool savePixels = true, int lastEpoch = 0) {
        clock_t apocalypse = clock();
        if (lastEpoch > 0) {
            char filename[100];
            sprintf(filename, "checkpoints/checkpoint-%d.pxl", lastEpoch);
            image.readPixels(filename);
        }
        for (int epoch = lastEpoch + 1; epoch <= epochs; ++epoch) {
            fprintf(stderr, "Round %d/%d\n", epoch, epochs);
            // Ray tracing pass
            fprintf(stderr, "\rRay tracing pass begin");
#pragma omp parallel for schedule(dynamic, 1)
            for (int x = 0; x < image.Width(); ++x) {
                for (int y = 0; y < image.Height(); ++y) {
                    Pixel &pixel = image(x, y);
                    Ray ray = camera->generateDistributedRay(Vector2f(x, y));
                    rayTrace(pixel, ray);
                }
            }
            fprintf(stderr, "\rRay tracing pass finish\n");
            // Photon tracing pass
            fprintf(stderr, "\rPhoton tracing pass begin");
            kdtree.construct();
#pragma omp parallel for schedule(dynamic, 1)
            for (int i = 0; i < Constant::numPhotons; ++i) {
                Vector3f color;
                Ray beam = generateBeam(color);
                photonTrace(beam, color);
            }
            kdtree.destroy();
            fprintf(stderr, "\rPhoton tracing pass finish\n");
            // Save checkpoint
            if (epoch % checkpoint == 0) {
                generateImage(epoch);
                char filename[100];
                sprintf(filename, "checkpoints/checkpoint-%d.bmp", epoch);
                image.SaveBMP(filename);
                if (savePixels) {
                    sprintf(filename, "checkpoints/checkpoint-%d.pxl", epoch);
                    image.SavePixels(filename);
                }
                fprintf(stderr, "Total time: %.3fs\n", float(clock() - apocalypse) / CLOCKS_PER_SEC);
            }
        }
        generateImage(epochs);
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