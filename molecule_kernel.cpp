#include "molecule_kernel.h"

#define SQUARE(x) ((x)*(x))

namespace MoleculeKernel
{
	const float bond_radius_cutoff_scale = 1.3;

	bool ifAtomsBonded(const MoleculeStruct::ChemistryAtom* const atom1, const MoleculeStruct::ChemistryAtom* const atom2)
	{
		float delta_r_sqr = SQUARE(atom1->xyz[0] - atom2->xyz[0]) + SQUARE(atom1->xyz[1] - atom2->xyz[1]) + SQUARE(atom1->xyz[2] - atom2->xyz[2]);
		float bond_radius_cutoff = atom1->bond_radius + atom2->bond_radius;
		return delta_r_sqr < SQUARE(bond_radius_cutoff_scale * bond_radius_cutoff);
	}

	std::vector<MoleculeStruct::ChemicalBond> getBonds(const int n_atom, const MoleculeStruct::ChemistryAtom* const atoms)
	{
		std::vector<MoleculeStruct::ChemicalBond> bonds;
		
		for (int i_atom = 0; i_atom < n_atom; i_atom++)
			for (int j_atom = i_atom + 1; j_atom < n_atom; j_atom++)
				if (ifAtomsBonded(atoms + i_atom, atoms + j_atom))
				{
					MoleculeStruct::ChemicalBond new_bond;
					for (int i = 0; i < 3; i++)
					{
						new_bond.atom1_xyz[i] = atoms[i_atom].xyz[i];
						new_bond.atom2_xyz[i] = atoms[j_atom].xyz[i];
						new_bond.atom1_rgb[i] = atoms[i_atom].rgb[i];
						new_bond.atom2_rgb[i] = atoms[j_atom].rgb[i];
					}

					bonds.push_back(new_bond);
				}

		return bonds;
	}
}