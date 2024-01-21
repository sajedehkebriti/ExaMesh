#include </home/kebriti/local/include/metis.h>

#if (HAVE_CGNS == 1)
#include "cgnslib.h"
#endif
//#include "ExaMesh.h"
#include "UMesh.h"
std::vector<std::vector<emInt>> partitionMetis(const UMesh &pEM, emInt iParts, std::vector<emInt> &vaicelltopart); 
void extractPartitions(); 
void mesh2MetisGraphs(const UMesh &pEM,
idx_t xadj[], idx_t adjncy[], idx_t adjwgt[]);

void printPart2Cell (const std::vector<std::vector<idx_t>>& part2cell); 