#include "mesh_renderer.h"
#include "molecule_kernel.h"
#include "primitive_geometry_mesh.h"

namespace MeshRenderer
{
    const float bond_radius = 0.3;

    bool renderMolecule(MoleculeStruct::MolecularDataOneFrame* frame,
        std::vector<Vertex>& out_vertices,
        std::vector<uint16_t>& out_indices)
    {
        out_vertices.clear();
        out_indices.clear();

        int triangle_offset = 0;
        for (int i_atom = 0; i_atom < frame->n_atom; i_atom++)
        {
            glm::vec3 center_position{ frame->atoms[i_atom].xyz[0],frame->atoms[i_atom].xyz[1],frame->atoms[i_atom].xyz[2], };
            glm::vec3 color{ frame->atoms[i_atom].rgb[0],frame->atoms[i_atom].rgb[1],frame->atoms[i_atom].rgb[2], };
            float radius = frame->atoms[i_atom].vdw_radius;

            for (int i_vertex = 0; i_vertex < PrimitiveGeometryMesh::SphereMesh.vertex_count_times_three / 3; i_vertex++)
            {
                glm::vec3 vertex_object_space{ PrimitiveGeometryMesh::SphereMesh.vertices[i_vertex * 3 + 0],
                                               PrimitiveGeometryMesh::SphereMesh.vertices[i_vertex * 3 + 1],
                                               PrimitiveGeometryMesh::SphereMesh.vertices[i_vertex * 3 + 2], };
                glm::vec3 vertex_global_space = center_position + radius * vertex_object_space;
                out_vertices.push_back(Vertex{ vertex_global_space, color, 0 });
            }

            for (int i_triangle = 0; i_triangle < PrimitiveGeometryMesh::SphereMesh.triangle_count_times_three / 3; i_triangle++)
            {
                out_indices.push_back(PrimitiveGeometryMesh::SphereMesh.triangles[i_triangle * 3 + 0] + triangle_offset);
                out_indices.push_back(PrimitiveGeometryMesh::SphereMesh.triangles[i_triangle * 3 + 2] + triangle_offset);
                out_indices.push_back(PrimitiveGeometryMesh::SphereMesh.triangles[i_triangle * 3 + 1] + triangle_offset); // The ordering is different
            }

            triangle_offset += PrimitiveGeometryMesh::SphereMesh.vertex_count_times_three / 3;
        }

        std::vector<MoleculeStruct::ChemicalBond> bonds = MoleculeKernel::getBonds(frame);
        int n_bond = bonds.size();
        for (int i_bond = 0; i_bond < n_bond; i_bond++)
        {
            glm::vec3 center[2]{ { bonds[i_bond].atom1_xyz[0], bonds[i_bond].atom1_xyz[1], bonds[i_bond].atom1_xyz[2], },
                                 { bonds[i_bond].atom2_xyz[0], bonds[i_bond].atom2_xyz[1], bonds[i_bond].atom2_xyz[2], } };
            glm::vec3 color[2]{ { bonds[i_bond].atom1_rgb[0], bonds[i_bond].atom1_rgb[1], bonds[i_bond].atom1_rgb[2], },
                                { bonds[i_bond].atom2_rgb[0], bonds[i_bond].atom2_rgb[1], bonds[i_bond].atom2_rgb[2], } };
            float bond_length = glm::distance(center[0], center[1]);
            glm::mat3 bond_scale{ glm::vec3{bond_radius, 0, 0}, glm::vec3{0, bond_length / 4, 0}, glm::vec3{0, 0, bond_radius} };

            for (int i_half_bond = 0; i_half_bond < 2; i_half_bond++)
            {
                glm::vec3 bond_direction = glm::normalize(center[1 - i_half_bond] - center[i_half_bond]);
                glm::mat3 bond_direction_rotation{ glm::cross(glm::vec3(0,0,1), bond_direction), bond_direction, glm::cross(glm::cross(glm::vec3(0,0,1), bond_direction), bond_direction) };

                for (int i_vertex = 0; i_vertex < PrimitiveGeometryMesh::CylinderMesh.vertex_count_times_three / 3; i_vertex++)
                {
                    glm::vec3 vertex_object_space{ PrimitiveGeometryMesh::CylinderMesh.vertices[i_vertex * 3 + 0],
                                                   PrimitiveGeometryMesh::CylinderMesh.vertices[i_vertex * 3 + 1],
                                                   PrimitiveGeometryMesh::CylinderMesh.vertices[i_vertex * 3 + 2], };
                    glm::vec3 vertex_global_space = center[i_half_bond] + bond_direction_rotation * bond_scale * vertex_object_space;
                    out_vertices.push_back(Vertex{ vertex_global_space, color[i_half_bond], 0 });
                }

                for (int i_triangle = 0; i_triangle < PrimitiveGeometryMesh::CylinderMesh.triangle_count_times_three / 3; i_triangle++)
                {
                    out_indices.push_back(PrimitiveGeometryMesh::CylinderMesh.triangles[i_triangle * 3 + 0] + triangle_offset);
                    out_indices.push_back(PrimitiveGeometryMesh::CylinderMesh.triangles[i_triangle * 3 + 2] + triangle_offset);
                    out_indices.push_back(PrimitiveGeometryMesh::CylinderMesh.triangles[i_triangle * 3 + 1] + triangle_offset); // The ordering is different
                }

                triangle_offset += PrimitiveGeometryMesh::CylinderMesh.vertex_count_times_three / 3;
            }
        }

        return true;
    }
}