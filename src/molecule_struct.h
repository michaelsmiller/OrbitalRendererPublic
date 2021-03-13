#pragma once

namespace MoleculeStruct
{
    struct ChemistryAtom
    {
        int atomic_number;
        float rgb[3];
        float vdw_radius;
        float bond_radius;
        float xyz[3];
    };
    struct AtomicOrbital
    {
        int quantum_number;
        int number_of_primitives;
        float xyz[3];
    };
    struct GaussianPrimitive
    {
        float exponent;
        float contraction;
    };
    struct ChemicalBond
    {
        float atom1_xyz[3];
        float atom2_xyz[3];
        float atom1_rgb[3];
        float atom2_rgb[3];
    };

    class MolecularDataOneFrame
    {
    public:
        int n_atom;
        int n_AO;
        int n_primitive;
        ChemistryAtom* atoms;
        AtomicOrbital* aos;
        GaussianPrimitive* primitives;
        float* mo_coefficients;

        MolecularDataOneFrame(const int set_n_atom, const int set_n_AO, const int set_n_primitive)
        {
            this->n_atom = set_n_atom;
            this->n_AO = set_n_AO;
            this->n_primitive = set_n_primitive;

            this->atoms = new ChemistryAtom[set_n_atom];
            this->aos = new AtomicOrbital[set_n_AO];
            this->primitives = new GaussianPrimitive[set_n_primitive];
            this->mo_coefficients = new float[set_n_AO];
        }

        ~MolecularDataOneFrame()
        {
            delete[] this->atoms;
            delete[] this->aos;
            delete[] this->primitives;
            delete[] this->mo_coefficients;
        }

    private:
        MolecularDataOneFrame() = delete;
        MolecularDataOneFrame(const MolecularDataOneFrame&) = delete;
        MolecularDataOneFrame& operator=(const MolecularDataOneFrame&) = delete;
    };
}