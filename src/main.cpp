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

    SceneParser sceneParser(inputFile.c_str());
    Image image(
        sceneParser.getCamera()->getWidth(), 
        sceneParser.getCamera()->getHeight()
    );
    Chroma chroma(sceneParser, image);
    // chroma.render(10, 200000, 5, true, 5);
    chroma.render(2500, 200000, 50, true, 0);
    // chroma.render();
    image.SaveBMP(outputFile.c_str());
    cout << "Hello! Computer Graphics!" << endl;
    return 0;
}

