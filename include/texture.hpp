#ifndef TEXTURE_H
#define TEXTURE_H

#include "image.hpp"

using namespace std;

class Texture {
public:
    Texture(int width, int height, int channel, unsigned char *img): image(width, height) {
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
    Vector3f getColor(float u, float v) {
        u = u * (image.Width() - 1);
        v = v * (image.Height() - 1);
        int x = u, y = v;
        double alpha = u - x, beta = v - y;
        Vector3f ret = Vector3f::ZERO;
        ret += (1 - alpha) * (1 - beta) * image.GetPixel(x, y);
        ret += alpha * (1 - beta) * image.GetPixel(x + 1, y);
        ret += (1 - alpha) * beta * image.GetPixel(x, y + 1);
        ret += alpha * beta * image.GetPixel(x + 1, y + 1);
        return ret;
    }
protected:
    Image image;
};

#endif