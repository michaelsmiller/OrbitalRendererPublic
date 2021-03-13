#pragma once

#include <vector>
#include "molecule_struct.h"

namespace MoleculeKernel
{
	bool ifAtomsBonded(const MoleculeStruct::ChemistryAtom* const atom1, const MoleculeStruct::ChemistryAtom* const atom2);
	std::vector<MoleculeStruct::ChemicalBond> getBonds(const int n_atom, const MoleculeStruct::ChemistryAtom* const atoms);


}