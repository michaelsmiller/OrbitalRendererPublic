#include <iostream>
#include <fstream>
#include <exception>
#include <string>

#include "molecule_reader.h"

namespace MoleculeReader
{
    const int n_element = 37;
    const char* element_name[n_element] = { "X", "H", "He", "Li", "Be", "B",
                                            "C", "N", "O", "F", "Ne",
                                            "Na", "Mg", "Al", "Si", "P",
                                            "S", "Cl", "Ar", "K", "Ca",
                                            "Sc", "Ti", "V", "Cr", "Mn",
                                            "Fe", "Co", "Ni", "Cu", "Zn",
                                            "Ga", "Ge", "As", "Se", "Br", "Kr" };
    const float element_color[n_element][3] = { { 0,0,0 }, { 0.9,0.9,0.9 }, { 0.850980392,1.000000000,1.000000000 }, { 0.800000000,0.501960784,1.000000000 }, { 0.760784314,1.000000000,0.000000000 }, { 1.000000000,0.709803922,0.709803922 },
                                                { 0.1, 0.1, 0.1 }, { 0.2,0.2,1.0 }, { 1.0,0.3,0.3 }, { 0.701960784,1.000000000,1.000000000 }, { 0.701960784,0.890196078,0.960784314 },
                                                { 0.670588235,0.360784314,0.949019608}, { 0.541176471,1.000000000,0.000000000 }, { 0.749019608,0.650980392,0.650980392 }, { 0.941176471,0.784313725,0.627450980 }, { 1.000000000,0.501960784,0.000000000 },
                                                { 0.9,0.775,0.25}, { 0.121568627,0.941176471,0.121568627 }, { 0.501960784,0.819607843,0.890196078 }, { 0.560784314,0.250980392,0.831372549 }, { 0.239215686,1.000000000,0.000000000 },
                                                { 0.901960784,0.901960784,0.901960784 }, { 0.749019608,0.760784314,0.780392157 }, { 0.650980392,0.650980392,0.670588235 }, { 0.541176471,0.600000000,0.780392157 }, { 0.611764706,0.478431373,0.780392157 },
                                                { 0.878431373,0.400000000,0.200000000 }, { 0.941176471,0.564705882,0.627450980 }, { 0.313725490,0.815686275,0.313725490 }, { 0.784313725,0.501960784,0.200000000 }, { 0.490196078,0.501960784,0.690196078 },
                                                { 0.760784314,0.560784314,0.560784314 }, { 0.400000000,0.560784314,0.560784314 }, { 0.741176471,0.501960784,0.890196078 }, { 1.000000000,0.631372549,0.000000000 }, { 0.650980392,0.160784314,0.160784314 }, { 0.360784314,0.721568627,0.819607843 } };
    const float element_vdw_radius[n_element] = { 0, 1.2, 1.4, 1.82, -1, -1,
                                                  1.7, 1.55, 1.52, 1.47, 1.54,
                                                  2.27, 1.73, -1, 2.1, 1.8,
                                                  1.8, 1.75, 1.88, 2.75, -1,
                                                  -1, -1, -1, -1, -1,
                                                  -1, -1, 1.63, 1.4, 1.39,
                                                  1.87, -1, 1.85, 1.9, 1.85, 2.02 };
    const float element_default_vdw_radius = 2.0;
    const float element_bond_radius[n_element] = { 0, 0.53, 0.31, 1.67, 1.12, 0.87,
                                                   0.67, 0.56, 0.48, 0.42, 0.38,
                                                   1.9, 1.45, 1.18, 1.11, 0.98,
                                                   0.88, 0.79, 0.71, 2.43, 1.94,
                                                   1.84, 1.76, 1.71, 1.66, 1.61,
                                                   1.56, 1.52, 1.49, 1.45, 1.42,
                                                   1.36, 1.25, 1.14, 1.03, 0.94, 0.88 };
    const float element_default_bond_radius = 1.0;

    const char* xyz_extension = ".xyz";
    const char* ao_extension = ".ao.txt";
    const char* prim_extension = ".prim.txt";
    const char* C_extension = ".C.txt";

    // http://www.cplusplus.com/articles/1UqpX9L8/
    class splitstring : public std::string
    {
    private:
        std::vector<std::string> flds;
    public:
        splitstring(char* s) : std::string(s) { };
        splitstring(std::string s) : std::string(s) { };
        std::vector<std::string>& split(char delim, int rep = 0)
        {
            if (!flds.empty()) flds.clear();
            std::string work = data();
            std::string buf = "";
            int i = 0;
            while (i < work.length()) {
                if (work[i] != delim)
                    buf += work[i];
                else if (rep == 1) {
                    flds.push_back(buf);
                    buf = "";
                }
                else if (buf.length() > 0) {
                    flds.push_back(buf);
                    buf = "";
                }
                i++;
            }
            if (!buf.empty())
                flds.push_back(buf);
            return flds;
        }
    };

    std::vector<MoleculeStruct::MolecularDataOneFrame*> readWholeTrajectory(const char* const filename)
    {
        std::vector<MoleculeStruct::MolecularDataOneFrame*> video_data;

        std::string filename_string(filename);
        std::ifstream xyz_file(filename_string + xyz_extension);
        std::ifstream ao_file(filename_string + ao_extension);
        std::ifstream prim_file(filename_string + prim_extension);
        std::ifstream C_file(filename_string + C_extension);

        if (!xyz_file.is_open())
        {
            std::cout << "Cannot open " + filename_string + xyz_extension << std::endl;
            return video_data;
        }
        if (!ao_file.is_open())
        {
            std::cout << "Cannot open " + filename_string + ao_extension << std::endl;
            return video_data;
        }
        if (!prim_file.is_open())
        {
            std::cout << "Cannot open " + filename_string + prim_extension << std::endl;
            return video_data;
        }
        if (!C_file.is_open())
        {
            std::cout << "Cannot open " + filename_string + C_extension << std::endl;
            return video_data;
        }

        while (!xyz_file.eof() && !ao_file.eof() && !prim_file.eof() && !C_file.eof())
        {
            int n_atom = 0, n_ao = 0, n_prim = 0;
            std::string temp;

            std::getline(xyz_file, temp);
            if (temp.empty()) break;
            n_atom = std::stoi(temp);
            std::getline(xyz_file, temp); // Skip comment line

            std::getline(ao_file, temp);
            if (temp.empty()) break;
            n_ao = std::stoi(temp);
            std::getline(ao_file, temp); // Skip comment line

            std::getline(prim_file, temp);
            if (temp.empty()) break;
            n_prim = std::stoi(temp);
            std::getline(prim_file, temp); // Skip comment line

            std::getline(C_file, temp);
            if (temp.empty()) break;
            if (n_ao != std::stoi(temp))
            {
                std::cout << "Inconsistent AO number from ao file and C file" << std::endl;
                return video_data;
            }
            std::getline(C_file, temp); // Skip comment line

            MoleculeStruct::MolecularDataOneFrame* frame = new MoleculeStruct::MolecularDataOneFrame(n_atom, n_ao, n_prim);

            for (int i_atom = 0; i_atom < n_atom; i_atom++)
            {
                std::getline(xyz_file, temp);
                std::vector<std::string> splitted = splitstring(temp).split(' ');
                int atomic_number = -1;
                for (int i = 0; i < n_element; i++)
                    if (element_name[i] == splitted[0])
                    {
                        atomic_number = i;
                        break;
                    }
                if (atomic_number < 0)
                {
                    std::cout << "Incorrect atom type: " + splitted[0] << std::endl;
                    return video_data;
                }
                frame->atoms[i_atom].atomic_number = atomic_number;
                frame->atoms[i_atom].xyz[0] = std::stof(splitted[1]);
                frame->atoms[i_atom].xyz[1] = std::stof(splitted[2]);
                frame->atoms[i_atom].xyz[2] = std::stof(splitted[3]);
                frame->atoms[i_atom].vdw_radius = element_vdw_radius[atomic_number] > 0 ? element_vdw_radius[atomic_number] : element_default_vdw_radius;
                frame->atoms[i_atom].bond_radius = element_bond_radius[atomic_number] > 0 ? element_bond_radius[atomic_number] : element_default_bond_radius;
                frame->atoms[i_atom].rgb[0] = element_color[atomic_number][0];
                frame->atoms[i_atom].rgb[1] = element_color[atomic_number][1];
                frame->atoms[i_atom].rgb[2] = element_color[atomic_number][2];
            }

            for (int i_ao = 0; i_ao < n_ao; i_ao++)
            {
                std::getline(ao_file, temp);
                std::vector<std::string> splitted = splitstring(temp).split(' ');
                frame->aos[i_ao].xyz[0] = std::stof(splitted[0]);
                frame->aos[i_ao].xyz[1] = std::stof(splitted[1]);
                frame->aos[i_ao].xyz[2] = std::stof(splitted[2]);
                frame->aos[i_ao].quantum_number = std::stoi(splitted[3]);
                frame->aos[i_ao].number_of_primitives = std::stoi(splitted[4]);

                std::getline(C_file, temp);
                frame->mo_coefficients[i_ao] = std::stof(temp);
            }

            for (int i_prim = 0; i_prim < n_prim; i_prim++)
            {
                std::getline(prim_file, temp);
                std::vector<std::string> splitted = splitstring(temp).split(' ');
                frame->primitives[i_prim].exponent = std::stof(splitted[0]);
                frame->primitives[i_prim].contraction = std::stof(splitted[1]);
            }

            video_data.push_back(frame);
        }

        std::cout << video_data.size() << " frames loaded!" << std::endl;
        return video_data;
    }

    bool clearTrajectory(std::vector<MoleculeStruct::MolecularDataOneFrame*>& trajectory)
    {
        for (auto it = trajectory.begin(); it != trajectory.end(); it++)
            delete (*it);
        trajectory.clear();

        return true;
    }
}
