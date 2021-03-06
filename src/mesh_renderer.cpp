#include "mesh_renderer.h"
#include "molecule_kernel.h"
#include "primitive_geometry_mesh.h"

namespace MarchingCubes
{
    // All following codes are modified from https://graphics.stanford.edu/~mdfisher/MarchingCubes.html
    int edgeTable[256] = {
        0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
        0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
        0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
        0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
        0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
        0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
        0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
        0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
        0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
        0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
        0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
        0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
        0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
        0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
        0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
        0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
        0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
        0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
        0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
        0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
        0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
        0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
        0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
        0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
        0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
        0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
        0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
        0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
        0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
        0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
        0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
        0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0 };

    char triTable[256][16] =
        { {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
        {3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
        {3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
        {3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
        {9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
        {9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
        {2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
        {8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
        {9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
        {4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
        {3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
        {1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
        {4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
        {4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
        {9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
        {5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
        {2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
        {9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
        {0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
        {2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
        {10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
        {4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
        {5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
        {5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
        {9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
        {0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
        {1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
        {10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
        {8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
        {2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
        {7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
        {9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
        {2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
        {11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
        {9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
        {5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
        {11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
        {11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
        {1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
        {9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
        {5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
        {2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
        {0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
        {5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
        {6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
        {3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
        {6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
        {5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
        {1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
        {10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
        {6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
        {8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
        {7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
        {3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
        {5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
        {0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
        {9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
        {8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
        {5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
        {0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
        {6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
        {10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
        {10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
        {8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
        {1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
        {3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
        {0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
        {10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
        {3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
        {6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
        {9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
        {8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
        {3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
        {6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
        {0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
        {10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
        {10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
        {2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
        {7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
        {7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
        {2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
        {1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
        {11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
        {8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
        {0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
        {7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
        {10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
        {2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
        {6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
        {7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
        {2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
        {1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
        {10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
        {10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
        {0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
        {7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
        {6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
        {8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
        {9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
        {6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
        {4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
        {10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
        {8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
        {0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
        {1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
        {8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
        {10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
        {4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
        {10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
        {5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
        {11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
        {9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
        {6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
        {7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
        {3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
        {7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
        {9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
        {3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
        {6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
        {9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
        {1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
        {4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
        {7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
        {6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
        {3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
        {0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
        {6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
        {0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
        {11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
        {6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
        {5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
        {9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
        {1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
        {1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
        {10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
        {0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
        {5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
        {10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
        {11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
        {9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
        {7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
        {2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
        {8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
        {9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
        {9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
        {1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
        {9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
        {9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
        {5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
        {0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
        {10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
        {2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
        {0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
        {0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
        {9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
        {5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
        {3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
        {5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
        {8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
        {0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
        {9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
        {0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
        {1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
        {3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
        {4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
        {9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
        {11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
        {11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
        {2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
        {9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
        {3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
        {1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
        {4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
        {4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
        {0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
        {3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
        {3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
        {0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
        {9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
        {1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1} };

    glm::vec3 VertexInterp(const glm::vec3& p1, const glm::vec3& p2, float valp1, float valp2, float isolevel)
    {
        float t = (isolevel - valp1) / (valp2 - valp1);
        return p2 * t + p1 * (1.0f - t);
    }

    struct GridCell
    {
        glm::vec3 p[8];
        float val[8];
    };

    int Polygonise(const GridCell Grid, const float isovalue, std::vector<int>& Triangles, std::vector<Vertex>& Vertices, const glm::vec3 color)
    {
        glm::vec3 VertexList[12];
        glm::vec3 NewVertexList[12];
        char LocalRemap[12];

        //Determine the index into the edge table which
        //tells us which vertices are inside of the surface
        int CubeIndex = 0;
        if (Grid.val[0] < isovalue) CubeIndex |= 1;
        if (Grid.val[1] < isovalue) CubeIndex |= 2;
        if (Grid.val[2] < isovalue) CubeIndex |= 4;
        if (Grid.val[3] < isovalue) CubeIndex |= 8;
        if (Grid.val[4] < isovalue) CubeIndex |= 16;
        if (Grid.val[5] < isovalue) CubeIndex |= 32;
        if (Grid.val[6] < isovalue) CubeIndex |= 64;
        if (Grid.val[7] < isovalue) CubeIndex |= 128;

        //Cube is entirely in/out of the surface
        if (edgeTable[CubeIndex] == 0)
            return 0;

        //Find the vertices where the surface intersects the cube
        if (edgeTable[CubeIndex] & 1)
            VertexList[0] =
            VertexInterp(Grid.p[0], Grid.p[1], Grid.val[0], Grid.val[1], isovalue);
        if (edgeTable[CubeIndex] & 2)
            VertexList[1] =
            VertexInterp(Grid.p[1], Grid.p[2], Grid.val[1], Grid.val[2], isovalue);
        if (edgeTable[CubeIndex] & 4)
            VertexList[2] =
            VertexInterp(Grid.p[2], Grid.p[3], Grid.val[2], Grid.val[3], isovalue);
        if (edgeTable[CubeIndex] & 8)
            VertexList[3] =
            VertexInterp(Grid.p[3], Grid.p[0], Grid.val[3], Grid.val[0], isovalue);
        if (edgeTable[CubeIndex] & 16)
            VertexList[4] =
            VertexInterp(Grid.p[4], Grid.p[5], Grid.val[4], Grid.val[5], isovalue);
        if (edgeTable[CubeIndex] & 32)
            VertexList[5] =
            VertexInterp(Grid.p[5], Grid.p[6], Grid.val[5], Grid.val[6], isovalue);
        if (edgeTable[CubeIndex] & 64)
            VertexList[6] =
            VertexInterp(Grid.p[6], Grid.p[7], Grid.val[6], Grid.val[7], isovalue);
        if (edgeTable[CubeIndex] & 128)
            VertexList[7] =
            VertexInterp(Grid.p[7], Grid.p[4], Grid.val[7], Grid.val[4], isovalue);
        if (edgeTable[CubeIndex] & 256)
            VertexList[8] =
            VertexInterp(Grid.p[0], Grid.p[4], Grid.val[0], Grid.val[4], isovalue);
        if (edgeTable[CubeIndex] & 512)
            VertexList[9] =
            VertexInterp(Grid.p[1], Grid.p[5], Grid.val[1], Grid.val[5], isovalue);
        if (edgeTable[CubeIndex] & 1024)
            VertexList[10] =
            VertexInterp(Grid.p[2], Grid.p[6], Grid.val[2], Grid.val[6], isovalue);
        if (edgeTable[CubeIndex] & 2048)
            VertexList[11] =
            VertexInterp(Grid.p[3], Grid.p[7], Grid.val[3], Grid.val[7], isovalue);

        int NewVertexCount = 0;
        for (int i = 0; i < 12; i++)
            LocalRemap[i] = -1;

        for (int i = 0; triTable[CubeIndex][i] != -1; i++)
        {
            if (LocalRemap[triTable[CubeIndex][i]] == -1)
            {
                NewVertexList[NewVertexCount] = VertexList[triTable[CubeIndex][i]];
                LocalRemap[triTable[CubeIndex][i]] = NewVertexCount;
                NewVertexCount++;
            }
        }

        for (int i = 0; i < NewVertexCount; i++) {
            Vertices.push_back(Vertex{ NewVertexList[i], color, glm::vec3{0,0,0}, 1 });
        }

        int TriangleCount = 0;
        for (int i = 0; triTable[CubeIndex][i] != -1; i += 3) {
            Triangles.push_back(LocalRemap[triTable[CubeIndex][i + 0]]);
            Triangles.push_back(LocalRemap[triTable[CubeIndex][i + 1]]);
            Triangles.push_back(LocalRemap[triTable[CubeIndex][i + 2]]);
            TriangleCount++;
        }

        return TriangleCount;
    }
}

namespace MeshRenderer
{
    const float bond_radius = 0.3;

    bool renderMolecule(const MoleculeStruct::MolecularDataOneFrame* const frame,
                        std::vector<Vertex>& out_vertices,
                        std::vector<uint32_t>& out_indices)
    {
        int triangle_offset = 0;
        for (int i_atom = 0; i_atom < frame->n_atom; i_atom++)
        {
            if (triangle_offset + PrimitiveGeometryMesh::SphereMesh.vertex_count_times_three / 3 > UINT32_MAX)
            {
                std::cout << "Too many vertices in your mesh!" << std::endl;
                return false;
            }

            glm::vec3 center_position{ frame->atoms[i_atom].xyz[0],frame->atoms[i_atom].xyz[1],frame->atoms[i_atom].xyz[2], };
            glm::vec3 color{ frame->atoms[i_atom].rgb[0],frame->atoms[i_atom].rgb[1],frame->atoms[i_atom].rgb[2], };
            float radius = frame->atoms[i_atom].vdw_radius;

            for (int i_vertex = 0; i_vertex < PrimitiveGeometryMesh::SphereMesh.vertex_count_times_three / 3; i_vertex++)
            {
                glm::vec3 vertex_object_space{ PrimitiveGeometryMesh::SphereMesh.vertices[i_vertex * 3 + 0],
                                               PrimitiveGeometryMesh::SphereMesh.vertices[i_vertex * 3 + 1],
                                               PrimitiveGeometryMesh::SphereMesh.vertices[i_vertex * 3 + 2], };
                glm::vec3 vertex_global_space = center_position + radius * vertex_object_space;
                out_vertices.push_back(Vertex{ vertex_global_space, color, vertex_object_space, 0 });
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
                if (triangle_offset + PrimitiveGeometryMesh::CylinderMesh.vertex_count_times_three / 3 > UINT32_MAX)
                {
                    std::cout << "Too many vertices in your mesh!" << std::endl;
                    return false;
                }

                glm::vec3 bond_direction = glm::normalize(center[1 - i_half_bond] - center[i_half_bond]);
                glm::mat3 bond_direction_rotation{ glm::normalize(glm::cross(glm::vec3(0,0,1), bond_direction)), bond_direction, glm::normalize(glm::cross(glm::cross(glm::vec3(0,0,1), bond_direction), bond_direction)) };

                for (int i_vertex = 0; i_vertex < PrimitiveGeometryMesh::CylinderMesh.vertex_count_times_three / 3; i_vertex++)
                {
                    glm::vec3 vertex_object_space{ PrimitiveGeometryMesh::CylinderMesh.vertices[i_vertex * 3 + 0],
                                                   PrimitiveGeometryMesh::CylinderMesh.vertices[i_vertex * 3 + 1],
                                                   PrimitiveGeometryMesh::CylinderMesh.vertices[i_vertex * 3 + 2], };
                    glm::vec3 vertex_global_space = center[i_half_bond] + bond_direction_rotation * bond_scale * vertex_object_space;

                    // Assumption: top and bottom faces are never saw, so ignore them.
                    glm::vec3 normal_object_space{ vertex_object_space.x, 0, vertex_object_space.z };
                    glm::vec3 normal_global_space = bond_direction_rotation * normal_object_space;

                    out_vertices.push_back(Vertex{ vertex_global_space, color[i_half_bond], normal_global_space, 0 });
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

    const float bounding_box_additional_extension = 3.0f;
    const float top_level_minimal_resolution = 0.5f;
    const int octree_level = 3;
    const float isosurface_threshold = 0.08f;
    const glm::vec3 orbital_color[2]{ glm::vec3(1,0,0), glm::vec3(0,0,1) };

    bool renderOrbitalRecursive(const MoleculeStruct::MolecularDataOneFrame* const frame,
                                const glm::vec3 voxel_origin,
                                const glm::vec3 voxel_unit_cell,
                                const int voxel_grid_dimension[3],
                                const float isovalue,
                                const int layer,
                                std::vector<Vertex>& out_vertices,
                                std::vector<uint32_t>& out_indices)
    {
        float* evaluation_pool = new float[(voxel_grid_dimension[0] + 1) * (voxel_grid_dimension[1] + 1) * (voxel_grid_dimension[2] + 1)];

        for (int i_x = 0; i_x < voxel_grid_dimension[0] + 1; i_x++)
            for (int i_y = 0; i_y < voxel_grid_dimension[1] + 1; i_y++)
                for (int i_z = 0; i_z < voxel_grid_dimension[2] + 1; i_z++)
                {
                    int i_pool = i_x * (voxel_grid_dimension[1] + 1) * (voxel_grid_dimension[2] + 1) + i_y * (voxel_grid_dimension[2] + 1) + i_z;
                    float evulation_position[3]{ voxel_origin.x + i_x * voxel_unit_cell.x,
                                                 voxel_origin.y + i_y * voxel_unit_cell.y,
                                                 voxel_origin.z + i_z * voxel_unit_cell.z, };
                    evaluation_pool[i_pool] = MoleculeKernel::evaluateOrbital(evulation_position, frame);
                }

        for (int i_x = 0; i_x < voxel_grid_dimension[0]; i_x++)
            for (int i_y = 0; i_y < voxel_grid_dimension[1]; i_y++)
                for (int i_z = 0; i_z < voxel_grid_dimension[2]; i_z++)
                {
                    float evaluation_voxel[8];
                    for (int i_x_voxel = 0; i_x_voxel < 2; i_x_voxel++)
                        for (int i_y_voxel = 0; i_y_voxel < 2; i_y_voxel++)
                            for (int i_z_voxel = 0; i_z_voxel < 2; i_z_voxel++)
                            {
                                int i_pool = (i_x + i_x_voxel) * (voxel_grid_dimension[1] + 1) * (voxel_grid_dimension[2] + 1)
                                    + (i_y + i_y_voxel) * (voxel_grid_dimension[2] + 1)
                                    + i_z + i_z_voxel;
                                int i_voxel = i_x_voxel * 4 + i_y_voxel * 2 + i_z_voxel;

                                evaluation_voxel[i_voxel] = evaluation_pool[i_pool];
                            }

                    float evaluation_max = evaluation_voxel[0], evaluation_min = evaluation_voxel[0];
                    for (int i = 1; i < 8; i++)
                    {
                        evaluation_max = fmaxf(evaluation_max, evaluation_voxel[i]);
                        evaluation_min = fminf(evaluation_min, evaluation_voxel[i]);
                    }

                    for (int i_sign = 1; i_sign >= -1; i_sign -= 2)
                        if (evaluation_min < isovalue * i_sign && isovalue * i_sign < evaluation_max)
                        {
                            glm::vec3 evulation_position = voxel_origin + glm::vec3(i_x, i_y, i_z) * voxel_unit_cell;

                            if (layer == 0) // Resolve
                            {
                                MarchingCubes::GridCell voxel{};
                                for (int i_x_voxel = 0; i_x_voxel < 2; i_x_voxel++)
                                    for (int i_y_voxel = 0; i_y_voxel < 2; i_y_voxel++)
                                        for (int i_z_voxel = 0; i_z_voxel < 2; i_z_voxel++)
                                        {
                                            int i_voxel = i_x_voxel * 4 + i_y_voxel * 2 + i_z_voxel;
                                            voxel.p[i_voxel] = evulation_position + glm::vec3(i_x_voxel, i_y_voxel, i_z_voxel) * voxel_unit_cell;
                                            voxel.val[i_voxel] = evaluation_voxel[i_voxel];
                                        }

                                std::vector<int> triangles_temp;
                                int vertices_count_before = out_vertices.size();
                                int n_new_triangles = MarchingCubes::Polygonise(voxel,
                                    isovalue * i_sign,
                                    triangles_temp,
                                    out_vertices,
                                    i_sign > 0 ? orbital_color[0] : orbital_color[1]);

                                if (n_new_triangles * 3 + vertices_count_before > UINT32_MAX)
                                {
                                    std::cout << "Too many vertices in your mesh!" << std::endl;
                                    delete[] evaluation_pool;
                                    return false;
                                }

                                for (int i = 0; i < n_new_triangles; i++)
                                {
                                    out_indices.push_back(triangles_temp[i * 3 + 0] + vertices_count_before);
                                    out_indices.push_back(triangles_temp[i * 3 + 1] + vertices_count_before);
                                    out_indices.push_back(triangles_temp[i * 3 + 2] + vertices_count_before);
                                }
                            }
                            else // Recursive
                            {
                                int unitcell_division[3]{ 2,2,2 };

                                bool success = renderOrbitalRecursive(frame,
                                    evulation_position,
                                    voxel_unit_cell * 0.5f,
                                    unitcell_division,
                                    isovalue,
                                    layer - 1,
                                    out_vertices,
                                    out_indices);

                                if (!success)
                                {
                                    delete[] evaluation_pool;
                                    return false;
                                }
                            }
                        }
                }

        delete[] evaluation_pool;

        return true;
    }

    bool renderOrbital(const MoleculeStruct::MolecularDataOneFrame* const frame,
                       std::vector<Vertex>& out_vertices,
                       std::vector<uint32_t>& out_indices)
    {
        float bounding_box[2][3]; // min, max
        if (frame->n_atom > 0)
            for (int i_xyz = 0; i_xyz < 3; i_xyz++)
                bounding_box[0][i_xyz] = frame->atoms[0].xyz[i_xyz], bounding_box[1][i_xyz] = frame->atoms[0].xyz[i_xyz];

        for (int i_atom = 1; i_atom < frame->n_atom; i_atom++)
            for (int i_xyz = 0; i_xyz < 3; i_xyz++)
            {
                bounding_box[0][i_xyz] = fminf(bounding_box[0][i_xyz], frame->atoms[i_atom].xyz[i_xyz]);
                bounding_box[1][i_xyz] = fmaxf(bounding_box[1][i_xyz], frame->atoms[i_atom].xyz[i_xyz]);
            }

        float bounding_box_origin[3], bounding_box_extension[3], bounding_box_grid_unitlength[3];
        int top_level_grid_dimension[3];
        for (int i_xyz = 0; i_xyz < 3; i_xyz++)
        {
            bounding_box_origin[i_xyz] = bounding_box[0][i_xyz] - bounding_box_additional_extension;
            bounding_box_extension[i_xyz] = bounding_box[1][i_xyz] - bounding_box[0][i_xyz] + 2 * bounding_box_additional_extension;
            top_level_grid_dimension[i_xyz] = ceilf(bounding_box_extension[i_xyz] / top_level_minimal_resolution);
            bounding_box_grid_unitlength[i_xyz] = bounding_box_extension[i_xyz] / top_level_grid_dimension[i_xyz];
        }
        // printf("Top level grid: %d, %d, %d\n", top_level_grid_dimension[0], top_level_grid_dimension[1], top_level_grid_dimension[2]);
        glm::vec3 bounding_box_origin_v3{ bounding_box_origin[0], bounding_box_origin[1], bounding_box_origin[2], };
        glm::vec3 bounding_box_grid_unitlength_v3{ bounding_box_grid_unitlength[0], bounding_box_grid_unitlength[1], bounding_box_grid_unitlength[2], };

        return renderOrbitalRecursive(frame,
            bounding_box_origin_v3,
            bounding_box_grid_unitlength_v3,
            top_level_grid_dimension,
            isosurface_threshold,
            octree_level - 1,
            out_vertices,
            out_indices);
    }
}