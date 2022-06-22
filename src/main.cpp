#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <omp.h>
#include <random>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"

#include <string>

using namespace std;

enum Refl_t
{
    DIFF,
    SPEC,
    REFR
}; // material types, used in radiance()
struct Sphere
{
    float rad;  // radius
    Vector3f p, e, c; // position, emission, color
    Refl_t refl; // reflection type (DIFFuse, SPECular, REFRactive)
    Sphere(float rad_, Vector3f p_, Vector3f e_, Vector3f c_, Refl_t refl_) : rad(rad_), p(p_), e(e_), c(c_), refl(refl_) {}
    float intersect(const Ray &r) const
    {                     // returns distance, 0 if nohit
        Vector3f rayToCenter = p - r.getOrigin();
        float squaredLengthToCenter = rayToCenter.squaredLength();
        float squaredRadius = rad*rad;
        // bool outer = rayToCenter.squaredLength() > squaredRadius;
        float disToFoot = Vector3f::dot(r.getDirection(), rayToCenter);
        if (squaredLengthToCenter >= squaredRadius && disToFoot <= 0) {
            return 0;
        }
        float squaredDisToCenter = squaredLengthToCenter - disToFoot * disToFoot;
        if (squaredDisToCenter > squaredRadius) {
            return false;
        }
        float disToCross = sqrt(squaredRadius - squaredDisToCenter);
        float t = squaredLengthToCenter > squaredRadius ? disToFoot - disToCross : disToFoot + disToCross;
        if (t < 1e-4) {
            return 0;
        }
        return t;
    }
};
Sphere spheres[] = {
    // Scene: radius, position, emission, color, material
    Sphere(5e3, Vector3f(5e3 + 1, 40.8, 81.6), Vector3f::ZERO, Vector3f(.75, .25, .25), DIFF),   // Left
    Sphere(5e3, Vector3f(-5e3 + 99, 40.8, 81.6), Vector3f::ZERO, Vector3f(.25, .25, .75), DIFF), // Rght
    Sphere(5e3, Vector3f(50, 40.8, 5e3), Vector3f::ZERO, Vector3f(.75, .75, .75), DIFF),         // Back
    Sphere(5e3, Vector3f(50, 40.8, -5e3 + 170), Vector3f::ZERO, Vector3f::ZERO, DIFF),               // Frnt
    Sphere(5e3, Vector3f(50, 5e3, 81.6), Vector3f::ZERO, Vector3f(.75, .75, .75), DIFF),         // Botm
    Sphere(5e3, Vector3f(50, -5e3 + 81.6, 81.6), Vector3f::ZERO, Vector3f(.75, .75, .75), DIFF), // Top
    Sphere(16.5, Vector3f(27, 16.5, 47), Vector3f::ZERO, Vector3f(1, 1, 1) * .999, SPEC),        // Mirr
    Sphere(16.5, Vector3f(73, 16.5, 78), Vector3f::ZERO, Vector3f(1, 1, 1) * .999, REFR),        // Glas
    Sphere(600, Vector3f(50, 681.6 - .27, 81.6), Vector3f(12, 12, 12), Vector3f::ZERO, DIFF)     // Lite
};
inline float clamp(float x) { 
    return x < 0 ? 0 : (x > 1 ? 1 : x);
}
Vector3f clamp(Vector3f vec) {
    return Vector3f(clamp(vec.x()), clamp(vec.y()), clamp(vec.z()));
}
inline float gammaCorrect(float x) { 
    return pow(x, 1 / 2.2);
}
Vector3f gammaCorrect(Vector3f vec) {
    return Vector3f(gammaCorrect(vec.x()), gammaCorrect(vec.y()), gammaCorrect(vec.z()));
}
float randomEngine() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<float> u(0.0, 1.0);
    return u(rng);
}
inline bool intersect(const Ray &r, float &t, int &id)
{
    float n = sizeof(spheres) / sizeof(Sphere), d, inf = t = 1e20;
    for (int i = int(n); i--;)
        if ((d = spheres[i].intersect(r)) && d < t)
        {
            t = d;
            id = i;
        }
    return t < inf;
}
Vector3f radiance(const Ray &r, int depth)
{
    float t;   // distance to intersection
    int id = 0; // id of intersected object
    if (!intersect(r, t, id))
        return Vector3f::ZERO;                // if miss, return black
    const Sphere &obj = spheres[id]; // the hit object
    Vector3f x = r.pointAtParameter(t), n = (x - obj.p).normalized(), nl = Vector3f::dot(n, r.getDirection()) < 0 ? n : -n, f = obj.c;
    float p = f.x() > f.y() && f.x() > f.z() ? f.x() : f.y() > f.z() ? f.y()
                                                        : f.z(); // max refl
    if (++depth > 5)
        if (randomEngine() < p)
            f = f * (1 / p);
        else
            return obj.e; // R.R.
    if (obj.refl == DIFF)
    { // Ideal DIFFUSE reflection
        float r1 = 2 * M_PI * randomEngine(), r2 = randomEngine(), r2s = sqrt(r2);
        Vector3f w = nl, u = Vector3f::cross((fabs(w.x()) > .1 ? Vector3f::UP : Vector3f::RIGHT), w).normalized(), v = Vector3f::cross(w, u);
        Vector3f d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalized();
        return obj.e + f * radiance(Ray(x, d), depth);
    }
    else if (obj.refl == SPEC) // Ideal SPECULAR reflection
        return obj.e + f * radiance(Ray(x, r.getDirection() - n * 2 * Vector3f::dot(n, r.getDirection())), depth);
    Ray reflRay(x, r.getDirection() - n * 2 * Vector3f::dot(n, r.getDirection())); // Ideal dielectric REFRACTION
    bool into = Vector3f::dot(n, nl) > 0;                // Ray from outside going in?
    float nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = Vector3f::dot(r.getDirection(), nl), cos2t;
    if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0) // Total internal reflection
        return obj.e + f * radiance(reflRay, depth);
    Vector3f tdir = (r.getDirection() * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).normalized();
    float a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : Vector3f::dot(tdir, n));
    float Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);
    return obj.e + f * (depth > 2 ? (randomEngine() < P ? // Russian roulette
                                           radiance(reflRay, depth) * RP
                                                       : radiance(Ray(x, tdir), depth) * TP)
                                    : radiance(reflRay, depth) * Re + radiance(Ray(x, tdir), depth) * Tr);
}

int main(int argc, char *argv[]) {
    for (int argNum = 1; argNum < argc; ++argNum) {
        std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    }

    if (argc != 3) {
        cout << "Usage: ./bin/PA1 <input scene file> <output bmp file>" << endl;
        return 1;
    }
    string inputFile = argv[1];
    string outputFile = argv[2];  // only bmp is allowed.

    // TODO: Main RayCasting Logic
    // First, parse the scene using SceneParser.
    // Then loop over each pixel in the image, shooting a ray
    // through that pixel and finding its intersection with
    // the scene.  Write the color at the intersection to that
    // pixel in your output image.
    SceneParser sceneParser(inputFile.c_str());
    Camera *camera = sceneParser.getCamera();
    Group *baseGroup = sceneParser.getGroup();
    Image image(camera->getWidth(), camera->getHeight());
    // for (int x = 0; x < camera->getWidth(); ++x) {
    //     for (int y = 0; y < camera->getHeight(); ++y) {
    //         Ray camRay = camera->generateRay(Vector2f(x, y));
    //         Hit hit;
    //         bool isIntersect = baseGroup->intersect(camRay, hit, 0);
    //         if (isIntersect) {
    //             Vector3f finalColor = Vector3f::ZERO;
    //             for (int li = 0; li < sceneParser.getNumLights(); ++li) {
    //                 Light *light = sceneParser.getLight(li);
    //                 Vector3f L, lightColor;
    //                 light->getIllumination(camRay.pointAtParameter(hit.getT()), L, lightColor);
    //                 finalColor += hit.getMaterial()->Shade(camRay, hit, L, lightColor);
    //             }
    //             image.SetPixel(x, y, finalColor);
    //         } else {
    //             image.SetPixel(x, y, sceneParser.getBackgroundColor());
    //         }
    //     }
    // }
    int w = 512, h = 384, samps = 10; // # samples
    Ray cam(Vector3f(50, 50, 295.6), Vector3f(0, -0.042612, -1).normalized());        // cam pos, dir
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
                        float r1 = 2 * randomEngine(), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                        float r2 = 2 * randomEngine(), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                        Vector3f d = cx * (((sx + .5 + dx) / 2 + x) / w - .5) +
                                cy * (((sy + .5 + dy) / 2 + y) / h - .5) + cam.getDirection();
                        r += radiance(Ray(cam.getOrigin() + d * 140, d.normalized()), 0);
                    } // Camera rays are pushed ^^^^^ forward to start in interior
                    c[i] += clamp(r / samps);
                }
            image.SetPixel(x, y, gammaCorrect(c[i] * 0.25));
        }
    }
    image.SaveBMP(outputFile.c_str());
    cout << "Hello! Computer Graphics!" << endl;
    return 0;
}

