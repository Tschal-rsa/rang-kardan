#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <map>
#include <cstring>

using namespace std;

typedef struct {
    float diffuse;
    float specular;
    float refract;
    float refr;
} Properties;

static map<string, Properties> nameToProperties = {
    {"Matte",       {1, 0, 0, 0}},
    {"Mirror",      {0, 1, 0, 0}},
    {"Glass",       {0, 0, 1, 1.65}},
    {"DiffLight",   {1, 0, 0, 0}},
    {"Light",       {0, 1, 0, 0}},
    {"China",       {0.8, 0.2, 0, 0}},
    {"Apple",       {0.99, 0.1, 0, 0}},
    {"Marble",      {0.9, 0.1, 0, 0}},
    {"Photo",       {0.8, 0.2, 0, 0}},
    {"Cobble",      {0.7, 0.3, 0, 0}}
};

class Distribution {
public:
    static const Properties& getProperties(string name) {
        return nameToProperties[name];
    }
};

#endif