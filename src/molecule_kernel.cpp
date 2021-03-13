#include "molecule_kernel.h"
#include <math.h>

#define ONE_OVER_PI_TO_3_OVER_4 0.4237772081f // 1 / PI ^ (3/4)
#define ANGSTROM2BOHR 1.889725989f
#define ANGSTROM2BOHR_SQUARE 3.5710643135f
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

    float evaluateOrbital(const float xyz[3], const MoleculeStruct::MolecularDataOneFrame* const frame)
    {
        return evaluateOrbital(xyz, frame->n_AO, frame->aos, frame->primitives, frame->mo_coefficients);
    }

    float evaluateOrbital(const float xyz[3],
                          const int n_ao,
                          const MoleculeStruct::AtomicOrbital* const aos,
                          const MoleculeStruct::GaussianPrimitive* const prims,
                          const float* const C)
    {
        const int i_occ = 0;
        float x = xyz[0], y = xyz[1], z = xyz[2];

        float psi = 0;
        for (int i_ao = 0, i_total_prim = 0; i_ao < n_ao; i_ao++)
            for (int i_prim = 0; i_prim < aos[i_ao].number_of_primitives; i_prim++, i_total_prim++)
            {
                float A_x = aos[i_ao].xyz[0], A_y = aos[i_ao].xyz[1], A_z = aos[i_ao].xyz[2],
                    exponent = prims[i_total_prim].exponent, contraction = prims[i_total_prim].contraction;

                switch (aos[i_ao].quantum_number)
                {
                case ((1 << (0 * 2)) + 0): //s
                    psi += C[i_ao + i_occ * n_ao] * contraction * powf(2 * exponent, 0.75f) * ONE_OVER_PI_TO_3_OVER_4 // (2 * exponent / PI) ^ (3/4)
                        * expf(-exponent * ANGSTROM2BOHR_SQUARE * (SQUARE(x - A_x) + SQUARE(y - A_y) + SQUARE(z - A_z)));
                    break;
                case ((1 << (1 * 2)) + 0): //p-x
                    psi += C[i_ao + i_occ * n_ao] * contraction * powf(exponent, 1.25f) * 3.363585661f * ONE_OVER_PI_TO_3_OVER_4 // ( 128 * exponent^5 / PI^3) ^ (1/4)
                        * (x - A_x) * ANGSTROM2BOHR
                        * expf(-exponent * ANGSTROM2BOHR_SQUARE * (SQUARE(x - A_x) + SQUARE(y - A_y) + SQUARE(z - A_z)));
                    break;
                case ((1 << (1 * 2)) + 1): //p-y
                    psi += C[i_ao + i_occ * n_ao] * contraction * powf(exponent, 1.25f) * 3.363585661f * ONE_OVER_PI_TO_3_OVER_4 // ( 128 * exponent^5 / PI^3) ^ (1/4)
                        * (y - A_y) * ANGSTROM2BOHR
                        * expf(-exponent * ANGSTROM2BOHR_SQUARE * (SQUARE(x - A_x) + SQUARE(y - A_y) + SQUARE(z - A_z)));
                    break;
                case ((1 << (1 * 2)) + 2): //p-z
                    psi += C[i_ao + i_occ * n_ao] * contraction * powf(exponent, 1.25f) * 3.363585661f * ONE_OVER_PI_TO_3_OVER_4 // ( 128 * exponent^5 / PI^3) ^ (1/4)
                        * (z - A_z) * ANGSTROM2BOHR
                        * expf(-exponent * ANGSTROM2BOHR_SQUARE * (SQUARE(x - A_x) + SQUARE(y - A_y) + SQUARE(z - A_z)));
                    break;
                case ((1 << (2 * 2)) + 0): //d-xy
                    psi += C[i_ao + i_occ * n_ao] * contraction * powf(exponent, 1.75f) * 6.727171322f * ONE_OVER_PI_TO_3_OVER_4 // (2048 * exponent^7 / PI^3) ^ (1/4)
                        * (x - A_x) * (y - A_y) * ANGSTROM2BOHR_SQUARE
                        * expf(-exponent * ANGSTROM2BOHR_SQUARE * (SQUARE(x - A_x) + SQUARE(y - A_y) + SQUARE(z - A_z)));
                    break;
                case ((1 << (2 * 2)) + 1): //d-xz
                    psi += C[i_ao + i_occ * n_ao] * contraction * powf(exponent, 1.75f) * 6.727171322f * ONE_OVER_PI_TO_3_OVER_4 // (2048 * exponent^7 / PI^3) ^ (1/4)
                        * (x - A_x) * (z - A_z) * ANGSTROM2BOHR_SQUARE
                        * expf(-exponent * ANGSTROM2BOHR_SQUARE * (SQUARE(x - A_x) + SQUARE(y - A_y) + SQUARE(z - A_z)));
                    break;
                case ((1 << (2 * 2)) + 2): //d-yz
                    psi += C[i_ao + i_occ * n_ao] * contraction * powf(exponent, 1.75f) * 6.727171322f * ONE_OVER_PI_TO_3_OVER_4 // (2048 * exponent^7 / PI^3) ^ (1/4)
                        * (y - A_y) * (z - A_z) * ANGSTROM2BOHR_SQUARE
                        * expf(-exponent * ANGSTROM2BOHR_SQUARE * (SQUARE(x - A_x) + SQUARE(y - A_y) + SQUARE(z - A_z)));
                    break;
                case ((1 << (2 * 2)) + 3): //d-xx
                    psi += C[i_ao + i_occ * n_ao] * contraction * powf(exponent, 1.75f) * 6.727171322f / 9 * ONE_OVER_PI_TO_3_OVER_4 // (2048 * exponent^7 / PI^3) ^ (1/4) / 9
                        * SQUARE(x - A_x) * ANGSTROM2BOHR_SQUARE
                        * expf(-exponent * ANGSTROM2BOHR_SQUARE * (SQUARE(x - A_x) + SQUARE(y - A_y) + SQUARE(z - A_z)));
                    break;
                case ((1 << (2 * 2)) + 4): //d-yy
                    psi += C[i_ao + i_occ * n_ao] * contraction * powf(exponent, 1.75f) * 6.727171322f / 9 * ONE_OVER_PI_TO_3_OVER_4 // (2048 * exponent^7 / PI^3) ^ (1/4) / 9
                        * SQUARE(y - A_y) * ANGSTROM2BOHR_SQUARE
                        * expf(-exponent * ANGSTROM2BOHR_SQUARE * (SQUARE(x - A_x) + SQUARE(y - A_y) + SQUARE(z - A_z)));
                    break;
                case ((1 << (2 * 2)) + 5): //d-zz
                    psi += C[i_ao + i_occ * n_ao] * contraction * powf(exponent, 1.75f) * 6.727171322f / 9 * ONE_OVER_PI_TO_3_OVER_4 // (2048 * exponent^7 / PI^3) ^ (1/4) / 9
                        * SQUARE(z - A_z) * ANGSTROM2BOHR_SQUARE
                        * expf(-exponent * ANGSTROM2BOHR_SQUARE * (SQUARE(x - A_x) + SQUARE(y - A_y) + SQUARE(z - A_z)));
                    break;
                case ((1 << (3 * 2)) + 0): //f
                    //Not supported for f orbitals.
                default:
                    psi = NAN;
                }
            }

        return psi;
    }
}
