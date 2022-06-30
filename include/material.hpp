#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include "distribution.hpp"
#include "texture.hpp"
#include <iostream>

// TODO: Implement Shade function that computes Phong introduced in class.
class Material {
public:

    // explicit Material(const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, const Vector3f &e_color = Vector3f::ZERO, float s = 0) :
    //         diffuseColor(d_color), specularColor(s_color), emissionColor(e_color), shininess(s) {

    // }
    explicit Material(const Vector3f &color, const Vector3f &phos, const Properties &prop, Texture *texture = nullptr, Texture *normal = nullptr): color(color), phos(phos), prop(prop), texture(texture), normal(normal) {}

    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor() const {
        return color;
    }
    Vector3f getColor() {
        return color;
    }
    Vector3f getColor(float u, float v) {
        return texture ? texture->getColor(u, v) : color;
    }
    bool hasNormal() {
        return normal != nullptr;
    }
    Vector3f getNormal(float u, float v) {
        return (normal->getColor(u, v) * 2 - 1).normalized();
    }
    Vector3f getPhos() const {
        return phos;
    }
    float getDiffuse() const {
        return prop.diffuse;
    }
    float getSpecular() const {
        return prop.specular;
    }
    float getRefract() const {
        return prop.refract;
    }
    float getRefr() const {
        return prop.refr;
    }

    // Vector3f Shade(const Ray &ray, const Hit &hit,
    //                const Vector3f &dirToLight, const Vector3f &lightColor) {
    //     float dotProduct = Vector3f::dot(dirToLight, hit.getNormal());
    //     Vector3f diffuse = diffuseColor * std::max<float>(0, dotProduct);
    //     Vector3f specular = specularColor * pow(std::max<float>(0, Vector3f::dot(-ray.getDirection(), 2 * dotProduct * hit.getNormal() - dirToLight)), shininess);
    //     Vector3f shaded = lightColor * (diffuse + specular + emissionColor);
    //     return shaded;
    // }
    Vector3f Shade(const Ray &ray, const Hit &hit, const Vector3f &dirToLight, const Vector3f &lightColor) {
        return Vector3f::ZERO;
    }

protected:
    // Vector3f diffuseColor;
    // Vector3f specularColor;
    // Vector3f emissionColor;
    // float shininess;
    Vector3f color, phos;
    Properties prop;
    Texture *texture, *normal;
};


#endif // MATERIAL_H
