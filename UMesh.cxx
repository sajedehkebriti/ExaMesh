//  Copyright 2019 by Carl Ollivier-Gooch.  The University of British
//  Columbia disclaims all copyright interest in the software ExaMesh.//
//
//  This file is part of ExaMesh.
//
//  ExaMesh is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  ExaMesh is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with ExaMesh.  If not, see <https://www.gnu.org/licenses/>.

/*
 * UMesh.cxx
 *
 *  Created on: Jul. 2, 2019
 *      Author: cfog
 */

#include <iostream>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <memory>
#include <set>
#include <vector>
// #include <execution>
#include <string.h>
#include <map>
#include <fstream>
#include "BdryTriDivider.h"
#include <chrono>
#include <thread>
// This include file is deliberately before ExaMesh headers so
// there aren't warnings about standard autoconf things being
// redefined.
#include "GMGW_FileWrapper.hxx"

#include "mpi.h"
#include "ExaMesh.h"
#include "exa-defs.h"
#include "UMesh.h"
#include <cstdint>
#include <cstddef>
#if (HAVE_CGNS == 1)
#include <cgnslib.h>
#endif

#ifndef BDRY_TRI
#define BDRY_TRI 5
#define BDRY_QUAD 9
#define TET 10
#define PYRAMID 14
#define PRISM 13
#define HEX 12
#endif

using std::cout;
using std::endl;

#ifndef NDEBUG

// void printTris(const exa_set<TriFaceVerts>  &tris, emInt nDivs){

// 	//cout<<"checking for tris: "<<endl;
// 	//for(auto i=0 ; i<tris.size(); i++){

// 		cout<<"size of set: "<< tris.size()<<endl;
// 		std::cout<<"-----------------------------------------------------------"<<std::endl;
// 		for(auto itr=tris.begin(); itr!=tris.end();itr++){
// 			std::cout<<"Part: "<< itr->getPartid()<<
// 			" local indices: "<<itr->getCorner(0)<<" "<<
// 			itr->getCorner(1)<<" "<<itr->getCorner(2)<<
// 			// " global: "<<itr->getGlobalSorted(0)<<" "<<
// 			// itr->getGlobalSorted(1)<<" "<<itr->getGlobalSorted(2)<<
// 			" Unsorted global: "<<itr->getGlobalCorner(0)<<" "<<
// 			 itr->getGlobalCorner(1)<<" "<<itr->getGlobalCorner(2)<<
// 		 	" Remote ID: "<<itr->getRemotePartid()<<
// 			" Remote Indices: "<<itr->getRemoteIndices(0)<<" "<<
// 			itr->getRemoteIndices(1)<<" "<<
// 			itr->getRemoteIndices(2)<<
// 			//" boolean value: "<<itr->getGlobalCompare()<<
// 			std::endl;
// 			std::cout<<"Refined verts: "<<std::endl;
// 			for (int ii = 0; ii <= nDivs ; ii++) {
// 	 			for (int jj = 0; jj <= nDivs-ii ; jj++) {

// 	 				 std::cout<<itr->getIntVertInd(ii,jj)<<" ";
// 	 			}
// 			}

// 			std::cout<<std::endl;
// 		}
// 	//}

// }

void printTrisSet(const std::set<TriFaceVerts> &tris, std::string fileName)
{
	std::ofstream out;
	out.open(fileName);

	for (auto itr = tris.begin(); itr != tris.end(); itr++)
	{
		out << "part: " << itr->getPartid() << " local: " << itr->getSorted(0) << " " << itr->getSorted(1) << " " << itr->getSorted(2) << " global: " << itr->getGlobalSorted(0) << " " << itr->getGlobalSorted(1) << " " << itr->getGlobalSorted(2) <<
			// " Unsorted global: "<<itr->getGlobalCorner(0)<<" "<<
			// itr->getGlobalCorner(1)<<" "<<itr->getGlobalCorner(2)<<
			// " Remote ID: "<<itr->getRemotePartid()<<
			// " Get Corner: "<<itr->getCorner(0)<<" "<<itr->getCorner(1)<<
			// itr->getCorner(2)<<
			endl;
	}
}

// void printQuads(const std::vector<std::vector<QuadFaceVerts>>  &quads,
// emInt nParts){

// 	 cout<<" checking quads: "<<endl;
// 	for(auto i=0 ; i< nParts; i++){
// 		//cout<<"part id: "<< i<<endl;
// 		for(auto j=0; j<quads[i].size(); j++){
// 			cout<<"part: "<< quads[i][j].getPartid()<<
// 			" local: "<<quads[i][j].getSorted(0)<<" "<<
// 			quads[i][j].getSorted(1)<<" "<<quads[i][j].getSorted(2)<<" "<<
// 			quads[i][j].getSorted(3)<<
// 			" global: "<<quads[i][j].getGlobalSorted(0)<<" "<<
// 			quads[i][j].getGlobalSorted(1)<<" "<<quads[i][j].getGlobalSorted(2)<<
// 			" "<<quads[i][j].getGlobalSorted(3)<<" "<<
// 		 	" Remote ID: "<<quads[i][j].getRemotePartid()<<endl;

// 		}

// 	}

// }

void printMapFaceVerts(const std::unordered_map<TriFaceVerts, std::set<emInt>>
						   &map)
{
	for (auto itr = map.begin(); itr != map.end(); itr++)
	{
		std::cout << "Face from part ID " << (itr->first).getPartid() << ": " << (itr->first).getCorner(0) << " " << (itr->first).getCorner(1) << " " << (itr->first).getCorner(2) << " Remote from Remote ID " << (itr->first).getRemoteId() << ": " << (itr->first).getRemoteIndices(0) << " " << (itr->first).getRemoteIndices(1) << " " << (itr->first).getRemoteIndices(2) << std::endl;
		for (auto itset = (itr->second).begin(); itset != (itr->second).end();
			 itset++)
		{
			std::cout << *itset << " ";
		}
		std::cout << std::endl;
	}
}

static bool memoryCheck(void *address, int nBytes)
{
	char *checkPtr = reinterpret_cast<char *>(address);
	bool retVal = true;
	for (int ii = 0; ii < nBytes; ii++)
	{
		retVal = retVal && (checkPtr[ii] == 0x00);
	}
	return retVal;
}
#endif

void UMesh::init(const emInt nVerts, const emInt nBdryVerts,
				 const emInt nBdryTris, const emInt nBdryQuads, const emInt nTets,
				 const emInt nPyramids, const emInt nPrisms, const emInt nHexes)
{
	m_nVerts = nVerts;
	m_nBdryVerts = nBdryVerts;
	m_nTris = nBdryTris;
	m_nQuads = nBdryQuads;
	m_nTets = nTets;
	m_nPyrs = nPyramids;
	m_nPrisms = nPrisms;
	m_nHexes = nHexes;

	// All sizes are computed in bytes.
	// Work out buffer size, including padding to ensure 8-byte alignment for the coordinates.
	size_t intSize = sizeof(emInt);
	size_t headerSize = 7 * intSize;
	// How many bytes to add to get eight-byte alignment for coordinates,
	// assuming eight-byte alignment for the buffer overall.
	size_t slack1Size = (intSize == 4) ? 4 : 0;
	size_t coordSize = 3 * sizeof(double) * nVerts;
	size_t connSize = (3 * nBdryTris + 4 * nBdryQuads + 4 * size_t(nTets) + 5 * size_t(nPyramids) + 6 * size_t(nPrisms) + 8 * size_t(nHexes)) * intSize;
	size_t BCSize = (nBdryTris + nBdryQuads) * intSize;

	// How many bytes to add to fill up the last eight-byte chunk?
	size_t slack2Size =
		((((connSize + BCSize) / 8 + 1) * 8) - (connSize + BCSize)) % 8;
	size_t bufferBytes = headerSize + coordSize + connSize + BCSize + slack1Size + slack2Size;
	assert((headerSize + slack1Size) % 8 == 0);
	assert((connSize + BCSize + slack2Size) % 8 == 0);
	assert(bufferBytes % 8 == 0);
	size_t bufferWords = bufferBytes / 8;
	// Use words instead of bytes to ensure 8-byte alignment.
	m_buffer = reinterpret_cast<char *>(calloc(bufferWords, 8));

	// The pointer arithmetic here is made more complicated because the pointers aren't
	// compatible with each other.
	m_header = reinterpret_cast<emInt *>(m_buffer + slack1Size);
	std::fill(m_header, m_header + 7, 0);
	m_coords =
		reinterpret_cast<double(*)[3]>(m_buffer + headerSize + slack1Size);
	m_TriConn = reinterpret_cast<emInt(*)[3]>(m_buffer + headerSize + slack1Size + coordSize);
	m_QuadConn = reinterpret_cast<emInt(*)[4]>(m_TriConn + nBdryTris);
	m_TriBC = reinterpret_cast<emInt *>(m_QuadConn + nBdryQuads);
	m_QuadBC = m_TriBC + nBdryTris;
	m_TetConn = reinterpret_cast<emInt(*)[4]>(m_QuadBC + nBdryQuads);
	m_PyrConn = reinterpret_cast<emInt(*)[5]>(m_TetConn + nTets);
	m_PrismConn = reinterpret_cast<emInt(*)[6]>(m_PyrConn + nPyramids);
	m_HexConn = reinterpret_cast<emInt(*)[8]>(m_PrismConn + nPrisms);
	m_fileImage = m_buffer + slack1Size;
	m_fileImageSize = bufferBytes - slack1Size - slack2Size;

	//	printf("Diagnostics for UMesh data struct:\n");
	//	printf("Buffer size, in bytes:     %lu\n", bufferBytes);
	//	printf("File image size, in bytes: %lu\n", m_fileImageSize);
	//	printf("Num verts: %10u\n", nVerts);
	//	printf("Num tris:  %10u\n", nBdryTris);
	//	printf("Num quads: %10u\n", nBdryQuads);
	//	printf("Num tets:  %10u\n", nTets);
	//	printf("Num pyrs:  %10u\n", nPyramids);
	//	printf("Num prisms: %8u\n", nPrisms);
	//	printf("Num hexes: %10u\n", nHexes);
	//	printf(
	//			"Coord offset: %10lu\n",
	//			reinterpret_cast<char*>(m_coords) - reinterpret_cast<char*>(m_header));
	//	printf(
	//			"Tri conn offset: %10lu\n",
	//			reinterpret_cast<char*>(m_TriConn) - reinterpret_cast<char*>(m_header));
	//	printf("Tri BC offset: %10lu\n",
	//					reinterpret_cast<char*>(m_TriBC) - reinterpret_cast<char*>(m_header));
	//	printf(
	//			"Tet conn offset: %10lu\n",
	//			reinterpret_cast<char*>(m_TetConn) - reinterpret_cast<char*>(m_header));

	m_lenScale = new double[m_nVerts];
	for (emInt ii = 0; ii < m_nVerts; ii++)
	{
		m_lenScale[ii] = 0;
	}
}

UMesh::UMesh(const emInt nVerts, const emInt nBdryVerts, const emInt nBdryTris,
			 const emInt nBdryQuads, const emInt nTets, const emInt nPyramids,
			 const emInt nPrisms, const emInt nHexes) : m_nVerts(nVerts), m_nBdryVerts(nBdryVerts), m_nTris(nBdryTris),
														m_nQuads(nBdryQuads), m_nTets(nTets), m_nPyrs(nPyramids),
														m_nPrisms(nPrisms), m_nHexes(nHexes), m_fileImageSize(0),
														m_header(nullptr), m_coords(nullptr), m_TriConn(nullptr),
														m_QuadConn(nullptr), m_TetConn(nullptr), m_PyrConn(nullptr),
														m_PrismConn(nullptr), m_HexConn(nullptr), m_buffer(nullptr),
														m_fileImage(nullptr)
{

	// All sizes are computed in bytes.

	// Work out buffer size, including padding to ensure 8-byte alignment for the coordinates.
	init(nVerts, nBdryVerts, nBdryTris, nBdryQuads, nTets, nPyramids, nPrisms,
		 nHexes);
}

emInt UMesh::addVert(const double newCoords[3])
{
	assert(m_header[eVert] < m_nVerts);
	assert(memoryCheck(m_coords[m_header[eVert]], 24));
	m_coords[m_header[eVert]][0] = newCoords[0];
	m_coords[m_header[eVert]][1] = newCoords[1];
	m_coords[m_header[eVert]][2] = newCoords[2];
	return (m_header[eVert]++);
}

emInt UMesh::addBdryTri(const emInt verts[3])
{
	assert(memoryCheck(m_TriConn[m_header[eTri]], 3 * sizeof(emInt)));
	for (int ii = 0; ii < 3; ii++)
	{
		assert(verts[ii] < m_header[eVert]);
		m_TriConn[m_header[eTri]][ii] = verts[ii];
	}
	return (m_header[eTri]++);
}

emInt UMesh::addBdryQuad(const emInt verts[4])
{
	assert(memoryCheck(m_QuadConn[m_header[eQuad]], 4 * sizeof(emInt)));
	for (int ii = 0; ii < 4; ii++)
	{
		assert(verts[ii] < m_header[eVert]);
		m_QuadConn[m_header[eQuad]][ii] = verts[ii];
	}
	return (m_header[eQuad]++);
}

emInt UMesh::addTet(const emInt verts[4])
{
#ifndef OLD_ADD_ELEMENT
	emInt thisTetInd = m_header[eTet]++;
	emInt *thisConn = m_TetConn[thisTetInd];
	assert(memoryCheck(thisConn, 4 * sizeof(emInt)));
	std::copy(verts, verts + 4, thisConn);
	return thisTetInd;
#else
	assert(memoryCheck(m_TetConn[m_header[eTet]], 4 * sizeof(emInt)));
	for (int ii = 0; ii < 4; ii++)
	{
		assert(verts[ii] < m_header[eVert]);
		m_TetConn[m_header[eTet]][ii] = verts[ii];
	}
	return (m_header[eTet]++);
#endif
}

emInt UMesh::addPyramid(const emInt verts[5])
{
#ifndef OLD_ADD_ELEMENT
	emInt thisPyrInd = m_header[ePyr]++;
	emInt *thisConn = m_PyrConn[thisPyrInd];
	assert(memoryCheck(thisConn, 5 * sizeof(emInt)));
	std::copy(verts, verts + 5, thisConn);
	return thisPyrInd;
#else
	assert(memoryCheck(m_PyrConn[m_header[ePyr]], 5 * sizeof(emInt)));
	for (int ii = 0; ii < 5; ii++)
	{
		assert(verts[ii] < m_header[eVert]);
		m_PyrConn[m_header[ePyr]][ii] = verts[ii];
	}
	return (m_header[ePyr]++);
#endif
}

emInt UMesh::addPrism(const emInt verts[6])
{
#ifndef OLD_ADD_ELEMENT
	emInt thisPrismInd = m_header[ePrism]++;
	emInt *thisConn = m_PrismConn[thisPrismInd];
	assert(memoryCheck(thisConn, 6 * sizeof(emInt)));
	std::copy(verts, verts + 6, thisConn);
	return thisPrismInd;
#else
	assert(memoryCheck(m_PrismConn[m_header[ePrism]], 6 * sizeof(emInt)));
	for (int ii = 0; ii < 6; ii++)
	{
		assert(verts[ii] < m_header[eVert]);
		m_PrismConn[m_header[ePrism]][ii] = verts[ii];
	}
	return (m_header[ePrism]++);
#endif
}
#define OLD_ADD_ELEMENT
emInt UMesh::addHex(const emInt verts[8])
{
#ifndef OLD_ADD_ELEMENT
	emInt thisHexInd = m_header[eHex]++;
	emInt *thisConn = m_HexConn[thisHexInd];
	assert(memoryCheck(thisConn, 8 * sizeof(emInt)));
	std::copy(verts, verts + 8, thisConn);
	return thisHexInd;
#else
	assert(memoryCheck(m_HexConn[m_header[eHex]], 8 * sizeof(emInt)));
	for (int ii = 0; ii < 8; ii++)
	{
		assert(verts[ii] < m_header[eVert]);
		m_HexConn[m_header[eHex]][ii] = verts[ii];
	}
	return (m_header[eHex]++);
#endif
}

UMesh::~UMesh()
{
	free(m_buffer);
}

void checkConnectivitySize(const char cellType, const emInt nVerts)
{
	emInt expected[] = {0, 0, 0, 0, 0, 3, 0, 0, 0, 4, 4, 0, 8, 6, 5};
	if (expected[int(cellType)] != nVerts)
	{
		fprintf(
			stderr,
			"Error reading mesh file.  Cell type %d expects %u verts; found %u.\n",
			cellType, expected[int(cellType)], nVerts);
		exit(1);
	}
}

class vertTriple
{
	emInt corners[3];

public:
	vertTriple(const emInt vA, const emInt vB, const emInt vC)
	{
		corners[0] = vA;
		corners[1] = vB;
		corners[2] = vC;
	}
	bool operator<(const vertTriple &that) const
	{
		emInt thisTemp[3], thatTemp[3];
		sortVerts3(corners, thisTemp);
		sortVerts3(that.corners, thatTemp);
		return (thisTemp[0] < thatTemp[0] || (thisTemp[0] == thatTemp[0] && thisTemp[1] < thatTemp[1]) || (thisTemp[0] == thatTemp[0] && thisTemp[1] == thatTemp[1] && thisTemp[2] < thatTemp[2]));
	}
	const emInt *getCorners() const
	{
		return corners;
	}
};

class vertQuadruple
{
	emInt corners[4];

public:
	vertQuadruple(const emInt vA, const emInt vB, const emInt vC,
				  const emInt vD)
	{
		corners[0] = vA;
		corners[1] = vB;
		corners[2] = vC;
		corners[3] = vD;
	}
	bool operator<(const vertQuadruple &that) const
	{
		emInt thisTemp[4], thatTemp[4];
		sortVerts4(corners, thisTemp);
		sortVerts4(that.corners, thatTemp);
		return (thisTemp[0] < thatTemp[0] || (thisTemp[0] == thatTemp[0] && thisTemp[1] < thatTemp[1]) || (thisTemp[0] == thatTemp[0] && thisTemp[1] == thatTemp[1] && thisTemp[2] < thatTemp[2]) || (thisTemp[0] == thatTemp[0] && thisTemp[1] == thatTemp[1] && thisTemp[2] == thatTemp[2] && thisTemp[3] < thatTemp[3]));
	}
	const emInt *getCorners() const
	{
		return corners;
	}
};

void updateTriSet(std::set<vertTriple> &triSet, const emInt v0, const emInt v1,
				  const emInt v2)
{
	vertTriple VT(v0, v1, v2);
	typename std::set<vertTriple>::iterator vertIter, VIend = triSet.end();

	vertIter = triSet.find(VT);
	if (vertIter == VIend)
	{
		triSet.insert(VT);
	}
	else
	{
		triSet.erase(vertIter);
	}
}

void updateQuadSet(std::set<vertQuadruple> &quadSet, const emInt v0,
				   const emInt v1, const emInt v2, const emInt v3)
{
	vertQuadruple VQ(v0, v1, v2, v3);
	typename std::set<vertQuadruple>::iterator vertIter, VQend = quadSet.end();

	vertIter = quadSet.find(VQ);
	if (vertIter == VQend)
	{
		quadSet.insert(VQ);
	}
	else
	{
		quadSet.erase(vertIter);
	}
}

UMesh::UMesh(const char baseFileName[], const char type[],
			 const char ugridInfix[]) : m_nVerts(0), m_nBdryVerts(0), m_nTris(0), m_nQuads(0), m_nTets(0),
										m_nPyrs(0), m_nPrisms(0), m_nHexes(0), m_fileImageSize(0),
										m_header(nullptr), m_coords(nullptr), m_TriConn(nullptr),
										m_QuadConn(nullptr), m_TetConn(nullptr), m_PyrConn(nullptr),
										m_PrismConn(nullptr), m_HexConn(nullptr), m_buffer(nullptr),
										m_fileImage(nullptr)
{
	// Use the same IO routines as the mesh analyzer code from GMGW.
	FileWrapper *reader = FileWrapper::factory(baseFileName, type, ugridInfix);

	reader->scanFile();

	// Identify any bdry tris and quads that aren't in the file

	emInt numBdryTris = reader->getNumBdryTris();
	emInt numBdryQuads = reader->getNumBdryQuads();

	std::set<vertTriple> setTris;
	std::set<vertQuadruple> setQuads;

	reader->seekStartOfConnectivity();
	for (emInt ii = 0; ii < reader->getNumCells(); ii++)
	{
		char cellType = reader->getCellType(ii);
		emInt nConn, connect[8];
		reader->getNextCellConnectivity(nConn, connect);
		checkConnectivitySize(cellType, nConn);
		switch (cellType)
		{
		case BDRY_TRI:
			updateTriSet(setTris, connect[0], connect[1], connect[2]);
			break;
		case BDRY_QUAD:
			updateQuadSet(setQuads, connect[0], connect[1], connect[2], connect[3]);
			break;
		case TET:
			updateTriSet(setTris, connect[0], connect[1], connect[2]);
			updateTriSet(setTris, connect[0], connect[1], connect[3]);
			updateTriSet(setTris, connect[1], connect[2], connect[3]);
			updateTriSet(setTris, connect[2], connect[0], connect[3]);
			break;
		case PYRAMID:
			updateTriSet(setTris, connect[0], connect[1], connect[4]);
			updateTriSet(setTris, connect[1], connect[2], connect[4]);
			updateTriSet(setTris, connect[2], connect[3], connect[4]);
			updateTriSet(setTris, connect[3], connect[0], connect[4]);
			updateQuadSet(setQuads, connect[0], connect[1], connect[2], connect[3]);
			break;
		case PRISM:
			updateTriSet(setTris, connect[0], connect[1], connect[2]);
			updateTriSet(setTris, connect[3], connect[4], connect[5]);
			updateQuadSet(setQuads, connect[0], connect[1], connect[4], connect[3]);
			updateQuadSet(setQuads, connect[1], connect[2], connect[5], connect[4]);
			updateQuadSet(setQuads, connect[2], connect[0], connect[3], connect[5]);
			break;
		case HEX:
			updateQuadSet(setQuads, connect[0], connect[1], connect[2], connect[3]);
			updateQuadSet(setQuads, connect[4], connect[5], connect[6], connect[7]);
			updateQuadSet(setQuads, connect[0], connect[1], connect[5], connect[4]);
			updateQuadSet(setQuads, connect[1], connect[2], connect[6], connect[5]);
			updateQuadSet(setQuads, connect[2], connect[3], connect[7], connect[6]);
			updateQuadSet(setQuads, connect[3], connect[0], connect[4], connect[7]);
			break;
		default:
			assert(0);
		}
	}

	numBdryTris += setTris.size();
	numBdryQuads += setQuads.size();

	init(reader->getNumVerts(), reader->getNumBdryVerts(), numBdryTris,
		 numBdryQuads, reader->getNumTets(), reader->getNumPyramids(),
		 reader->getNumPrisms(), reader->getNumHexes());

	reader->seekStartOfCoords();
	for (emInt ii = 0; ii < m_nVerts; ii++)
	{
		double coords[3];
		reader->getNextVertexCoords(coords[0], coords[1], coords[2]);
		addVert(coords);
	}

	reader->seekStartOfConnectivity();
	for (emInt ii = 0; ii < reader->getNumCells(); ii++)
	{
		char cellType = reader->getCellType(ii);
		emInt nConn, connect[8];
		reader->getNextCellConnectivity(nConn, connect);
		checkConnectivitySize(cellType, nConn);
		switch (cellType)
		{
		case BDRY_TRI:
			addBdryTri(connect);
			break;
		case BDRY_QUAD:
			addBdryQuad(connect);
			break;
		case TET:
			addTet(connect);
			break;
		case PYRAMID:
			addPyramid(connect);
			break;
		case PRISM:
			addPrism(connect);
			break;
		case HEX:
			addHex(connect);
			break;
		default:
			assert(0);
		}
	}

	for (auto VT : setTris)
	{
		const emInt *const corners = VT.getCorners();
		addBdryTri(corners);
	}
	for (auto VQ : setQuads)
	{
		const emInt *const corners = VQ.getCorners();
		addBdryQuad(corners);
	}

	// Now tag all bdry verts
	bool *isBdryVert = new bool[m_nVerts];
	for (emInt ii = 0; ii < m_nVerts; ii++)
	{
		isBdryVert[ii] = false;
	}
	for (emInt iTri = 0; iTri < m_nTris; iTri++)
	{
		isBdryVert[m_TriConn[iTri][0]] = true;
		isBdryVert[m_TriConn[iTri][1]] = true;
		isBdryVert[m_TriConn[iTri][2]] = true;
	}
	for (emInt iQuad = 0; iQuad < m_nQuads; iQuad++)
	{
		isBdryVert[m_QuadConn[iQuad][0]] = true;
		isBdryVert[m_QuadConn[iQuad][1]] = true;
		isBdryVert[m_QuadConn[iQuad][2]] = true;
		isBdryVert[m_QuadConn[iQuad][3]] = true;
	}
	m_nBdryVerts = 0;
	for (emInt ii = 0; ii < m_nVerts; ii++)
	{
		if (isBdryVert[ii])
		{
			m_nBdryVerts++;
		}
	}
	delete[] isBdryVert;

	// If any of these fail, your file was invalid.
	assert(m_nVerts == m_header[eVert]);
	assert(m_nTris == m_header[eTri]);
	assert(m_nQuads == m_header[eQuad]);
	assert(m_nTets == m_header[eTet]);
	assert(m_nPyrs == m_header[ePyr]);
	assert(m_nPrisms == m_header[ePrism]);
	assert(m_nHexes == m_header[eHex]);

	delete reader;

	setupLengthScales();
}

UMesh::UMesh(const UMesh &UMIn, const int nDivs, const emInt partID) : m_nVerts(0), m_nBdryVerts(0), m_nTris(0), m_nQuads(0), m_nTets(0),
																	   m_nPyrs(0), m_nPrisms(0), m_nHexes(0), m_fileImageSize(0),
																	   m_header(nullptr), m_coords(nullptr), m_TriConn(nullptr),
																	   m_QuadConn(nullptr), m_TetConn(nullptr), m_PyrConn(nullptr),
																	   m_PrismConn(nullptr), m_HexConn(nullptr), m_buffer(nullptr),
																	   m_fileImage(nullptr)
{

	setlocale(LC_ALL, "");
	size_t totalInputCells = size_t(UMIn.m_nTets) + UMIn.m_nPyrs + UMIn.m_nPrisms + UMIn.m_nHexes;
	fprintf(
		stderr,
		"Initial mesh has:\n %'15u verts,\n %'15u bdry tris,\n %'15u bdry quads,\n %'15u tets,\n %'15u pyramids,\n %'15u prisms,\n %'15u hexes,\n%'15lu cells total\n",
		UMIn.m_nVerts, UMIn.m_nTris, UMIn.m_nQuads, UMIn.m_nTets, UMIn.m_nPyrs,
		UMIn.m_nPrisms, UMIn.m_nHexes, totalInputCells);

	MeshSize MSOut = UMIn.computeFineMeshSize(nDivs);
	init(MSOut.nVerts, MSOut.nBdryVerts, MSOut.nBdryTris, MSOut.nBdryQuads,
		 MSOut.nTets, MSOut.nPyrs, MSOut.nPrisms, MSOut.nHexes);
	// Copy length scale data from the other mesh.
	for (emInt vv = 0; vv < UMIn.m_nVerts; vv++)
	{
		m_lenScale[vv] = UMIn.m_lenScale[vv];
	}

	subdividePartMesh(&UMIn, this, nDivs, partID);
	setlocale(LC_ALL, "");
	fprintf(
		stderr,
		"Final mesh has:\n %'15u verts,\n %'15u bdry tris,\n %'15u bdry quads,\n %'15u tets,\n %'15u pyramids,\n %'15u prisms,\n %'15u hexes,\n%'15u cells total\n",
		m_nVerts, m_nTris, m_nQuads, m_nTets, m_nPyrs, m_nPrisms, m_nHexes,
		numCells());
}

UMesh::UMesh(const CubicMesh &CMIn, const int nDivs, const emInt partID) : m_nVerts(0), m_nBdryVerts(0), m_nTris(0), m_nQuads(0), m_nTets(0),
																		   m_nPyrs(0), m_nPrisms(0), m_nHexes(0), m_fileImageSize(0),
																		   m_header(nullptr), m_coords(nullptr), m_TriConn(nullptr),
																		   m_QuadConn(nullptr), m_TetConn(nullptr), m_PyrConn(nullptr),
																		   m_PrismConn(nullptr), m_HexConn(nullptr), m_buffer(nullptr),
																		   m_fileImage(nullptr)
{

#ifndef NDEBUG
	setlocale(LC_ALL, "");
	size_t totalInputCells = size_t(CMIn.numTets()) + CMIn.numPyramids() + CMIn.numPrisms() + CMIn.numHexes();
	fprintf(
		stderr,
		"Initial mesh has:\n %'15u verts,\n %'15u bdry tris,\n %'15u bdry quads,\n %'15u tets,\n %'15u pyramids,\n %'15u prisms,\n %'15u hexes,\n%'15lu cells total\n",
		CMIn.numVertsToCopy(), CMIn.numBdryTris(), CMIn.numBdryQuads(),
		CMIn.numTets(), CMIn.numPyramids(), CMIn.numPrisms(), CMIn.numHexes(),
		totalInputCells);
#endif

	MeshSize MSIn, MSOut;
	MSIn.nBdryVerts = CMIn.numBdryVerts();
	MSIn.nVerts = CMIn.numVertsToCopy();
	MSIn.nBdryTris = CMIn.numBdryTris();
	MSIn.nBdryQuads = CMIn.numBdryQuads();
	MSIn.nTets = CMIn.numTets();
	MSIn.nPyrs = CMIn.numPyramids();
	MSIn.nPrisms = CMIn.numPrisms();
	MSIn.nHexes = CMIn.numHexes();
	bool sizesOK = ::computeMeshSize(MSIn, nDivs, MSOut);
	if (!sizesOK)
		exit(2);

	init(MSOut.nVerts, MSOut.nBdryVerts, MSOut.nBdryTris, MSOut.nBdryQuads,
		 MSOut.nTets, MSOut.nPyrs, MSOut.nPrisms, MSOut.nHexes);
	// Copy length scale data from the other mesh.
	for (emInt vv = 0; vv < CMIn.numVertsToCopy(); vv++)
	{
		m_lenScale[vv] = CMIn.getLengthScale(vv);
	}

	subdividePartMesh(&CMIn, this, nDivs, partID);

#ifndef NDEBUG
	setlocale(LC_ALL, "");
	fprintf(
		stderr,
		"Final mesh has:\n %'15u verts,\n %'15u bdry tris,\n %'15u bdry quads,\n %'15u tets,\n %'15u pyramids,\n %'15u prisms,\n %'15u hexes,\n%'15u cells total\n",
		m_nVerts, m_nTris, m_nQuads, m_nTets, m_nPyrs, m_nPrisms, m_nHexes,
		numCells());
#endif
}

bool UMesh::writeVTKFile(const char fileName[])
{
	double timeBefore = exaTime();

	FILE *outFile = fopen(fileName, "w");
	if (!outFile)
	{
		fprintf(stderr, "Couldn't open file %s for writing.  Bummer!\n", fileName);
		return false;
	}

	fprintf(outFile, "# vtk DataFile Version 1.0\n");
	fprintf(outFile, "GRUMMP Tetra example\n");
	fprintf(outFile, "ASCII\n");
	fprintf(outFile, "DATASET UNSTRUCTURED_GRID\n");
	fprintf(outFile, "POINTS %d float\n", m_header[eVert]);

	//-------------------------------------
	// write 3d vertex data
	//-------------------------------------
	for (emInt i = 0; i < m_header[eVert]; ++i)
	{
		fprintf(outFile, "%16.8f %16.8f %16.8f\n", getX(i), getY(i), getZ(i));
	}

	const size_t nTris = numBdryTris();
	const size_t nQuads = numBdryQuads();
	const size_t nTets = numTets();
	const size_t nPyrs = numPyramids();
	const size_t nPrisms = numPrisms();
	const size_t nHexes = numHexes();

	const size_t numEnts = nTris + nQuads + nTets + nPyrs + nPrisms + nHexes;
	const size_t dataSize = 4 * nTris + 5 * (nQuads + nTets) + 6 * nPyrs + 7 * nPrisms + 9 * nHexes;

	fprintf(outFile, "CELLS %lu %lu\n", numEnts, dataSize);

	// Write all the bdry tris
	for (std::size_t i = 0; i < nTris; i++)
	{
		const emInt *verts = getBdryTriConn(i);
		fprintf(outFile, "3 %d %d %d\n", verts[0], verts[1], verts[2]);
	}

	// Write all the bdry quads
	for (std::size_t i = 0; i < nQuads; i++)
	{
		const emInt *verts = getBdryQuadConn(i);
		fprintf(outFile, "4 %d %d %d %d\n", verts[0], verts[1], verts[2], verts[3]);
	}

	// Write all the tets
	for (std::size_t i = 0; i < nTets; i++)
	{
		const emInt *verts = getTetConn(i);
		fprintf(outFile, "4 %d %d %d %d\n", verts[0], verts[1], verts[2], verts[3]);
	}

	// Write all the pyramids
	for (std::size_t i = 0; i < nPyrs; i++)
	{
		const emInt *verts = getPyrConn(i);
		fprintf(outFile, "5 %d %d %d %d %d\n", verts[0], verts[1], verts[2],
				verts[3], verts[4]);
	}

	// Write all the prisms
	for (std::size_t i = 0; i < nPrisms; i++)
	{
		const emInt *verts = getPrismConn(i);
		fprintf(outFile, "6 %d %d %d %d %d %d\n", verts[0], verts[1], verts[2],
				verts[3], verts[4], verts[5]);
	}

	// Write all the hexes
	for (std::size_t i = 0; i < nHexes; i++)
	{
		const emInt *verts = getHexConn(i);
		fprintf(outFile, "8 %d %d %d %d %d %d %d %d\n", verts[0], verts[1],
				verts[2], verts[3], verts[4], verts[5], verts[6], verts[7]);
	}

	//-------------------------------------
	// write cell type (VTK_TRIANGLE = 5, VTK_QUAD = 9,
	// VTK_TETRA = 10, VTK_HEXAHEDRON = 12, VTK_WEDGE = 13,
	// VTK_PYRAMID = 14)
	//-------------------------------------
	fprintf(outFile, "CELL_TYPES %lu\n", numEnts);
	for (std::size_t ct = 0; ct < nTris; ++ct)
		fprintf(outFile, "5\n");
	for (std::size_t ct = 0; ct < nQuads; ++ct)
		fprintf(outFile, "9\n");
	for (std::size_t ct = 0; ct < nTets; ++ct)
		fprintf(outFile, "10\n");
	for (std::size_t ct = 0; ct < nPyrs; ++ct)
		fprintf(outFile, "14\n");
	for (std::size_t ct = 0; ct < nPrisms; ++ct)
		fprintf(outFile, "13\n");
	for (std::size_t ct = 0; ct < nHexes; ++ct)
		fprintf(outFile, "12\n");

	fclose(outFile);
	double timeAfter = exaTime();
	double elapsed = timeAfter - timeBefore;
	size_t totalCells = size_t(m_nTets) + m_nPyrs + m_nPrisms + m_nHexes;
	fprintf(stderr, "CPU time for VTK file write = %5.2F seconds\n", elapsed);
	fprintf(stderr, "                          %5.2F million cells / minute\n",
			(totalCells / 1000000.) / (elapsed / 60));

	return true;
}

void UMesh::incrementVertIndices(emInt *conn, emInt size, int inc)
{
	for (emInt ii = 0; ii < size; ii++)
	{
		conn[ii] += inc;
	}
}

bool UMesh::writeUGridFile(const char fileName[])
{
	double timeBefore = exaTime();

	// Need to increment all vert indices, because UGRID files are 1-based.
	emInt size = m_nTris * 3 + m_nQuads * 4;
	incrementVertIndices(reinterpret_cast<emInt *>(m_TriConn), size, 1);
	size = m_nTets * 4 + m_nPyrs * 5 + m_nPrisms * 6 + m_nHexes * 8;
	incrementVertIndices(reinterpret_cast<emInt *>(m_TetConn), size, 1);

	// Also need to swap verts 2 and 4 for pyramids, because UGRID treats
	// pyramids as prisms with the edge from 2 to 5 collapsed.  Compared
	// with the ordering the rest of the world uses, this has the effect
	// of switching verts 2 and 4.
	for (emInt ii = 0; ii < m_nPyrs; ii++)
	{
		std::swap(m_PyrConn[ii][2], m_PyrConn[ii][4]);
	}

	FILE *outFile = fopen(fileName, "w");
	if (!outFile)
	{
		fprintf(stderr, "Couldn't open file %s for writing.  Bummer!\n", fileName);
		return false;
	}

	fwrite(m_fileImage, m_fileImageSize, 1, outFile);
	fclose(outFile);

	// Need to undo the increment for future use
	size = m_nTris * 3 + m_nQuads * 4;
	incrementVertIndices(reinterpret_cast<emInt *>(m_TriConn), size, -1);
	size = m_nTets * 4 + m_nPyrs * 5 + m_nPrisms * 6 + m_nHexes * 8;
	incrementVertIndices(reinterpret_cast<emInt *>(m_TetConn), size, -1);

	// Need to swap verts 2 and 4 back for future use.
	for (emInt ii = 0; ii < m_nPyrs; ii++)
	{
		std::swap(m_PyrConn[ii][2], m_PyrConn[ii][4]);
	}

	double timeAfter = exaTime();
	double elapsed = timeAfter - timeBefore;
	size_t totalCells = size_t(m_nTets) + m_nPyrs + m_nPrisms + m_nHexes;
	fprintf(stderr, "CPU time for UGRID file write = %5.2F seconds\n", elapsed);
	fprintf(stderr, "                          %5.2F million cells / minute\n",
			(totalCells / 1000000.) / (elapsed / 60));

	return true;
}

static void remapIndices(const emInt nPts, const std::vector<emInt> &newIndices,
						 const emInt *conn, emInt *newConn)
{
	for (emInt jj = 0; jj < nPts; jj++)
	{
		newConn[jj] = newIndices[conn[jj]];
	}
}

// std::unique_ptr<UMesh> UMesh::extractCoarseMesh(Part& P,
// 		std::vector<CellPartData>& vecCPD, const int numDivs) const {
// 	// Count the number of tris, quads, tets, pyrs, prisms and hexes.
// 	const emInt first = P.getFirst();
// 	const emInt last = P.getLast();

// 	exa_set<TriFaceVerts> partBdryTris;
// 	exa_set<QuadFaceVerts> partBdryQuads;

// 	emInt nTris(0), nQuads(0), nTets(0), nPyrs(0), nPrisms(0), nHexes(0);
// 	const emInt *conn;

// 	std::vector<bool> isBdryVert(numVerts(), false);
// 	std::vector<bool> isVertUsed(numVerts(), false);

// 	for (emInt ii = first; ii < last; ii++) {
// 		emInt type = vecCPD[ii].getCellType();
// 		emInt ind = vecCPD[ii].getIndex();
// 		switch (type) {
// 			default:
// 				// Panic! Should never get here.
// 				assert(0);
// 				break;
// 			case TETRA_4: {
// 				nTets++;
// 				conn = getTetConn(ind);
// 				TriFaceVerts TFV012(numDivs, conn[0], conn[1], conn[2]);
// 				TriFaceVerts TFV013(numDivs, conn[0], conn[1], conn[3]);
// 				TriFaceVerts TFV123(numDivs, conn[1], conn[2], conn[3]);
// 				TriFaceVerts TFV203(numDivs, conn[2], conn[0], conn[3]);
// 				addUniquely(partBdryTris, TFV012);
// 				addUniquely(partBdryTris, TFV013);
// 				addUniquely(partBdryTris, TFV123);
// 				addUniquely(partBdryTris, TFV203);
// 				isVertUsed[conn[0]] = true;
// 				isVertUsed[conn[1]] = true;
// 				isVertUsed[conn[2]] = true;
// 				isVertUsed[conn[3]] = true;
// 				break;
// 			}
// 			case PYRA_5: {
// 				nPyrs++;
// 				conn = getPyrConn(ind);
// 				QuadFaceVerts QFV0123(numDivs, conn[0], conn[1], conn[2], conn[3]);
// 				TriFaceVerts TFV014(numDivs, conn[0], conn[1], conn[4]);
// 				TriFaceVerts TFV124(numDivs, conn[1], conn[2], conn[4]);
// 				TriFaceVerts TFV234(numDivs, conn[2], conn[3], conn[4]);
// 				TriFaceVerts TFV304(numDivs, conn[3], conn[0], conn[4]);
// 				addUniquely(partBdryQuads, QFV0123);
// 				addUniquely(partBdryTris, TFV014);
// 				addUniquely(partBdryTris, TFV124);
// 				addUniquely(partBdryTris, TFV234);
// 				addUniquely(partBdryTris, TFV304);
// 				isVertUsed[conn[0]] = true;
// 				isVertUsed[conn[1]] = true;
// 				isVertUsed[conn[2]] = true;
// 				isVertUsed[conn[3]] = true;
// 				isVertUsed[conn[4]] = true;
// 				break;
// 			}
// 			case PENTA_6: {
// 				nPrisms++;
// 				conn = getPrismConn(ind);
// 				QuadFaceVerts QFV0143(numDivs, conn[0], conn[1], conn[4], conn[3]);
// 				QuadFaceVerts QFV1254(numDivs, conn[1], conn[2], conn[5], conn[4]);
// 				QuadFaceVerts QFV2035(numDivs, conn[2], conn[0], conn[3], conn[5]);
// 				TriFaceVerts TFV012(numDivs, conn[0], conn[1], conn[2]);
// 				TriFaceVerts TFV345(numDivs, conn[3], conn[4], conn[5]);
// 				addUniquely(partBdryQuads, QFV0143);
// 				addUniquely(partBdryQuads, QFV1254);
// 				addUniquely(partBdryQuads, QFV2035);
// 				addUniquely(partBdryTris, TFV012);
// 				addUniquely(partBdryTris, TFV345);
// 				isVertUsed[conn[0]] = true;
// 				isVertUsed[conn[1]] = true;
// 				isVertUsed[conn[2]] = true;
// 				isVertUsed[conn[3]] = true;
// 				isVertUsed[conn[4]] = true;
// 				isVertUsed[conn[5]] = true;
// 				break;
// 			}
// 			case HEXA_8: {
// 				nHexes++;
// 				conn = getHexConn(ind);
// 				QuadFaceVerts QFV0154(numDivs, conn[0], conn[1], conn[5], conn[4]);
// 				QuadFaceVerts QFV1265(numDivs, conn[1], conn[2], conn[6], conn[5]);
// 				QuadFaceVerts QFV2376(numDivs, conn[2], conn[3], conn[7], conn[6]);
// 				QuadFaceVerts QFV3047(numDivs, conn[3], conn[0], conn[4], conn[7]);
// 				QuadFaceVerts QFV0123(numDivs, conn[0], conn[1], conn[2], conn[3]);
// 				QuadFaceVerts QFV4567(numDivs, conn[4], conn[5], conn[6], conn[7]);
// 				addUniquely(partBdryQuads, QFV0154);
// 				addUniquely(partBdryQuads, QFV1265);
// 				addUniquely(partBdryQuads, QFV2376);
// 				addUniquely(partBdryQuads, QFV3047);
// 				addUniquely(partBdryQuads, QFV0123);
// 				addUniquely(partBdryQuads, QFV4567);
// 				isVertUsed[conn[0]] = true;
// 				isVertUsed[conn[1]] = true;
// 				isVertUsed[conn[2]] = true;
// 				isVertUsed[conn[3]] = true;
// 				isVertUsed[conn[4]] = true;
// 				isVertUsed[conn[5]] = true;
// 				isVertUsed[conn[6]] = true;
// 				isVertUsed[conn[7]] = true;
// 				break;
// 			}
// 		} // end switch
// 	} // end loop to gather information

// 	// Now check to see which bdry entities are in this part.  That'll be the
// 	// ones whose verts are all marked as used.  Unfortunately, this requires
// 	// searching through -all- the bdry entities for each part.
// 	std::vector<emInt> realBdryTris;
// 	std::vector<emInt> realBdryQuads;
// 	for (emInt ii = 0; ii < numBdryTris(); ii++) {
// 		conn = getBdryTriConn(ii);
// 		if (isVertUsed[conn[0]] && isVertUsed[conn[1]] && isVertUsed[conn[2]]) {
// 			TriFaceVerts TFV(numDivs, conn[0], conn[1], conn[2]);
// 			auto iter = partBdryTris.find(TFV);
// 			// If this bdry tri is an unmatched tri from this part, match it, and
// 			// add the bdry tri to the list of things to copy to the part coarse
// 			// mesh.  Otherwise, do nothing.  This will keep the occasional wrong
// 			// bdry face from slipping through.
// 			if (iter != partBdryTris.end()) {
// 				partBdryTris.erase(iter);
// 				isBdryVert[conn[0]] = true;
// 				isBdryVert[conn[1]] = true;
// 				isBdryVert[conn[2]] = true;
// 				realBdryTris.push_back(ii);
// 				nTris++;
// 			}
// 		}
// 	}
// 	for (emInt ii = 0; ii < numBdryQuads(); ii++) {
// 		conn = getBdryQuadConn(ii);
// 		if (isVertUsed[conn[0]] && isVertUsed[conn[1]] && isVertUsed[conn[2]]
// 				&& isVertUsed[conn[3]]) {
// 			QuadFaceVerts QFV(numDivs, conn[0], conn[1], conn[2], conn[3]);
// 			auto iter = partBdryQuads.find(QFV);
// 			// If this bdry tri is an unmatched tri from this part, match it, and
// 			// add the bdry tri to the list of things to copy to the part coarse
// 			// mesh.  Otherwise, do nothing.  This will keep the occasional wrong
// 			// bdry face from slipping through.
// 			if (iter != partBdryQuads.end()) {
// 				partBdryQuads.erase(iter);
// 				isBdryVert[conn[0]] = true;
// 				isBdryVert[conn[1]] = true;
// 				isBdryVert[conn[2]] = true;
// 				isBdryVert[conn[3]] = true;
// 				realBdryQuads.push_back(ii);
// 				nQuads++;
// 			}
// 		}
// 	}

// 	emInt nPartBdryTris = partBdryTris.size();
// 	emInt nPartBdryQuads = partBdryQuads.size();

// 	for (auto tri : partBdryTris) {
// 		isBdryVert[tri.getCorner(0)] = true;
// 		isBdryVert[tri.getCorner(1)] = true;
// 		isBdryVert[tri.getCorner(2)] = true;
// 	}

// 	for (auto quad : partBdryQuads) {
// 		isBdryVert[quad.getCorner(0)] = true;
// 		isBdryVert[quad.getCorner(1)] = true;
// 		isBdryVert[quad.getCorner(2)] = true;
// 		isBdryVert[quad.getCorner(3)] = true;
// 	}
// 	emInt nBdryVerts = 0, nVerts = 0;
// 	for (emInt ii = 0; ii < numVerts(); ii++) {
// 		if (isBdryVert[ii]) nBdryVerts++;
// 		if (isVertUsed[ii]) nVerts++;
// 	}

// 	// Now set up the data structures for the new coarse UMesh
// 	auto UUM = std::make_unique<UMesh>(nVerts, nBdryVerts, nTris + nPartBdryTris,
// 																			nQuads + nPartBdryQuads, nTets, nPyrs,
// 																			nPrisms, nHexes);

// 	// Store the vertices, while keeping a mapping from the full list of verts
// 	// to the restricted list so the connectivity can be copied properly.
// 	std::vector<emInt> newIndices(numVerts(), EMINT_MAX);
// 	for (emInt ii = 0; ii < numVerts(); ii++) {
// 		if (isVertUsed[ii]) {
// 			double coords[3];
// 			getCoords(ii, coords);
// 			newIndices[ii] = UUM->addVert(coords);
// 			// Copy length scale for vertices from the parent; otherwise, there will be
// 			// mismatches in the refined meshes.
// 			UUM->setLengthScale(newIndices[ii], getLengthScale(ii));
// 		}
// 	}

// 	// Now copy connectivity.
// 	emInt newConn[8];
// 	for (emInt ii = first; ii < last; ii++) {
// 		emInt type = vecCPD[ii].getCellType();
// 		emInt ind = vecCPD[ii].getIndex();
// 		switch (type) {
// 			default:
// 				// Panic! Should never get here.
// 				assert(0);
// 				break;
// 			case TETRA_4: {
// 				conn = getTetConn(ind);
// 				remapIndices(4, newIndices, conn, newConn);
// 				UUM->addTet(newConn);
// 				break;
// 			}
// 			case PYRA_5: {
// 				conn = getPyrConn(ind);
// 				remapIndices(5, newIndices, conn, newConn);
// 				UUM->addPyramid(newConn);
// 				break;
// 			}
// 			case PENTA_6: {
// 				conn = getPrismConn(ind);
// 				remapIndices(6, newIndices, conn, newConn);
// 				UUM->addPrism(newConn);
// 				break;
// 			}
// 			case HEXA_8: {
// 				conn = getHexConn(ind);
// 				remapIndices(8, newIndices, conn, newConn);
// 				UUM->addHex(newConn);
// 				break;
// 			}
// 		} // end switch
// 	} // end loop to copy most connectivity

// 	for (emInt ii = 0; ii < realBdryTris.size(); ii++) {
// 		conn = getBdryTriConn(realBdryTris[ii]);
// 		remapIndices(3, newIndices, conn, newConn);
// 		UUM->addBdryTri(newConn);
// 	}
// 	for (emInt ii = 0; ii < realBdryQuads.size(); ii++) {
// 		conn = getBdryQuadConn(realBdryQuads[ii]);
// 		remapIndices(4, newIndices, conn, newConn);
// 		UUM->addBdryQuad(newConn);
// 	}

// 	// Now, finally, the part bdry connectivity.
// 	// TODO: Currently, there's nothing in the data structure that marks which
// 	// are part bdry faces.
// 	for (auto tri : partBdryTris) {
// 		emInt conn[] = { newIndices[tri.getCorner(0)],
// 				newIndices[tri.getCorner(1)],
// 				newIndices[tri.getCorner(2)] };
// 		UUM->addBdryTri(conn);
// 	}

// 	for (auto quad : partBdryQuads) {
// 		emInt conn[] = { newIndices[quad.getCorner(0)],
// 				newIndices[quad.getCorner(1)],
// 				newIndices[quad.getCorner(2)],
// 				newIndices[quad.getCorner(3)] };
// 		UUM->addBdryQuad(conn);
// 	}

// 	return UUM;
// }

std::unique_ptr<UMesh> UMesh::createFineUMesh(const emInt numDivs, Part &P,
											  std::vector<CellPartData> &vecCPD, struct RefineStats &RS) const
{
	// Create a coarse
	double start = exaTime();
	auto coarse = extractCoarseMesh(P, vecCPD, numDivs);
	double middle = exaTime();
	RS.extractTime = middle - start;

	auto UUM = std::make_unique<UMesh>(*coarse, numDivs);
	RS.cells = UUM->numCells();
	RS.refineTime = exaTime() - middle;
	return UUM;
}

void UMesh::setupCellDataForPartitioning(std::vector<CellPartData> &vecCPD,
										 double &xmin, double &ymin, double &zmin, double &xmax, double &ymax,
										 double &zmax) const
{
	// Partitioning only cells, not bdry faces.  Also, currently no
	// cost differential for different cell types.
	for (emInt ii = 0; ii < numTets(); ii++)
	{
		const emInt *verts = getTetConn(ii);
		addCellToPartitionData(verts, 4, ii, TETRA_4, vecCPD, xmin, ymin, zmin,
							   xmax, ymax, zmax);
	}
	for (emInt ii = 0; ii < numPyramids(); ii++)
	{
		const emInt *verts = getPyrConn(ii);
		addCellToPartitionData(verts, 5, ii, PYRA_5, vecCPD, xmin, ymin, zmin, xmax,
							   ymax, zmax);
	}
	for (emInt ii = 0; ii < numPrisms(); ii++)
	{
		const emInt *verts = getPrismConn(ii);
		addCellToPartitionData(verts, 6, ii, PENTA_6, vecCPD, xmin, ymin, zmin,
							   xmax, ymax, zmax);
	}
	for (emInt ii = 0; ii < numHexes(); ii++)
	{
		const emInt *verts = getHexConn(ii);
		addCellToPartitionData(verts, 8, ii, HEXA_8, vecCPD, xmin, ymin, zmin, xmax,
							   ymax, zmax);
	}
}

// bool UMesh::writeCompressedUGridFile(const char fileName[]) {
//	double timeBefore = clock() / double(CLOCKS_PER_SEC);
//
//	gzFile outFile = gzopen(fileName, "wb");
//	if (!outFile) {
//		fprintf(stderr, "Couldn't open file %s for writing.  Bummer!\n", fileName);
//		return false;
//	}
//
//	int bytesWritten = gzwrite(outFile, reinterpret_cast<void*>(m_fileImage),
//															m_fileImageSize);
//	gzclose(outFile);
//	fprintf(stderr, "Wrote %d bytes into a compressed file as %d bytes\n",
//					m_fileImageSize, bytesWritten);
//
//	double timeAfter = clock() / double(CLOCKS_PER_SEC);
//	double elapsed = timeAfter - timeBefore;
//	size_t totalCells = size_t(m_nTets) + m_nPyrs + m_nPrisms + m_nHexes;
//	fprintf(stderr, "CPU time for compressed UGRID file write = %5.2F seconds\n",
//					elapsed);
//	fprintf(stderr, "                          %5.2F million cells / minute\n",
//					(totalCells / 1000000.) / (elapsed / 60));
//
//	return true;
// }
void
buildTrisMap(hashTri const& tris, std::map<int, vecTri> &remoteTotris,
std::set<int> &neighbors)
{

	for(auto it= tris.begin(); it!=tris.end(); it++ )
	{
		int remoteId= it->getRemoteId(); 
		neighbors.insert(remoteId); 
		remoteTotris[remoteId].push_back(*it); 

	}
	

	//return remoteTotris; 
	
}
void
buildQuadsMap(hashQuad const& quads, std::map<int, vecQuad> &remoteToquads,
std::set<int> &neighbors)
{
	
	for(auto it= quads.begin(); it!=quads.end(); it++ )
	{
		int remoteId= it->getRemoteId(); 
		neighbors.insert(remoteId); 
		remoteToquads[remoteId].push_back(*it); 

	}
	//assert(remoteToquads.size()==quads.size()); 
}
void printMatchedTris (const std::unordered_map<emInt, emInt> &localRemote, 
const int &rank)
{
	char fileName [100]; 
	sprintf(fileName, "Results/tris%03d.vtk", rank);
	std::ofstream out(fileName); 
	out<<"I'm part: "<<rank<<std::endl; 
	for(const auto it:localRemote)
	{
		out<<"key: "<<it.first<<" value: "<<it.second<<std::endl; 
	}

}
void printMatchedQuads (const std::unordered_map<emInt, emInt> &localRemote, 
const int &rank)
{
	char fileName [100]; 
	sprintf(fileName, "Results/Quads%03d.vtk", rank);
	std::ofstream out(fileName); 
	out<<"I'm part: "<<rank<<std::endl; 
	for(const auto it:localRemote)
	{
		out<<"key: "<<it.first<<" value: "<<it.second<<std::endl; 
	}

}
void 
UMesh::TestMPI(const emInt &nDivs, const emInt &nParts)
{

	vecPart parts;
	vecCellPartData vecCPD;

	std::set<int> triRotations;
	std::set<int> quadRotations;

	vecHashTri tris;
	vecHashQuad quads;

	vecSharePtrUmesh submeshes;
	vecSharePtrUmesh refinedUMeshes;

	partitionCells(this, nParts, parts, vecCPD);


	//tester->setVecpart(parts); 
	//tester->setvecCellPartData(vecCPD); 
	//auto submeshes= getExaMeshType(isUMesh); 

	this->partFaceMatching(parts, vecCPD, tris, quads);
	//tester->setInputTri(tris); 

	

	for (auto i = 0; i < nParts; i++)
	{
		auto coarse = this->extractCoarseMesh(parts[i], vecCPD, nDivs, tris[i], quads[i], i);
		std::shared_ptr<UMesh> shared_ptr = std::move(coarse);
		submeshes.push_back(shared_ptr);
		char fileName[100];
		sprintf(fileName, "TestCases/Coarsesubmesh%03d.vtk", i);
		shared_ptr->writeVTKFile(fileName);
	}

	assert(submeshes.size() == static_cast<std::size_t>(nParts));
	// Refine the mesh
	for (auto i = 0; i < nParts; i++)
	{
		auto refineUmesh = std::make_shared<UMesh>(
			*(submeshes[i].get()), nDivs, i);
		refinedUMeshes.push_back(refineUmesh);
		char fileName[100];
		sprintf(fileName, "TestCases/Refinedmesh%03d.vtk", i);
		refineUmesh->writeVTKFile(fileName);
	}
	for (auto iPart = 0; iPart < nParts; iPart++)
	{
		exa_set<TriFaceVerts> tri =
			refinedUMeshes[iPart]->getRefinedPartTris();
		exa_set<QuadFaceVerts> quads = refinedUMeshes[iPart]->getRefinedPartQuads();
		for (auto it = tri.begin(); it != tri.end(); it++)
		{
			std::unordered_map<emInt, emInt> localRemote;
			emInt whereToLook = it->getRemoteId();
			exa_set<TriFaceVerts> remoteTriSet =
				refinedUMeshes[whereToLook]->getRefinedPartTris();
			emInt rotation = getTriRotation(*it, remoteTriSet, nDivs);
			triRotations.insert(rotation);

			matchTri(*it, rotation, nDivs, remoteTriSet, localRemote);
			printMatchedTris(localRemote,iPart); 
			for (auto itmap = localRemote.begin();
				 itmap != localRemote.end(); itmap++)
			{
				assert(abs(refinedUMeshes[iPart]->getX(itmap->first) -
						   refinedUMeshes[whereToLook]->getX(itmap->second)) < TOLTEST);
				assert(abs(refinedUMeshes[iPart]->getY(itmap->first) -
						   refinedUMeshes[whereToLook]->getY(itmap->second)) < TOLTEST);
				assert(abs(refinedUMeshes[iPart]->getZ(itmap->first) -
						   refinedUMeshes[whereToLook]->getZ(itmap->second)) < TOLTEST);
			}
		}
		for (auto it = quads.begin(); it != quads.end(); it++)
		{
			std::unordered_map<emInt, emInt> localRemote;
			emInt whereToLook = it->getRemoteId();
			exa_set<QuadFaceVerts> remoteQuadSet =
				refinedUMeshes[whereToLook]->getRefinedPartQuads();
			emInt rotation = getQuadRotation(*it, remoteQuadSet, nDivs);
			quadRotations.insert(rotation);

			matchQuad(*it, rotation, nDivs, remoteQuadSet, localRemote);
			//std::cout<<localRemote.size()<<std::endl; 
			printMatchedQuads(localRemote,iPart); 
			for (auto itmap = localRemote.begin();
				 itmap != localRemote.end(); itmap++)
			{
				assert(abs(refinedUMeshes[iPart]->getX(itmap->first) -
						   refinedUMeshes[whereToLook]->getX(itmap->second)) < TOLTEST);
				assert(abs(refinedUMeshes[iPart]->getY(itmap->first) -
						   refinedUMeshes[whereToLook]->getY(itmap->second)) < TOLTEST);
				assert(abs(refinedUMeshes[iPart]->getZ(itmap->first) -
						   refinedUMeshes[whereToLook]->getZ(itmap->second)) < TOLTEST);
			}
		}
	}
	// Which rotation cases are covered?
	std::cout << "Covered Tri Rotations: " << std::endl;
	for (auto it = triRotations.begin();
		 it != triRotations.end(); it++)
	{
		std::cout << *it << " ";
	};
	std::cout << std::endl;
	std::cout << "Covered Quad Rotations: " << std::endl;
	for (auto it = quadRotations.begin();
		 it != quadRotations.end(); it++)
	{
		std::cout << *it << " ";
	};
	std::cout << std::endl;
}

void UMesh::partFaceMatching(
	std::vector<Part> &parts, const std::vector<CellPartData> &vecCPD,
	std::vector<std::unordered_set<TriFaceVerts>> &tris,
	std::vector<std::unordered_set<QuadFaceVerts>> &quads) const
{

	// std::set<TriFaceVerts>  SetTriPartbdry;

	// std::set<QuadFaceVerts> SetQuadPartbdry;

	std::set<TriFaceVerts> partBdryTris;
	std::set<QuadFaceVerts> partBdryQuads;

	tris.resize(parts.size());
	quads.resize(parts.size());

	emInt numDivs = 1;

	for (std::size_t iPart = 0; iPart < parts.size(); iPart++)
	{
		// emInt iPart=1;

		const emInt first = parts[iPart].getFirst();
		const emInt last = parts[iPart].getLast();

		const emInt *conn;

		std::vector<bool> isBdryVert(numVerts(), false);
		std::vector<bool> isVertUsed(numVerts(), false);

		for (emInt ii = first; ii < last; ii++)
		{
			emInt type = vecCPD[ii].getCellType();
			emInt ind = vecCPD[ii].getIndex();
			switch (type)
			{
			default:
				// Panic! Should never get here.
				assert(0);
				break;
			case TETRA_4:
			{

				conn = getTetConn(ind);

				emInt global012[3] = {conn[0], conn[1], conn[2]};
				emInt global013[3] = {conn[0], conn[1], conn[3]};
				emInt global123[3] = {conn[1], conn[2], conn[3]};
				emInt global203[3] = {conn[2], conn[0], conn[3]};
				TriFaceVerts T012(numDivs, global012, iPart);
				TriFaceVerts T013(numDivs, global013, iPart);
				TriFaceVerts T123(numDivs, global123, iPart);
				TriFaceVerts T203(numDivs, global203, iPart);
				addUniquely(partBdryTris, T012);
				addUniquely(partBdryTris, T013);
				addUniquely(partBdryTris, T123);
				addUniquely(partBdryTris, T203);
				break;
			}
			case PYRA_5:
			{
				// nPyrs++;
				conn = getPyrConn(ind);

				TriFaceVerts TFV014(numDivs, conn[0], conn[1], conn[4]);
				TriFaceVerts TFV124(numDivs, conn[1], conn[2], conn[4]);
				TriFaceVerts TFV234(numDivs, conn[2], conn[3], conn[4]);
				TriFaceVerts TFV304(numDivs, conn[3], conn[0], conn[4]);
				emInt global0123[4] = {conn[0], conn[1], conn[2], conn[3]};
				emInt global014[3] = {conn[0], conn[1], conn[4]};
				emInt global124[3] = {conn[1], conn[2], conn[4]};
				emInt global234[3] = {conn[2], conn[3], conn[4]};
				emInt global304[3] = {conn[3], conn[0], conn[4]};
				TriFaceVerts T014(numDivs, global014, iPart);
				TriFaceVerts T124(numDivs, global124, iPart);
				TriFaceVerts T234(numDivs, global234, iPart);
				TriFaceVerts T304(numDivs, global304, iPart);
				QuadFaceVerts Q0123(numDivs, global0123, iPart);
				addUniquely(partBdryTris, T014);
				addUniquely(partBdryTris, T124);
				addUniquely(partBdryTris, T234);
				addUniquely(partBdryTris, T304);
				addUniquely(partBdryQuads, Q0123);
				break;
			}
			case PENTA_6:
			{
				// nPrisms++;
				conn = getPrismConn(ind);

				emInt global0143[4] = {conn[0], conn[1], conn[4], conn[3]};
				emInt global1254[4] = {conn[1], conn[2], conn[5], conn[4]};
				emInt global2035[4] = {conn[2], conn[0], conn[3], conn[5]};

				emInt global012[3] = {conn[0], conn[1], conn[2]};
				emInt global345[3] = {conn[3], conn[4], conn[5]};

				TriFaceVerts T012(numDivs, global012, iPart);
				TriFaceVerts T345(numDivs, global345, iPart);
				QuadFaceVerts Q0143(numDivs, global0143, iPart);
				QuadFaceVerts Q1254(numDivs, global1254, iPart);
				QuadFaceVerts Q2035(numDivs, global2035, iPart);

				addUniquely(partBdryTris, T012);
				addUniquely(partBdryTris, T345);
				addUniquely(partBdryQuads, Q0143);
				addUniquely(partBdryQuads, Q1254);
				addUniquely(partBdryQuads, Q2035);
				break;
			}
			case HEXA_8:
			{
				// nHexes++;
				conn = getHexConn(ind);

				emInt global0154[4] = {conn[0], conn[1], conn[5], conn[4]};
				emInt global1265[4] = {conn[1], conn[2], conn[6], conn[5]};
				emInt global2376[4] = {conn[2], conn[3], conn[7], conn[6]};
				emInt global3047[4] = {conn[3], conn[0], conn[4], conn[7]};
				emInt global0123[4] = {conn[0], conn[1], conn[2], conn[3]};
				emInt global4567[4] = {conn[4], conn[5], conn[6], conn[7]};

				QuadFaceVerts Q0154(numDivs, global0154, iPart);
				QuadFaceVerts Q1265(numDivs, global1265, iPart);
				QuadFaceVerts Q2376(numDivs, global2376, iPart);
				QuadFaceVerts Q3047(numDivs, global3047, iPart);
				QuadFaceVerts Q0123(numDivs, global0123, iPart);
				QuadFaceVerts Q4567(numDivs, global4567, iPart);
				addUniquely(partBdryQuads, Q0154);
				addUniquely(partBdryQuads, Q1265);
				addUniquely(partBdryQuads, Q2376);
				addUniquely(partBdryQuads, Q3047);
				addUniquely(partBdryQuads, Q0123);
				addUniquely(partBdryQuads, Q4567);

				break;
			}
			} // end switch
		}	  // end loop to gather information
	}

	auto k = 0;

	for (auto itr = partBdryTris.begin(); itr != partBdryTris.end(); itr++)
	{
		k++;

		auto next = std::next(itr, 1);
		if (k != partBdryTris.size())
		{
			if (
				itr->getGlobalSorted(0) == next->getGlobalSorted(0) &&

				itr->getGlobalSorted(1) == next->getGlobalSorted(1) &&
				itr->getGlobalSorted(2) == next->getGlobalSorted(2))
			{

				emInt global[3] = {itr->getGlobalCorner(0),
								   itr->getGlobalCorner(1), itr->getGlobalCorner(2)};
				emInt globalNext[3] = {next->getGlobalCorner(0),
									   next->getGlobalCorner(1), next->getGlobalCorner(2)};

				TriFaceVerts tripart(numDivs, global, itr->getPartid(),
									 next->getPartid(), true);

				TriFaceVerts tripartNext(numDivs, globalNext, next->getPartid(), itr->getPartid(), true);

				addUniquely(tris[itr->getPartid()], tripart);
				addUniquely(tris[next->getPartid()], tripartNext);
			}
		}
	}

	auto kquad = 0;
	for (auto itr = partBdryQuads.begin();
		 itr != partBdryQuads.end(); itr++)
	{

		auto next = std::next(itr, 1);
		kquad++;
		if (kquad != partBdryQuads.size())
		{
			emInt v0Global = next->getGlobalCorner(0);
			emInt v1Global = next->getGlobalCorner(1);
			emInt v2Global = next->getGlobalCorner(2);
			emInt v3Global = next->getGlobalCorner(3);

			emInt partid = next->getPartid();

			emInt v0SortedGlobal = next->getGlobalSorted(0);
			emInt v1SortedGlobal = next->getGlobalSorted(1);
			emInt v2SortedGlobal = next->getGlobalSorted(2);
			emInt v3SortedGlobal = next->getGlobalSorted(3);

			emInt v0Global_ = itr->getGlobalCorner(0);
			emInt v1Global_ = itr->getGlobalCorner(1);
			emInt v2Global_ = itr->getGlobalCorner(2);
			emInt v3Global_ = itr->getGlobalCorner(3);

			emInt partid_ = itr->getPartid();

			emInt v0SortedGlobal_ = itr->getGlobalSorted(0);
			emInt v1SortedGlobal_ = itr->getGlobalSorted(1);
			emInt v2SortedGlobal_ = itr->getGlobalSorted(2);
			emInt v3SortedGlobal_ = itr->getGlobalSorted(3);

			if (v0SortedGlobal_ == v0SortedGlobal &&
				v1SortedGlobal == v1SortedGlobal_ &&
				v2SortedGlobal == v2SortedGlobal_ &&
				v3SortedGlobal == v3SortedGlobal_)
			{
				emInt global[4] = {v0Global, v1Global, v2Global, v3Global};
				emInt global_[4] = {v0Global_, v1Global_, v2Global_, v3Global_};

				QuadFaceVerts quadpart(numDivs, global, partid, partid_, true);
				QuadFaceVerts quadpart_(numDivs, global_, partid_, partid, true);
				addUniquely(quads[partid], quadpart);
				addUniquely(quads[partid_], quadpart_);
			}
		}
	}
}
std::unique_ptr<UMesh> UMesh::extractCoarseMesh(Part &P,
												std::vector<CellPartData> &vecCPD, const int numDivs,
												const std::unordered_set<TriFaceVerts> &tris,
												const std::unordered_set<QuadFaceVerts> &quads, const emInt partID) const
{
	// Count the number of tris, quads, tets, pyrs, prisms and hexes.
	const emInt first = P.getFirst();
	const emInt last = P.getLast();

	exa_set<TriFaceVerts> partBdryTris;
	exa_set<QuadFaceVerts> partBdryQuads;

	emInt nTris(0), nQuads(0), nTets(0), nPyrs(0), nPrisms(0), nHexes(0);
	const emInt *conn;

	std::vector<bool> isBdryVert(numVerts(), false);
	std::vector<bool> isVertUsed(numVerts(), false);

	for (emInt ii = first; ii < last; ii++)
	{
		emInt type = vecCPD[ii].getCellType();
		emInt ind = vecCPD[ii].getIndex();
		switch (type)
		{
		default:
			// Panic! Should never get here.
			assert(0);
			break;
		case TETRA_4:
		{
			nTets++;
			conn = getTetConn(ind);
			TriFaceVerts TFV012(numDivs, conn[0], conn[1], conn[2]);
			TriFaceVerts TFV013(numDivs, conn[0], conn[1], conn[3]);
			TriFaceVerts TFV123(numDivs, conn[1], conn[2], conn[3]);
			TriFaceVerts TFV203(numDivs, conn[2], conn[0], conn[3]);
			addUniquely(partBdryTris, TFV012);
			addUniquely(partBdryTris, TFV013);
			addUniquely(partBdryTris, TFV123);
			addUniquely(partBdryTris, TFV203);
			isVertUsed[conn[0]] = true;
			isVertUsed[conn[1]] = true;
			isVertUsed[conn[2]] = true;
			isVertUsed[conn[3]] = true;
			break;
		}
		case PYRA_5:
		{
			nPyrs++;
			conn = getPyrConn(ind);
			QuadFaceVerts QFV0123(numDivs, conn[0], conn[1], conn[2], conn[3]);
			TriFaceVerts TFV014(numDivs, conn[0], conn[1], conn[4]);
			TriFaceVerts TFV124(numDivs, conn[1], conn[2], conn[4]);
			TriFaceVerts TFV234(numDivs, conn[2], conn[3], conn[4]);
			TriFaceVerts TFV304(numDivs, conn[3], conn[0], conn[4]);
			addUniquely(partBdryQuads, QFV0123);
			addUniquely(partBdryTris, TFV014);
			addUniquely(partBdryTris, TFV124);
			addUniquely(partBdryTris, TFV234);
			addUniquely(partBdryTris, TFV304);
			isVertUsed[conn[0]] = true;
			isVertUsed[conn[1]] = true;
			isVertUsed[conn[2]] = true;
			isVertUsed[conn[3]] = true;
			isVertUsed[conn[4]] = true;
			break;
		}
		case PENTA_6:
		{
			nPrisms++;
			conn = getPrismConn(ind);
			QuadFaceVerts QFV0143(numDivs, conn[0], conn[1], conn[4], conn[3]);
			QuadFaceVerts QFV1254(numDivs, conn[1], conn[2], conn[5], conn[4]);
			QuadFaceVerts QFV2035(numDivs, conn[2], conn[0], conn[3], conn[5]);
			TriFaceVerts TFV012(numDivs, conn[0], conn[1], conn[2]);
			TriFaceVerts TFV345(numDivs, conn[3], conn[4], conn[5]);
			addUniquely(partBdryQuads, QFV0143);
			addUniquely(partBdryQuads, QFV1254);
			addUniquely(partBdryQuads, QFV2035);
			addUniquely(partBdryTris, TFV012);
			addUniquely(partBdryTris, TFV345);
			isVertUsed[conn[0]] = true;
			isVertUsed[conn[1]] = true;
			isVertUsed[conn[2]] = true;
			isVertUsed[conn[3]] = true;
			isVertUsed[conn[4]] = true;
			isVertUsed[conn[5]] = true;
			break;
		}
		case HEXA_8:
		{
			nHexes++;
			conn = getHexConn(ind);
			QuadFaceVerts QFV0154(numDivs, conn[0], conn[1], conn[5], conn[4]);
			QuadFaceVerts QFV1265(numDivs, conn[1], conn[2], conn[6], conn[5]);
			QuadFaceVerts QFV2376(numDivs, conn[2], conn[3], conn[7], conn[6]);
			QuadFaceVerts QFV3047(numDivs, conn[3], conn[0], conn[4], conn[7]);
			QuadFaceVerts QFV0123(numDivs, conn[0], conn[1], conn[2], conn[3]);
			QuadFaceVerts QFV4567(numDivs, conn[4], conn[5], conn[6], conn[7]);
			addUniquely(partBdryQuads, QFV0154);
			addUniquely(partBdryQuads, QFV1265);
			addUniquely(partBdryQuads, QFV2376);
			addUniquely(partBdryQuads, QFV3047);
			addUniquely(partBdryQuads, QFV0123);
			addUniquely(partBdryQuads, QFV4567);
			isVertUsed[conn[0]] = true;
			isVertUsed[conn[1]] = true;
			isVertUsed[conn[2]] = true;
			isVertUsed[conn[3]] = true;
			isVertUsed[conn[4]] = true;
			isVertUsed[conn[5]] = true;
			isVertUsed[conn[6]] = true;
			isVertUsed[conn[7]] = true;
			break;
		}
		} // end switch
	}	  // end loop to gather information

	// Now check to see which bdry entities are in this part.  That'll be the
	// ones whose verts are all marked as used.  Unfortunately, this requires
	// searching through -all- the bdry entities for each part.
	std::vector<emInt> realBdryTris;
	std::vector<emInt> realBdryQuads;
	for (emInt ii = 0; ii < numBdryTris(); ii++)
	{
		conn = getBdryTriConn(ii);
		if (isVertUsed[conn[0]] && isVertUsed[conn[1]] && isVertUsed[conn[2]])
		{
			TriFaceVerts TFV(numDivs, conn[0], conn[1], conn[2]);
			auto iter = partBdryTris.find(TFV);
			// If this bdry tri is an unmatched tri from this part, match it, and
			// add the bdry tri to the list of things to copy to the part coarse
			// mesh.  Otherwise, do nothing.  This will keep the occasional wrong
			// bdry face from slipping through.
			if (iter != partBdryTris.end())
			{
				partBdryTris.erase(iter);
				isBdryVert[conn[0]] = true;
				isBdryVert[conn[1]] = true;
				isBdryVert[conn[2]] = true;
				realBdryTris.push_back(ii);
				nTris++;
			}
		}
	}
	for (emInt ii = 0; ii < numBdryQuads(); ii++)
	{
		conn = getBdryQuadConn(ii);
		if (isVertUsed[conn[0]] && isVertUsed[conn[1]] && isVertUsed[conn[2]] && isVertUsed[conn[3]])
		{
			QuadFaceVerts QFV(numDivs, conn[0], conn[1], conn[2], conn[3]);
			auto iter = partBdryQuads.find(QFV);
			// If this bdry tri is an unmatched tri from this part, match it, and
			// add the bdry tri to the list of things to copy to the part coarse
			// mesh.  Otherwise, do nothing.  This will keep the occasional wrong
			// bdry face from slipping through.
			if (iter != partBdryQuads.end())
			{
				partBdryQuads.erase(iter);
				isBdryVert[conn[0]] = true;
				isBdryVert[conn[1]] = true;
				isBdryVert[conn[2]] = true;
				isBdryVert[conn[3]] = true;
				realBdryQuads.push_back(ii);
				nQuads++;
			}
		}
	}

	emInt nPartBdryTris = partBdryTris.size();
	emInt nPartBdryQuads = partBdryQuads.size();

	for (auto tri : partBdryTris)
	{
		isBdryVert[tri.getCorner(0)] = true;
		isBdryVert[tri.getCorner(1)] = true;
		isBdryVert[tri.getCorner(2)] = true;
	}

	for (auto quad : partBdryQuads)
	{
		isBdryVert[quad.getCorner(0)] = true;
		isBdryVert[quad.getCorner(1)] = true;
		isBdryVert[quad.getCorner(2)] = true;
		isBdryVert[quad.getCorner(3)] = true;
	}
	emInt nBdryVerts = 0, nVerts = 0;
	for (emInt ii = 0; ii < numVerts(); ii++)
	{
		if (isBdryVert[ii])
			nBdryVerts++;
		if (isVertUsed[ii])
			nVerts++;
	}

	// Now set up the data structures for the new coarse UMesh
	auto UUM = std::make_unique<UMesh>(nVerts, nBdryVerts, nTris + nPartBdryTris,
									   nQuads + nPartBdryQuads, nTets, nPyrs,
									   nPrisms, nHexes);

	// Store the vertices, while keeping a mapping from the full list of verts
	// to the restricted list so the connectivity can be copied properly.
	std::vector<emInt> newIndices(numVerts(), EMINT_MAX);
	for (emInt ii = 0; ii < numVerts(); ii++)
	{
		if (isVertUsed[ii])
		{
			double coords[3];
			getCoords(ii, coords);
			newIndices[ii] = UUM->addVert(coords);
			// Copy length scale for vertices from the parent; otherwise, there will be
			// mismatches in the refined meshes.
			UUM->setLengthScale(newIndices[ii], getLengthScale(ii));
		}
	}

	// Now copy connectivity.
	emInt newConn[8];
	for (emInt ii = first; ii < last; ii++)
	{
		emInt type = vecCPD[ii].getCellType();
		emInt ind = vecCPD[ii].getIndex();
		switch (type)
		{
		default:
			// Panic! Should never get here.
			assert(0);
			break;
		case TETRA_4:
		{
			conn = getTetConn(ind);
			remapIndices(4, newIndices, conn, newConn);
			UUM->addTet(newConn);
			break;
		}
		case PYRA_5:
		{
			conn = getPyrConn(ind);
			remapIndices(5, newIndices, conn, newConn);
			UUM->addPyramid(newConn);
			break;
		}
		case PENTA_6:
		{
			conn = getPrismConn(ind);
			remapIndices(6, newIndices, conn, newConn);
			UUM->addPrism(newConn);
			break;
		}
		case HEXA_8:
		{
			conn = getHexConn(ind);
			remapIndices(8, newIndices, conn, newConn);
			UUM->addHex(newConn);
			break;
		}
		} // end switch
	}	  // end loop to copy most connectivity

	for (emInt ii = 0; ii < realBdryTris.size(); ii++)
	{
		conn = getBdryTriConn(realBdryTris[ii]);
		remapIndices(3, newIndices, conn, newConn);
		UUM->addBdryTri(newConn);
	}
	for (emInt ii = 0; ii < realBdryQuads.size(); ii++)
	{
		conn = getBdryQuadConn(realBdryQuads[ii]);
		remapIndices(4, newIndices, conn, newConn);
		UUM->addBdryQuad(newConn);
	}

	// Now, finally, the part bdry connectivity.
	// TODO: Currently, there's nothing in the data structure that marks which
	// are part bdry faces.

	assert(partBdryTris.size() == tris.size());
	for (auto tri : partBdryTris)
	{
		emInt conn[] = {newIndices[tri.getCorner(0)],
						newIndices[tri.getCorner(1)],
						newIndices[tri.getCorner(2)]};
		emInt global[3] = {tri.getCorner(0), tri.getCorner(1),
						   tri.getCorner(2)};
		TriFaceVerts TF(numDivs, global, partID, -1, true);
		auto itr = tris.find(TF);
		if (itr != tris.end())
		{
			assert(itr->getGlobalCorner(0) == global[0] &&
				   itr->getGlobalCorner(1) == global[1] &&
				   itr->getGlobalCorner(2) == global[2] && itr->getPartid() == partID);
			TriFaceVerts TFV(numDivs, conn, global, partID,
							 itr->getRemoteId(), 0, EMINT_MAX,false);
			// need to be corrected, I could not generate with correct bool value unless
			// I pass all arguments

			UUM->addPartTritoSet(TFV);
		}

		UUM->addBdryTri(conn);
	}

	assert(UUM->getSizePartTris() == tris.size());

	assert(partBdryQuads.size() == quads.size());
	for (auto quad : partBdryQuads)
	{
		emInt conn[] = {newIndices[quad.getCorner(0)],
						newIndices[quad.getCorner(1)],
						newIndices[quad.getCorner(2)],
						newIndices[quad.getCorner(3)]};
		emInt global[4] = {quad.getCorner(0),
						   quad.getCorner(1), quad.getCorner(2), quad.getCorner(3)};
		QuadFaceVerts QF(numDivs, global, partID, -1, true);
		auto itr = quads.find(QF);
		if (itr != quads.end())
		{
			assert(itr->getGlobalCorner(0) == global[0] &&
				   itr->getGlobalCorner(1) == global[1] &&
				   itr->getGlobalCorner(2) == global[2] &&
				   itr->getGlobalCorner(3) == global[3] &&
				   itr->getPartid() == partID);
			QuadFaceVerts QFV(numDivs, conn, global, partID,
							  itr->getRemoteId(), 0, EMINT_MAX, false);
			// need to be corrected, I could not generate with correct bool value unless
			// I pass all arguments
			UUM->addPartQuadtoSet(QFV);
		}

		UUM->addBdryQuad(conn);
	}
	assert(UUM->getSizePartQuads() == quads.size());

	return UUM;
}

void
printMyNeigbours(std::set<int> const& neigbours, int const& rank)
{
	std::cout<<"my rank is: "<<rank<< " my neigbours are: "; 
	for(auto it:neigbours)
	{
		std::cout<<(it) <<" "<<std::endl; 
	}
	std::cout<<std::endl; 
}
void
writeTrisMap(hashTri const& tris, 
const char* fileName, std::map<int, vecTri> const&  remoteTotris, int nDivs)
{
	std:: ofstream out(fileName); 
 	

	// for(auto itr=tris.begin(); itr!=tris.end();itr++){
	// 	out<<"Remote ID: "<< itr->getRemoteId()<<
	// 	" local indices: "<<itr->getCorner(0)<<" "<<
	// 	itr->getCorner(1)<<" "<<itr->getCorner(2)<<
	// 	" sorted global: "<<itr->getGlobalSorted(0)<<" "<<
	// 	itr->getGlobalSorted(1)<<" "<<itr->getGlobalSorted(2)<<
	// 	" Unsorted global: "<<itr->getGlobalCorner(0)<<" "<<
	// 		 itr->getGlobalCorner(1)<<" "<<itr->getGlobalCorner(2)<<
	// 	 	" Part ID: "<<itr->getPartid()<<
	// 		// " Remote Indices: "<<itr->getRemoteIndices(0)<<" "<<
	// 		// itr->getRemoteIndices(1)<<" "<<
	// 		// itr->getRemoteIndices(2)<<
	// 		" boolean value: "<<itr->getGlobalCompare()<<
	// 		std::endl;
	// 		out<<"Refined verts: "<<std::endl; 
			
	// 		for (int ii = 0; ii <= nDivs; ii++) {
	//  			for (int jj = 0; jj <= nDivs-ii ; jj++) {

	//  				 out<<itr->getIntVertInd(ii,jj)<<" "; 
	//  			}
	// 		}
			
	// 		out<<std::endl;
	// 	}

	// 	out<<std::endl; 

	// out<<"map: "<<std::endl; 
	for (const auto& pair : remoteTotris) 
	{
        int key = pair.first;
        const vecTri& value = pair.second;

        out << "Key/RemoteId: " << key << std::endl;
        out << "TriFaceVerts: " << std::endl;

        for (auto itr=value.begin(); itr!=value.end();itr++) 
		{
			out<<"Remote ID: "<< itr->getRemoteId()<<
			" local indices: "<<itr->getCorner(0)<<" "<<
			itr->getCorner(1)<<" "<<itr->getCorner(2)<<
			" sorted global: "<<itr->getGlobalSorted(0)<<" "<<
			itr->getGlobalSorted(1)<<" "<<itr->getGlobalSorted(2)<<
			" Unsorted global: "<<itr->getGlobalCorner(0)<<" "<<
				itr->getGlobalCorner(1)<<" "<<itr->getGlobalCorner(2)<<
				" Part ID: "<<itr->getPartid()<<
				" Remote Indices: "<<itr->getRemoteIndices(0)<<" "<<
				itr->getRemoteIndices(1)<<" "<<
				itr->getRemoteIndices(2)<<
				" boolean value: "<<itr->getGlobalCompare()<<
				std::endl;
				out<<"Refined verts: "<<std::endl; 
				
				for (int ii = 0; ii <= nDivs; ii++) {
					for (int jj = 0; jj <= nDivs-ii ; jj++) {

						out<<itr->getIntVertInd(ii,jj)<<" "; 
					}
				}
			
			out<<std::endl;
        }

        out << std::endl;
    }


	

		

	


}

void 
UMesh::refineForMPI( const int numDivs) const
{

	boost::mpi::environment   env; 
	boost::mpi::communicator  world; 

	//std::chrono::seconds sleepDuration(20);
   // std::this_thread::sleep_for(sleepDuration);

	vecPart         parts; 
	vecCellPartData vecCPD; 
	
	std::size_t    vecCPDSize; 
	std::size_t    trisSize; 
	std::size_t    quadsSize; 
	std::size_t nParts = world.size();

	hashTri  trisS; 
	hashQuad quadsS;

	vecTri   triV;
	vecQuad  quadV;  

	int MASTER = 0; 
	int tag    = 0; 

	intToVecTri  remoteTovecTris;
	intToVecQuad remoteTovecQuads; 

	std::set<int> triNeighbrs;  
	std::set<int> quadNeighbrs; 

	vecTri trisTobeSend; 
	vecTri trisTobeRcvd;

	vecQuad quadsTobeSend; 
	vecQuad quadsTobeRcvd; 

	hashTri  recvdTris;
	hashQuad recvdQuads;  

	if(world.rank()==MASTER)
	{
		partitionCells(this, nParts, parts,vecCPD); 

		vecCPDSize = vecCPD.size(); 

		assert(vecCPDSize>0); 

		vecHashTri  VectrisHash; 
		vecHashQuad VecquadsHash;

		vecVecTri   VecTriVec; 
		vecVecQuad  vecQuadVec; 

		this->partFaceMatching(parts,vecCPD,VectrisHash,VecquadsHash); 
		for(auto  itri=0 ; itri<VectrisHash.size(); itri++)
		{
			vecTri TriVec; 
			SetToVector(VectrisHash[itri],TriVec); 
			VecTriVec.emplace_back(TriVec); 
		}
		for(auto iquad=0 ; iquad<VecquadsHash.size(); iquad++)
		{
			vecQuad QuadVec; 
			SetToVector(VecquadsHash[iquad],QuadVec); 
			vecQuadVec.emplace_back(QuadVec); 
		}

		trisS = VectrisHash[0]; // For MASTER 
		quadsS= VecquadsHash[0];
		
		for(auto irank=1 ; irank<world.size();irank++)
		{
			world.send(irank,tag,parts[irank]); 
			world.send(irank,tag,VecTriVec[irank]); 
			world.send(irank,tag,vecQuadVec[irank]); 
		}
	
	
	}
	else
	{
		parts.resize(world.size()); 
		world.recv(MASTER,tag,parts[world.rank()]);
		world.recv(MASTER,tag,triV); 
		world.recv(MASTER,tag,quadV);
		vectorToSet(triV,trisS);
		vectorToSet(quadV,quadsS); 
	}

	boost::mpi::broadcast(world,vecCPDSize,MASTER);

	if(world.rank()==MASTER)
	{
		for(auto irank=1 ; irank<world.size();irank++)
		{
			
			world.send(irank,tag,vecCPD); 
		
		}
	
	}

	if(world.rank()!= MASTER)
	{
		vecCPD.resize(vecCPDSize); 
		
		world.recv(MASTER,tag,vecCPD); 
	}

	

	auto coarse= this->extractCoarseMesh(parts[world.rank()],vecCPD,numDivs, 
	 trisS,quadsS,world.rank()); 

	auto refinedMesh = std::make_shared<UMesh>(
	 		*(coarse.get()), numDivs, world.rank());


 
	auto tris  = refinedMesh->getRefinedPartTris();

	auto quads = refinedMesh->getRefinedPartQuads();
	



	buildTrisMap(tris,remoteTovecTris,triNeighbrs); 
	buildQuadsMap(quads,remoteTovecQuads,quadNeighbrs); 

	for(auto itri:remoteTovecQuads)
	{
		int target    = itri.first; 
		quadsTobeSend = itri.second; 
		world.send(target,tag,itri.second.size()); 
		world.send(target,tag,itri.second); 
	}

	for(auto isource:quadNeighbrs)
	{
		int source= isource; 
		world.recv(source,tag,quadsSize); 
		quadsTobeRcvd.resize(quadsSize,QuadFaceVerts(1)); 
		world.recv(source,tag,quadsTobeRcvd); 
		recvdQuads.insert(quadsTobeRcvd.begin(),quadsTobeRcvd.end()); 
	}
	
	for(auto itri:remoteTovecTris)
	{
		
		int target   = itri.first;
			
		trisTobeSend = itri.second; 
			
		world.send(target,tag,itri.second.size()); 

		world.send(target,tag,trisTobeSend); 
			
	}

	for(auto isource:triNeighbrs)
	{
		
		int source= isource;
		world.recv(source,tag,trisSize); 
		trisTobeRcvd.resize(trisSize,TriFaceVerts(1)); 
		world.recv(source,tag,trisTobeRcvd); 
		recvdTris.insert(trisTobeRcvd.begin(),trisTobeRcvd.end()); 
			
	}
	

	for(auto it=tris.begin(); it!=tris.end(); it++)
	{
		std::unordered_map<emInt, emInt> localRemote;
		int rotation = getTriRotation(*it,recvdTris,numDivs);
		matchTri(*it,rotation,numDivs,recvdTris,localRemote); 
		printMatchedTris(localRemote,world.rank()); 

	}
	for(auto iq=quads.begin(); iq!=quads.end();iq++)
	{
		std::unordered_map<emInt, emInt> localRemote;
		int rotation = getQuadRotation(*iq,recvdQuads,numDivs);
		matchQuad(*iq,rotation,numDivs,recvdQuads,localRemote); 
		printMatchedQuads(localRemote,world.rank()); 
	}

	// char fileName[100]; 

	// sprintf(fileName, "Results/tris%03d.vtk", world.rank());
	// writeTrisMap(tris,fileName,remoteTovecTris,numDivs); 


	
	
}