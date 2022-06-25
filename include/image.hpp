#ifndef IMAGE_H
#define IMAGE_H

#include <cassert>
#include <vecmath.h>
#include "constant.hpp"

typedef struct {
    Vector3f color;
    Vector3f hitPoint;
    Vector3f accumulate;
    Vector3f flux;
    Vector3f phos;
    Vector3f normal;
    int numPhotons;
    float squaredRadius = Constant::squaredRadius;
} Pixel;

// Simple image class
class Image {

public:

    Image(int w, int h) {
        width = w;
        height = h;
        data = new Pixel[width * height];
    }

    ~Image() {
        delete[] data;
    }

    int Width() const {
        return width;
    }

    int Height() const {
        return height;
    }

    const Vector3f &GetPixel(int x, int y) const {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        return data[y * width + x].color;
    }

    Pixel* operator() (int r) {
        return data + r;
    }
    Pixel& operator() (int x, int y) {
        return data[y * width + x];
    }

    void SetAllPixels(const Vector3f &color) {
        for (int i = 0; i < width * height; ++i) {
            data[i].color = color;
        }
    }

    void SetPixel(int x, int y, const Vector3f &color) {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
        data[y * width + x].color = color;
    }

    static Image *LoadPPM(const char *filename);

    void SavePPM(const char *filename) const;

    static Image *LoadTGA(const char *filename);

    void SaveTGA(const char *filename) const;

    int SaveBMP(const char *filename);

    void SaveImage(const char *filename);

private:

    int width;
    int height;
    Pixel *data;

};

#endif // IMAGE_H
