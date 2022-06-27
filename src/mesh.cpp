#include "mesh.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>

bool Mesh::intersect(const Ray &r, Hit &h, float tmin) {

    // Optional: Change this brute force method into a faster one.
    // bool result = false;
    // for (int triId = 0; triId < (int) patches.size(); ++triId) {
    //     Triangle *tria = patches[triId];
    //     result |= tria->intersect(r, h, tmin);
    // }
    // return result;
    return tree.intersect(r, h, tmin);
}

Mesh::Mesh(const char *filename, Material *material) : Object3D(material), patches(0), tree() {

    // Optional: Use tiny obj loader to replace this simple one.
    std::ifstream f;
    f.open(filename);
    if (!f.is_open()) {
        std::cout << "Cannot open " << filename << "\n";
        return;
    }
    std::string line;
    std::string vTok("v");
    std::string fTok("f");
    std::string texTok("vt"); 
    std::string normTok("vn");
    char bslash = '/';
    std::string tok;
    std::vector<Vector3f> v;
    std::vector<TriangleIndex> t, text, norm;
    std::vector<Vector2f> vt;
    std::vector<Vector3f> vn;
    while (true) {
        std::getline(f, line);
        if (f.eof()) {
            break;
        }
        if (line.size() < 3) {
            continue;
        }
        if (line.at(0) == '#') {
            continue;
        }
        std::stringstream ss(line);
        ss >> tok;
        if (tok == vTok) {
            Vector3f vec;
            ss >> vec[0] >> vec[1] >> vec[2];
            v.push_back(vec);
        } else if (tok == fTok) {
            TriangleIndex trig, tex, nor;
            for (int i = 0; i < 3; ++i) {
                std::string face;
                std::vector<std::string> res;
                ss >> face;
                Utils::stringSplit(face, bslash, res);
                trig[i] = stoi(res[0]) - 1;
                if (res.size() > 1 && res[1] != "") {
                    tex[i] = stoi(res[1]) - 1;
                }
                if (res.size() > 2 && res[2] != "") {
                    nor[i] = stoi(res[2]) - 1;
                }
            }
            t.push_back(trig);
            text.push_back(tex);
            norm.push_back(nor);
        } else if (tok == texTok) {
            Vector2f texcoord;
            ss >> texcoord[0];
            ss >> texcoord[1];
            vt.push_back(texcoord);
        } else if (tok == normTok) {
            Vector3f norcoord;
            ss >> norcoord[0] >> norcoord[1] >> norcoord[2];
            vn.push_back(norcoord);
        }
    }

    f.close();
    for (int triId = 0; triId < (int)t.size(); ++triId) {
        TriangleIndex &idx = t[triId];
        Triangle *tria = new Triangle(v[idx[0]], v[idx[1]], v[idx[2]], material);
        if (text[triId][0] >= 0) {
            tria->setTextures(vt[text[triId][0]], vt[text[triId][1]], vt[text[triId][2]]);
        }
        if (norm[triId][0] >= 0) {
            tria->setNormals(vn[norm[triId][0]], vn[norm[triId][1]], vn[norm[triId][2]]);
        }
        patches.push_back(tria);
    }
    tree.construct(patches);
    setBound(tree.getKonta(), tree.getMakria());
}
