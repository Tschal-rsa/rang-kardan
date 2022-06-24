#include "chroma.hpp"

using namespace std;

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
    Image image(sceneParser.getCamera()->getWidth(), sceneParser.getCamera()->getHeight());
    Chroma chroma(sceneParser, image);
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
    chroma.render(2500, 200000, 50);
    // chroma.render();
    chroma.getImage()->SaveBMP(outputFile.c_str());
    cout << "Hello! Computer Graphics!" << endl;
    return 0;
}

