#include "mesh_renderer.h"
#include "molecule_kernel.h"
#include "primitive_geometry_mesh.h"

namespace MeshRenderer
{
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

        //TODO: bonds

        return true;
    }
}