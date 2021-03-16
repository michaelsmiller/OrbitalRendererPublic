
#include "molecule_struct.h"
#include "molecule_reader.h"

#include "renderer.h"

int main() {
    TriangleRenderer app;

    try {
        std::vector<MoleculeStruct::MolecularDataOneFrame*> trajectory
            = MoleculeReader::readWholeTrajectory("./molecule_demo/demo");

        MoleculeReader::clearTrajectory(trajectory);

        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
