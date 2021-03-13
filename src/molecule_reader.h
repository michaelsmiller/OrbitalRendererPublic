#pragma once

#include <vector>
#include "molecule_struct.h"

namespace MoleculeReader
{
    std::vector<MoleculeStruct::MolecularDataOneFrame*> readWholeTrajectory(const char* const filename);

    bool clearTrajectory(std::vector<MoleculeStruct::MolecularDataOneFrame*>& trajectory);
}