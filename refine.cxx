/*
 * examesh.cxx
 *
 *  Created on: Jul. 10, 2019
 *      Author: cfog
 */

#include <ExaMesh.h>
#include <unistd.h>
#include <cstdio>

#include "CubicMesh.h"
#include "UMesh.h"

int main(int argc, char* const argv[]) {
	char opt = EOF;
	emInt nDivs = 1;
	emInt maxCellsPerPart = 1000000;
	char type[10];
	char infix[10];
	char inFileBaseName[1024];
	char cgnsFileName[1024];
	char outFileName[1024];
	bool isInputCGNS = false, isParallel = false;

	sprintf(type, "vtk");
	sprintf(infix, "b8");
	sprintf(outFileName, "/dev/null");
	sprintf(inFileBaseName, "/need/a/file/name");
	sprintf(cgnsFileName, "/need/a/file/name");

	while ((opt = getopt(argc, argv, "c:i:m:n:o:pt:u:")) != EOF) {
		switch (opt) {
			case 'c':
				sscanf(optarg, "%1023s", cgnsFileName);
				isInputCGNS = true;
				break;
			case 'i':
				sscanf(optarg, "%1023s", inFileBaseName);
				break;
			case 'n':
				sscanf(optarg, "%d", &nDivs);
				break;
			case 'm':
				sscanf(optarg, "%d", &maxCellsPerPart);
				break;
			case 'o':
				sscanf(optarg, "%1023s", outFileName);
				break;
			case 'p':
				isParallel = true;
				break;
			case 't':
				sscanf(optarg, "%9s", type);
				break;
			case 'u':
				sscanf(optarg, "%9s", infix);
				break;
		}
	}

	if (isInputCGNS) {
		CubicMesh CMorig(cgnsFileName);
		if (isParallel) {
			CMorig.refineForParallel(nDivs, maxCellsPerPart);
		}
		else {
			double start = exaTime();
			UMesh UMrefined(CMorig, nDivs);
			double time = exaTime() - start;
			size_t cells = UMrefined.numCells();
			fprintf(stderr, "\nDone serial refinement.\n");
			fprintf(stderr, "CPU time for refinement = %5.2F seconds\n", time);
			fprintf(stderr,
							"                          %5.2F million cells / minute\n",
							(cells / 1000000.) / (time / 60));

//			UMrefined.writeUGridFile("/tmp/junk.b8.ugrid");
//			UMrefined.writeVTKFile("/tmp/junk.vtk");
		}
	}
	else {
		UMesh UMorig(inFileBaseName, type, infix);
		if (isParallel) {
			UMorig.refineForParallel(nDivs, maxCellsPerPart);
		}
		if (!isParallel) {
			double start = exaTime();
			UMesh UMrefined(UMorig, nDivs);
			double time = exaTime() - start;
			size_t cells = UMrefined.numCells();
			fprintf(stderr, "\nDone serial refinement.\n");
			fprintf(stderr, "CPU time for refinement = %5.2F seconds\n", time);
			fprintf(stderr,
							"                          %5.2F million cells / minute\n",
							(cells / 1000000.) / (time / 60));
//			UMrefined.writeUGridFile("/tmp/junk.b8.ugrid");
			UMrefined.writeVTKFile("/tmp/junk.vtk");
		}
	}

	printf("Exiting\n");
	exit(0);
}

