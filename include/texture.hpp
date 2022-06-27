#ifndef TEXTURE_H
#define TEXTURE_H

#include "image.hpp"

using namespace std;

class Texture {
public:
    Texture(int width, int height, int channel, unsigned char *img): image(width, height), width(width), height(height) {
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                int idx = (x + y * width) * channel;
                image.SetPixel(x, y, Vector3f(
                    img[idx] / 255.0,
                    img[idx + 1] / 255.0,
                    img[idx + 2] / 255.0
                ));
            }
        }
    }
    Vector3f getPixel(int x, int y) {
        x = x < 0 ? 0 : (x >= width ? (width - 1) : x);
        y = y < 0 ? 0 : (y >= height ? (height - 1) : y);
        return image.GetPixel(x, y);
    }
    Vector3f getColor(float u, float v) {
        // u -= int(u);
        // v -= int(v);
        // if (u < 0) u += 1;
        // if (v < 0) v += 1;
        u = u * width;
        v = v * height;
        int x = u, y = v;
        float alpha = u - x, beta = v - y;
        Vector3f ret = Vector3f::ZERO;
        ret += (1 - alpha) * (1 - beta) * getPixel(x, y);
        ret += alpha * (1 - beta) * getPixel(x + 1, y);
        ret += (1 - alpha) * beta * getPixel(x, y + 1);
        ret += alpha * beta * getPixel(x + 1, y + 1);
        return ret;
    }
protected:
    Image image;
    int width, height;
};

#endif