#pragma once

#include <vector>

#include "molecule_struct.h"
#include "renderer.h"

namespace MeshRenderer
{
    bool renderMolecule(const MoleculeStruct::MolecularDataOneFrame* const frame,
                        std::vector<Vertex>& out_vertices,
                        std::vector<uint32_t>& out_indices);

    bool renderOrbital(const MoleculeStruct::MolecularDataOneFrame* const frame,
                       std::vector<Vertex>& out_vertices,
                       std::vector<uint32_t>& out_indices);
}