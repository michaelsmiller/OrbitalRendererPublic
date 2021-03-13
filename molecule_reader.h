#pragma once

#include <vector>
#include "glm/vec3.hpp"

namespace MoleculeReader
{
	struct ChemistryAtom
	{
		int atomic_number;
		float rgb[3];
		float vdw_radius;
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
	
	class MolecularDataOneFrame
	{
	public:
		MolecularDataOneFrame(int set_n_atom, int set_n_AO, int set_n_primitive);
		~MolecularDataOneFrame();

		int n_atom;
		int n_AO;
		int n_primitive;
		ChemistryAtom* atoms;
		AtomicOrbital* aos;
		GaussianPrimitive* primitives;
		float* mo_coefficients;

	private:
		MolecularDataOneFrame() = delete;
		MolecularDataOneFrame(const MolecularDataOneFrame&) = delete;
		MolecularDataOneFrame& operator=(const MolecularDataOneFrame&) = delete;
	};

	std::vector<MolecularDataOneFrame*> readWholeTrajectory(const char* filename) throw();
}