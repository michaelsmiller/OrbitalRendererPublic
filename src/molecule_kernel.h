#pragma once

#include <vector>
#include "molecule_struct.h"

namespace MoleculeKernel
{
    bool ifAtomsBonded(const MoleculeStruct::ChemistryAtom* const atom1, const MoleculeStruct::ChemistryAtom* const atom2);
    std::vector<MoleculeStruct::ChemicalBond> getBonds(const MoleculeStruct::MolecularDataOneFrame* const frame);
    std::vector<MoleculeStruct::ChemicalBond> getBonds(const int n_atom, const MoleculeStruct::ChemistryAtom* const atoms);

    float evaluateOrbital(const float xyz[3], const MoleculeStruct::MolecularDataOneFrame* const frame);
    float evaluateOrbital(const float xyz[3],
                          const int n_ao,
                          const MoleculeStruct::AtomicOrbital* const aos,
                          const MoleculeStruct::GaussianPrimitive* const prims,
                          const float* const C);
}