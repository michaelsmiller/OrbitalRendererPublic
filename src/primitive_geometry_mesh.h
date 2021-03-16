#pragma once

namespace PrimitiveGeometryMesh
{
	struct GeometryMesh
	{
		const int vertex_count;
		const int triangle_count;
		const float* vertices;
		const int* triangles;
	};

	namespace RawData
	{
		const int sphere_mesh_vertex_count = 486;
		const int sphere_mesh_triangle_count = 320 * 3;
		const float sphere_mesh_vertices[sphere_mesh_vertex_count] = { -0.52573109,0.85065079,0,-0.69378048,0.70204645,0.16062203,-0.43388855,0.86266851,0.2598919,-0.80901694,0.49999997,0.30901697,-0.58778524,0.688191,0.42532539,-0.30901697,0.80901694,0.49999997,-0.85065079,0,0.52573109,-0.70204645,0.16062203,0.69378048,-0.86266851,0.2598919,0.43388855,-0.49999997,0.30901697,0.80901694,-0.688191,0.42532539,0.58778524,0,0.52573109,0.85065079,-0.16062203,0.69378048,0.70204645,-0.2598919,0.43388855,0.86266851,-0.42532539,0.58778524,0.688191,-0.27326649,0.96193826,0,-0.16245985,0.95105654,0.26286554,0,0.99999994,0,0.16062203,0.69378048,0.70204645,0.30901697,0.80901694,0.49999997,0,0.85065079,0.52573109,0.52573109,0.85065079,0,0.27326649,0.96193826,0,0.43388855,0.86266851,0.2598919,0.16245985,0.95105654,0.26286554,-0.43388855,0.86266851,-0.2598919,-0.16245985,0.95105654,-0.26286554,-0.30901697,0.80901694,-0.49999997,0.43388855,0.86266851,-0.2598919,0.30901697,0.80901694,-0.49999997,0.16245985,0.95105654,-0.26286554,0,0.52573109,-0.85065079,-0.16062203,0.69378048,-0.70204645,0.16062203,0.69378048,-0.70204645,0,0.85065079,-0.52573109,-0.69378048,0.70204645,-0.16062203,-0.58778524,0.688191,-0.42532539,-0.80901694,0.49999997,-0.30901697,-0.2598919,0.43388855,-0.86266851,-0.49999997,0.30901697,-0.80901694,-0.42532539,0.58778524,-0.688191,-0.85065079,0,-0.52573109,-0.86266851,0.2598919,-0.43388855,-0.70204645,0.16062203,-0.69378048,-0.688191,0.42532539,-0.58778524,-0.85065079,0.52573109,0,-0.96193826,0,-0.27326649,-0.99999994,0,0,-0.95105654,0.26286554,-0.16245985,-0.96193826,0,0.27326649,-0.95105654,0.26286554,0.16245985,0.69378048,0.70204645,0.16062203,0.58778524,0.688191,0.42532539,0.80901694,0.49999997,0.30901697,0.2598919,0.43388855,0.86266851,0.49999997,0.30901697,0.80901694,0.42532539,0.58778524,0.688191,0.85065079,0,0.52573109,0.86266851,0.2598919,0.43388855,0.70204645,0.16062203,0.69378048,0.688191,0.42532539,0.58778524,0,0.27326649,0.96193826,-0.26286554,0.16245985,0.95105654,0,0,0.99999994,-0.70204645,-0.16062203,0.69378048,-0.49999997,-0.30901697,0.80901694,-0.52573109,0,0.85065079,0,-0.52573109,0.85065079,0,-0.27326649,0.96193826,-0.2598919,-0.43388855,0.86266851,-0.26286554,-0.16245985,0.95105654,-0.86266851,-0.2598919,0.43388855,-0.95105654,-0.26286554,0.16245985,-0.80901694,-0.49999997,0.30901697,-0.86266851,-0.2598919,-0.43388855,-0.80901694,-0.49999997,-0.30901697,-0.95105654,-0.26286554,-0.16245985,-0.52573109,-0.85065079,0,-0.69378048,-0.70204645,0.16062203,-0.69378048,-0.70204645,-0.16062203,-0.85065079,-0.52573109,0,-0.70204645,-0.16062203,-0.69378048,-0.52573109,0,-0.85065079,-0.49999997,-0.30901697,-0.80901694,0,0.27326649,-0.96193826,0,0,-0.99999994,-0.26286554,0.16245985,-0.95105654,0,-0.52573109,-0.85065079,-0.2598919,-0.43388855,-0.86266851,0,-0.27326649,-0.96193826,-0.26286554,-0.16245985,-0.95105654,0.2598919,0.43388855,-0.86266851,0.42532539,0.58778524,-0.688191,0.49999997,0.30901697,-0.80901694,0.69378048,0.70204645,-0.16062203,0.80901694,0.49999997,-0.30901697,0.58778524,0.688191,-0.42532539,0.85065079,0,-0.52573109,0.70204645,0.16062203,-0.69378048,0.86266851,0.2598919,-0.43388855,0.688191,0.42532539,-0.58778524,0.52573109,-0.85065079,0,0.69378048,-0.70204645,0.16062203,0.43388855,-0.86266851,0.2598919,0.80901694,-0.49999997,0.30901697,0.58778524,-0.688191,0.42532539,0.30901697,-0.80901694,0.49999997,0.70204645,-0.16062203,0.69378048,0.86266851,-0.2598919,0.43388855,0.49999997,-0.30901697,0.80901694,0.688191,-0.42532539,0.58778524,0.16062203,-0.69378048,0.70204645,0.2598919,-0.43388855,0.86266851,0.42532539,-0.58778524,0.688191,0.27326649,-0.96193826,0,0.16245985,-0.95105654,0.26286554,0,-0.99999994,0,-0.16062203,-0.69378048,0.70204645,-0.30901697,-0.80901694,0.49999997,0,-0.85065079,0.52573109,-0.27326649,-0.96193826,0,-0.43388855,-0.86266851,0.2598919,-0.16245985,-0.95105654,0.26286554,0.43388855,-0.86266851,-0.2598919,0.16245985,-0.95105654,-0.26286554,0.30901697,-0.80901694,-0.49999997,-0.43388855,-0.86266851,-0.2598919,-0.30901697,-0.80901694,-0.49999997,-0.16245985,-0.95105654,-0.26286554,0.16062203,-0.69378048,-0.70204645,-0.16062203,-0.69378048,-0.70204645,0,-0.85065079,-0.52573109,0.69378048,-0.70204645,-0.16062203,0.58778524,-0.688191,-0.42532539,0.80901694,-0.49999997,-0.30901697,0.2598919,-0.43388855,-0.86266851,0.49999997,-0.30901697,-0.80901694,0.42532539,-0.58778524,-0.688191,0.86266851,-0.2598919,-0.43388855,0.70204645,-0.16062203,-0.69378048,0.688191,-0.42532539,-0.58778524,0.85065079,-0.52573109,0,0.96193826,0,-0.27326649,0.99999994,0,0,0.95105654,-0.26286554,-0.16245985,0.96193826,0,0.27326649,0.95105654,-0.26286554,0.16245985,0.26286554,-0.16245985,0.95105654,0.52573109,0,0.85065079,0.26286554,0.16245985,0.95105654,-0.58778524,-0.688191,0.42532539,-0.42532539,-0.58778524,0.688191,-0.688191,-0.42532539,0.58778524,-0.42532539,-0.58778524,-0.688191,-0.58778524,-0.688191,-0.42532539,-0.688191,-0.42532539,-0.58778524,0.52573109,0,-0.85065079,0.26286554,-0.16245985,-0.95105654,0.26286554,0.16245985,-0.95105654,0.95105654,0.26286554,0.16245985,0.95105654,0.26286554,-0.16245985,0.85065079,0.52573109,0 };
		const int sphere_mesh_triangles[sphere_mesh_triangle_count] = { 2,0,1,1,3,4,4,5,2,2,1,4,8,6,7,7,9,10,10,3,8,8,7,10,13,11,12,12,5,14,14,9,13,13,12,14,4,3,10,10,9,14,14,5,4,4,10,14,15,0,2,2,5,16,16,17,15,15,2,16,12,11,18,18,19,20,20,5,12,12,18,20,23,21,22,22,17,24,24,19,23,23,22,24,16,5,20,20,19,24,24,17,16,16,20,24,25,0,15,15,17,26,26,27,25,25,15,26,22,21,28,28,29,30,30,17,22,22,28,30,33,31,32,32,27,34,34,29,33,33,32,34,26,17,30,30,29,34,34,27,26,26,30,34,35,0,25,25,27,36,36,37,35,35,25,36,32,31,38,38,39,40,40,27,32,32,38,40,43,41,42,42,37,44,44,39,43,43,42,44,36,27,40,40,39,44,44,37,36,36,40,44,1,0,35,35,37,45,45,3,1,1,35,45,42,41,46,46,47,48,48,37,42,42,46,48,49,6,8,8,3,50,50,47,49,49,8,50,45,37,48,48,47,50,50,3,45,45,48,50,51,21,23,23,19,52,52,53,51,51,23,52,18,11,54,54,55,56,56,19,18,18,54,56,59,57,58,58,53,60,60,55,59,59,58,60,52,19,56,56,55,60,60,53,52,52,56,60,61,11,13,13,9,62,62,63,61,61,13,62,7,6,64,64,65,66,66,9,7,7,64,66,69,67,68,68,63,70,70,65,69,69,68,70,62,9,66,66,65,70,70,63,62,62,66,70,71,6,49,49,47,72,72,73,71,71,49,72,46,41,74,74,75,76,76,47,46,46,74,76,79,77,78,78,73,80,80,75,79,79,78,80,72,47,76,76,75,80,80,73,72,72,76,80,81,41,43,43,39,82,82,83,81,81,43,82,38,31,84,84,85,86,86,39,38,38,84,86,89,87,88,88,83,90,90,85,89,89,88,90,82,39,86,86,85,90,90,83,82,82,86,90,91,31,33,33,29,92,92,93,91,91,33,92,28,21,94,94,95,96,96,29,28,28,94,96,99,97,98,98,93,100,100,95,99,99,98,100,92,29,96,96,95,100,100,93,92,92,96,100,103,101,102,102,104,105,105,106,103,103,102,105,108,57,107,107,109,110,110,104,108,108,107,110,112,67,111,111,106,113,113,109,112,112,111,113,105,104,110,110,109,113,113,106,105,105,110,113,114,101,103,103,106,115,115,116,114,114,103,115,111,67,117,117,118,119,119,106,111,111,117,119,121,77,120,120,116,122,122,118,121,121,120,122,115,106,119,119,118,122,122,116,115,115,119,122,123,101,114,114,116,124,124,125,123,123,114,124,120,77,126,126,127,128,128,116,120,120,126,128,130,87,129,129,125,131,131,127,130,130,129,131,124,116,128,128,127,131,131,125,124,124,128,131,132,101,123,123,125,133,133,134,132,132,123,133,129,87,135,135,136,137,137,125,129,129,135,137,139,97,138,138,134,140,140,136,139,139,138,140,133,125,137,137,136,140,140,134,133,133,137,140,102,101,132,132,134,141,141,104,102,102,132,141,138,97,142,142,143,144,144,134,138,138,142,144,145,57,108,108,104,146,146,143,145,145,108,146,141,134,144,144,143,146,146,104,141,141,144,146,68,67,112,112,109,147,147,63,68,68,112,147,107,57,59,59,55,148,148,109,107,107,59,148,54,11,61,61,63,149,149,55,54,54,61,149,147,109,148,148,55,149,149,63,147,147,148,149,78,77,121,121,118,150,150,73,78,78,121,150,117,67,69,69,65,151,151,118,117,117,69,151,64,6,71,71,73,152,152,65,64,64,71,152,150,118,151,151,65,152,152,73,150,150,151,152,88,87,130,130,127,153,153,83,88,88,130,153,126,77,79,79,75,154,154,127,126,126,79,154,74,41,81,81,83,155,155,75,74,74,81,155,153,127,154,154,75,155,155,83,153,153,154,155,98,97,139,139,136,156,156,93,98,98,139,156,135,87,89,89,85,157,157,136,135,135,89,157,84,31,91,91,93,158,158,85,84,84,91,158,156,136,157,157,85,158,158,93,156,156,157,158,58,57,145,145,143,159,159,53,58,58,145,159,142,97,99,99,95,160,160,143,142,142,99,160,94,21,51,51,53,161,161,95,94,94,51,161,159,143,160,160,95,161,161,53,159,159,160,161 };
		
		const int cylinder_mesh_vertex_count = 84;
		const int cylinder_mesh_triangle_count = 48 * 3;
		const float cylinder_mesh_vertices[cylinder_mesh_vertex_count] = { 0,0,0,0.43301266,0,0.25,0.25000003,0,0.43301272,-4.3711388e-08,0,0.49999997,-0.25000009,0,0.43301266,-0.43301272,0,0.24999993,-0.49999997,0,-4.3711388e-08,-0.4330126,0,-0.25000018,-0.24999988,0,-0.43301272,8.7422777e-08,0,-0.49999997,0.25000024,0,-0.4330126,0.43301278,0,-0.24999988,0.49999997,0,8.7422777e-08,0.25000003,2,0.43301272,0.43301266,2,0.25,0,2,0,-4.3711388e-08,2,0.49999997,-0.25000009,2,0.43301266,-0.43301272,2,0.24999993,-0.49999997,2,-4.3711388e-08,-0.4330126,2,-0.25000018,-0.24999988,2,-0.43301272,8.7422777e-08,2,-0.49999997,0.25000024,2,-0.4330126,0.43301278,2,-0.24999988,0.49999997,2,8.7422777e-08,0.5,0,0,0.5,2,0 };
		const int cylinder_mesh_triangles[cylinder_mesh_triangle_count] = { 2,0,1,3,0,2,4,0,3,5,0,4,6,0,5,7,0,6,8,0,7,9,0,8,10,0,9,11,0,10,12,0,11,1,0,12,15,13,14,15,16,13,15,17,16,15,18,17,15,19,18,15,20,19,15,21,20,15,22,21,15,23,22,15,24,23,15,25,24,15,14,25,27,14,26,26,14,1,14,13,1,1,13,2,13,16,2,2,16,3,16,17,3,3,17,4,17,18,4,4,18,5,18,19,5,5,19,6,19,20,6,6,20,7,20,21,7,7,21,8,21,22,8,8,22,9,22,23,9,9,23,10,23,24,10,10,24,11,24,27,11,11,27,26 };
	}

	const GeometryMesh SphereMesh
	{
		RawData::sphere_mesh_vertex_count,
		RawData::sphere_mesh_triangle_count,
		RawData::sphere_mesh_vertices,
		RawData::sphere_mesh_triangles,
	};
	const GeometryMesh CylinderMesh
	{
		RawData::cylinder_mesh_vertex_count,
		RawData::cylinder_mesh_triangle_count,
		RawData::cylinder_mesh_vertices,
		RawData::cylinder_mesh_triangles,
	};
}