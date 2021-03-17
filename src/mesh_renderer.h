#pragma once

#include <vector>

#include "molecule_struct.h"
#include "renderer.h"

namespace MeshRenderer
{
    bool renderMolecule(MoleculeStruct::MolecularDataOneFrame* frame,
                        std::vector<Vertex>& out_vertices,
                        std::vector<uint16_t>& out_indices);
}