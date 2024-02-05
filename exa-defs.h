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
 * exa-defs.h
 *
 *  Created on: Oct. 24, 2019
 *      Author: cfog
 */

#ifndef SRC_EXA_DEFS_H_
#define SRC_EXA_DEFS_H_
#include <iostream>
#include <cmath>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <algorithm>
#include "exa_config.h"
#include <boost/mpi/datatype.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/mpi.hpp> 



#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef USE_ORDERED

#include <set>
#include <map>
#define exa_set std::set
#define exa_map std::map
#define exaMultiMap std::multi_map

#else

#include <unordered_set>
#include <unordered_map>

#include <set> 
#include <map>

#define exa_set std::unordered_set
#define exa_map std::unordered_map
#define exa_multimap std::unordered_multimap

#endif

#undef PROFILE
#ifdef PROFILE
#include <valgrind/callgrind.h>
#else
#define CALLGRIND_TOGGLE_COLLECT
#endif

#define MAXADJ 6
#define MAX_DIVS 50
#define FILE_NAME_LEN 1024
#define TOLTEST 1e-9
#define MASTER 0 
typedef int32_t emInt;
#define EMINT_MAX UINT_MAX

#if (HAVE_CGNS == 0)
#define CGNS_ENUMV(a) a
#define TRI_3 5
#define QUAD_4 7
#define TETRA_4 10
#define PYRA_5 12
#define PENTA_6 14
#define HEXA_8 17
#define TRI_10 26
#define QUAD_16 28
#define TETRA_20 30
#define PYRA_30 33
#define PENTA_40 36
#define HEXA_64 39
#endif

// Some vector operators

#define DIFF(a,b) {a[0] -b[0], a[1]-b[1],a[2]-b[2]}
#define SCALE(x, a, y) y[0]=(a)*x[0]; y[1]=(a)*x[1]; y[2]=(a)*x[2]
#define LEN(x) sqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2])
#define CROSS(a,b,c) c[0] = a[1]*b[2]-a[2]*b[1], c[1] = a[2]*b[0]-a[0]*b[2], c[2] = a[0]*b[1]-a[1]*b[0]
#define DOT(a,b) (a[0]*b[0] + a[1]*b[1]+ a[2]*b[2])
#define NORMALIZE(a) {double tmp = 1./sqrt(DOT(a,a)); a[0]*=tmp; a[1]*=tmp; a[2]*=tmp;}

inline double safe_acos(const double arg) {
	if (arg < -1) return M_PI;
	else if (arg > 1) return 0;
	else return acos(arg);
}

inline double exaTime() {
#ifdef _OPENMP
	return omp_get_wtime();
#else
	return clock() / double(CLOCKS_PER_SEC);
#endif
}

class Edge {
private:
	emInt v0, v1;
public:
	Edge(emInt vA, emInt vB) {
		if (vA < vB) {
			v0 = vA;
			v1 = vB;
		}
		else {
			v0 = vB;
			v1 = vA;
		}
	}
	bool operator<(const Edge &E) const {
		return (v0 < E.v0 || (v0 == E.v0 && v1 < E.v1));
	}
	bool operator==(const Edge &E) const {
		return v0 == E.v0 && v1 == E.v1;
	}
	emInt getV0() const {
		return v0;
	}

	emInt getV1() const {
		return v1;
	}
};

struct EdgeVerts {
	emInt m_verts[MAX_DIVS + 1];
	double m_param_t[MAX_DIVS + 1];
	double m_totalDihed;
};

class FaceVerts
{
private:
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive &ar,const unsigned int /*version*/)
	{
		ar &m_remoteId;
		ar &m_global;
		ar &m_sortedGlobal;
		//ar &m_remote;
		//ar &m_sortedRemote; 
		//ar &m_corners; 
		//ar &m_sorted; 
		//ar &m_cornerUVW; 
		ar &m_nCorners; 
		ar &m_nDivs; 
		ar &m_intVerts; 
		//ar &m_param_st; 
		//ar &m_param_uvw; 
		//ar &m_volElem; 
		//ar &m_volElemType; 
		//ar &m_bothSidesDone; 
		ar &m_partId; 
		ar &m_globalComparison; 

	}
protected:
	// CUATION: ANY CHANGE IN MEMBER SIZE OR DELETION OR ADDING SHOULD BE REFLECTED IN ITS MPI TYPE

	emInt m_global[4], m_sortedGlobal[4];
	emInt m_remote[4], m_sortedRemote[4];
	emInt m_corners[4], m_sorted[4];
	double m_cornerUVW[4][3];
	int64_t m_nCorners, m_nDivs;
	std::vector<std::vector<emInt>> m_intVerts; 
	//emInt m_intVerts [MAX_DIVS + 1][MAX_DIVS + 1]; 
	//double m_param_st[MAX_DIVS + 1][MAX_DIVS + 1][2];
	 std::vector<std::vector<std::vector<double>>> m_param_st;
	//double m_param_uvw[MAX_DIVS + 1][MAX_DIVS + 1][3];
	std::vector<std::vector<std::vector<double>>> m_param_uvw; 
	emInt m_volElem, m_volElemType;
	bool m_bothSidesDone;
	emInt m_partId; 
	emInt m_remoteId; 
	bool m_globalComparison; 
public:
	FaceVerts(){};
	FaceVerts(const int nDivs, const emInt NC = 0) : 
	m_nCorners(NC), 
	m_nDivs(nDivs), 
	m_intVerts (nDivs + 1, std::vector<emInt>(nDivs + 1)), 
    m_param_st (nDivs + 1, std::vector<std::vector<double>>(nDivs + 1, std::vector<double>(2))),
	m_param_uvw(nDivs + 1, std::vector<std::vector<double>>(nDivs + 1, std::vector<double>(3))),
	m_volElem(EMINT_MAX), 
	m_volElemType(0),
	m_bothSidesDone(false)

	{
		assert(NC == 3 || NC == 4);
	}
	//virtual ~FaceVerts() {};
	bool isValidIJ(const int ii, const int jj) const
	{
		bool retVal = (ii >= 0) && (jj >= 0);
		if (m_nCorners == 3) {
			retVal = retVal && ((ii + jj) <= m_nDivs);
		}
		else {
			assert(m_nCorners == 4);
			retVal = retVal && (ii <= m_nDivs) && (jj <= m_nDivs);
		}
		return retVal;
	}
	bool isValidParam(const double param) const {
		// This isn't comprehensive, in that not all faces
		// can have this full parameter range.  But the more
		// accurate test requires significantly more information.
		return (param >= 0 && param <= 1);
	}
	//virtual void setupSorted() = 0;
	void setupSorted() {}; 
	emInt getSorted(const int ii) const
	{
		return m_sorted[ii];
	}
	void setIntVertInd(const int ii, const int jj, const emInt vert) {
		assert(isValidIJ(ii, jj));
		m_intVerts[ii][jj] = vert;
	}
	emInt getIntVertInd(const int ii, const int jj) const
	{
		assert(isValidIJ(ii, jj));
		return m_intVerts[ii][jj];
	}
	void setVertSTParams(const int ii, const int jj, const double st[2]){
		assert(isValidIJ(ii, jj));
		assert(isValidParam(st[0]));
		assert(isValidParam(st[1]));
		m_param_st[ii][jj][0] = st[0];
		m_param_st[ii][jj][1] = st[1];
	}
	//virtual void getVertAndST(const int ii, const int jj, emInt &vert,
		//					  double st[2], const int rotCase = 0) const = 0;
	void setVertUVWParams(const int ii, const int jj, const double uvw[3])
	{
		assert(isValidIJ(ii, jj));
		assert(isValidParam(uvw[0]));
		assert(isValidParam(uvw[1]));
		assert(isValidParam(uvw[2]));
		m_param_uvw[ii][jj][0] = uvw[0];
		m_param_uvw[ii][jj][1] = uvw[1];
		m_param_uvw[ii][jj][2] = uvw[2];
	}
	void getVertUVWParams(const int ii, const int jj, double uvw[3]) const {
		assert(isValidIJ(ii, jj));
		uvw[0] = m_param_uvw[ii][jj][0];
		uvw[1] = m_param_uvw[ii][jj][1];
		uvw[2] = m_param_uvw[ii][jj][2];
		assert(isValidParam(uvw[0]));
		assert(isValidParam(uvw[1]));
		assert(isValidParam(uvw[2]));
	}
	// void setCorners(const emInt cA, const emInt cB, const emInt cC,
	// 				const emInt cD = EMINT_MAX)
	// {
	// 	m_corners[0] = cA;
	// 	m_corners[1] = cB;
	// 	m_corners[2] = cC;
	// 	m_corners[3] = cD;
	// 	setupSorted();
	// }
	void setGlobalCorners(const emInt cA, const emInt cB, const emInt cC,
			const emInt cD = EMINT_MAX) {
		m_global[0] = cA;
		m_global[1] = cB;
		m_global[2] = cC;
		m_global[3] = cD;
		// setupSorted();
	}
	emInt getCorner(const int ii) const {
		assert(ii >= 0 && ii < m_nCorners);
		return m_corners[ii];
	}
	emInt getGlobalCorner(const int ii) const{
		assert(ii >= 0 && ii < m_nCorners);
		return m_global[ii];
	}
	emInt getSortedGlobal(const int ii) const{
		assert(ii >= 0 && ii < m_nCorners);
		return m_sortedGlobal[ii];
	}
	emInt getRemoteIndices(const int ii) const{
		assert(ii >= 0 && ii < m_nCorners);
		return m_remote[ii];
	}
	emInt getSortedRemoteIndices(const int ii) const {
		assert(ii >= 0 && ii < m_nCorners);
		return m_sortedRemote[ii];
	}
	//virtual void computeParaCoords(const int ii, const int jj,
								//   double st[2]) const = 0;
	emInt getVolElement() const
	{
		return m_volElem;
	}

	emInt getVolElementType() const {
		return m_volElemType;
	}
	emInt getPartid () const {
		return m_partId; 
	}
	void setRemotePartID (const emInt remotePartid_){
		m_remoteId=remotePartid_; 
	}
	void setPartID(const emInt partID_){
		m_partId=partID_; 
	}
	// void setRemoteIndices(const emInt remote[3]){
	// 	for(auto i=0; i<3; i++){
	// 		remoteIndices[i]=remote[i]; 
	// 	}

	// }
	void setRemoteIndices(const emInt* remote){
		for(auto i=0; i<4; i++){
			m_remote[i]=*(remote + i); 
		}

	}
	emInt getRemoteId ()const{
		return m_remoteId; 
	}

	bool getGlobalCompare() const {
		return m_globalComparison; 
	}
	emInt getNumDivs() const
	{
		return m_nDivs;
	}
	void setCompare(const bool compare)
	{
		m_globalComparison=compare; 
	}
	//friend MPI_Datatype register_mpi_type(FaceVerts const &);


};
//Notes from Documentation : 
// It may not be immediately obvious how this one template serves 
//for both saving data to an archive as well as loading data from the archive
// The key is that the & operator is defined as << for output archives and as >> input archives.
class TriFaceVerts : public FaceVerts
{
private: 

	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive &ar, const unsigned int /*version*/)
  	{
    	ar & boost::serialization::base_object<FaceVerts>(*this);
   
  	}


public:
	TriFaceVerts(){}; // Dangerous! CHANGE IT LATER ON
	TriFaceVerts(const int nDivs, const emInt partID = -1, bool globalComparison = false) : FaceVerts(nDivs, 3) {
		m_partId=partID; 
		m_globalComparison=globalComparison;
	}
	TriFaceVerts(const int nDivs, emInt v0, const emInt v1, const emInt v2,
			const emInt type = 0, const emInt elemInd = EMINT_MAX, 
			const emInt partID=-1, const emInt remoteId=-1 ,bool globalComparison=false);

	TriFaceVerts(const int nDivs, const emInt local[3], 
	const emInt global[3],const emInt partid_=-1, const emInt remoteID=-1 ,
	const emInt type=0 ,const emInt elemInd=EMINT_MAX,bool globalComparison=false);

	// TriFaceVerts(const int nDivs,const emInt global[3],
	// const emInt partid_=-1, const emInt remoteID=-1 ,const emInt type=0 ,
	// const emInt elemInd=EMINT_MAX,const bool globalComparison=false);

	TriFaceVerts(const int nDivs, const emInt global[3],
				 const emInt partid_ = -1, const emInt remoteID = -1, const bool globalComparison = false,
				 const emInt type = 0,
				 const emInt elemInd = EMINT_MAX);

	TriFaceVerts(const int nDivs, const emInt local[3],
				 const emInt global[3], const emInt remoteIndices_[3], const emInt partid_,
				 const emInt remoteID,
				 const emInt type = 0, const emInt elemInd = EMINT_MAX, bool globalComparison = false);

	//virtual ~TriFaceVerts() {}; 
	//	void allocVertMemory() {
	//		m_intVerts = new emInt[MAX_DIVS - 2][MAX_DIVS - 2];
	//	}
	//	void freeVertMemory() const {
	//		if (m_intVerts) delete[] m_intVerts;
	//	}
	void computeParaCoords(const int ii, const int jj,
								   double st[2]) const;
	// virtual void computeParaCoords(const int ii, const int jj,
	// 							   double st[2]) const;
	//virtual void setupSorted();
	void setupSorted();
	void getVertAndST(const int ii, const int jj, emInt &vert,
					  double st[2], const int rotCase = 0) const;
	void getTrueIJ(const int ii, const int jj,
				   int &trueI, int &trueJ, const int rotCase = 0) const;
	friend bool operator<(const TriFaceVerts &a, const TriFaceVerts &b);
	friend bool operator==(const TriFaceVerts &a, const TriFaceVerts &b);
	friend inline bool compare(const TriFaceVerts &a, const TriFaceVerts &b); 
	friend inline MPI_Datatype register_mpi_type(TriFaceVerts const &);
	void setCorners(const emInt cA, const emInt cB, const emInt cC,
					const emInt cD = EMINT_MAX)
	{
		m_corners[0] = cA;
		m_corners[1] = cB;
		m_corners[2] = cC;
		m_corners[3] = cD;
		setupSorted();
	}
};

struct QuadFaceVerts : public FaceVerts
{
	private: 
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive &ar, const unsigned int /*version*/)
  	{
    	ar & boost::serialization::base_object<FaceVerts>(*this);
   
  	}

public:
	QuadFaceVerts(){}; // Dangerous! CHANGE IT LATER ON
	QuadFaceVerts(const int nDivs, const emInt partID = -1,
				  const emInt _remotePartid = -1, bool globalCompare = false) : FaceVerts(nDivs, 4) {
					m_partId=partID; 
					m_remoteId=_remotePartid; 
	 				m_globalComparison=globalCompare; 
				  }
	QuadFaceVerts(const int nDivs, const emInt v0, const emInt v1, const emInt v2, const emInt v3,

				  const emInt type = 0, const emInt elemInd = EMINT_MAX,
				  const emInt partID = -1, const emInt remoteID = -1, bool globalCompare = false);

	QuadFaceVerts(const int nDivs, const emInt local[4],
				  const emInt global[4], const emInt partid_ = -1, const emInt remoteID = -1, const emInt type = 0, const emInt elemInd = EMINT_MAX,
				  bool globalCompare = false);
	// QuadFaceVerts(const int nDivs,const emInt global[4],const emInt partid_=-1, const emInt remoteID=-1
	// ,const emInt type=0 ,const emInt elemInd=EMINT_MAX,
	// bool globalCompare=false);
	QuadFaceVerts(const int nDivs, const emInt global[4], const emInt partid_ = -1,
				  const emInt remoteID = -1, bool globalCompare = false, const emInt type = 0, const emInt elemInd = EMINT_MAX);
	QuadFaceVerts(const int nDivs, const emInt local[4],
				  const emInt global[4], const emInt remotelocal[4], const emInt partid_ = -1, const emInt remoteID = -1, const emInt type = 0, const emInt elemInd = EMINT_MAX,
				  bool globalCompare = false);

	//virtual ~QuadFaceVerts() {}
	void computeParaCoords(const int ii, const int jj,
								   double st[2]) const;
	// virtual void computeParaCoords(const int ii, const int jj,
	// 							   double st[2]) const;
	//virtual void setupSorted();
	void setupSorted();
	void getVertAndST(const int ii, const int jj, emInt &vert,
					  double st[2], const int rotCase = 0) const;
	void getTrueIJ(const int ii, const int jj,
				   int &trueI, int &trueJ, const int rotCase = 0) const;
	friend bool operator<(const QuadFaceVerts &a, const QuadFaceVerts &b);
	friend bool operator==(const QuadFaceVerts &a, const QuadFaceVerts &b);
	friend inline MPI_Datatype register_mpi_type(QuadFaceVerts const &);
	void setCorners(const emInt cA, const emInt cB, const emInt cC,
					const emInt cD = EMINT_MAX)
	{
		m_corners[0] = cA;
		m_corners[1] = cB;
		m_corners[2] = cC;
		m_corners[3] = cD;
		setupSorted();
	}
};

namespace std {
	template<> struct hash<TriFaceVerts> {
		typedef TriFaceVerts argument_type;
		typedef std::size_t result_type;
		result_type operator()(const argument_type& TFV) const noexcept
		{
			result_type h0,h1,h2;

			if(TFV.getGlobalCompare()==false)
			{
				// const result_type h0 = TFV.getSorted(0);
				// const result_type h1 = TFV.getSorted(1);
				// const result_type h2 = TFV.getSorted(2);
				h0 = TFV.getSorted(0); 
				h1 = TFV.getSorted(1); 
				h2 = TFV.getSorted(2); 
				//return (h0 ^ (h1 << 1)) ^ (h2 << 2);
			}
			if(TFV.getGlobalCompare()==true)
			{
				h0 = TFV.getSortedGlobal(0); 
				h1 = TFV.getSortedGlobal(1); 
				h2 = TFV.getSortedGlobal(2); 

				// const result_type h0 = TFV.getSortedGlobal(0);
				// const result_type h1 = TFV.getSortedGlobal(1);
				// const result_type h2 = TFV.getSortedGlobal(2);
				//return (h0 ^ (h1 << 1)) ^ (h2 << 2);
			}
			return (h0 ^ (h1 << 1)) ^ (h2 << 2);

			
		}
	};

	template<> struct hash<QuadFaceVerts> {
		typedef QuadFaceVerts argument_type;
		typedef std::size_t result_type;
		result_type operator()(const argument_type& QFV) const noexcept
		{
			if(QFV.getGlobalCompare()==false){
				const result_type h0 = QFV.getSorted(0);
				const result_type h1 = QFV.getSorted(1);
				const result_type h2 = QFV.getSorted(2);
				const result_type h3 = QFV.getSorted(3);
				return h0 ^ (h1 << 1) ^ (h2 << 2) ^ (h3 << 3);
			}else{
				const result_type h0 = QFV.getSortedGlobal(0);
				const result_type h1 = QFV.getSortedGlobal(1);
				const result_type h2 = QFV.getSortedGlobal(2);
				const result_type h3 = QFV.getSortedGlobal(3);
				return h0 ^ (h1 << 1) ^ (h2 << 2) ^ (h3 << 3);

			}

		}
	};

	template<> struct hash<Edge> {
		typedef Edge argument_type;
		typedef std::size_t result_type;
		result_type operator()(const argument_type& E) const noexcept
		{
			const result_type h0 = E.getV0();
			const result_type h1 = E.getV1();
			return (h0 ^ (h1 << 1));
		}
	};
}

bool operator==(const TriFaceVerts& a, const TriFaceVerts& b);
bool operator<(const TriFaceVerts& a, const TriFaceVerts& b);
bool operator==(const QuadFaceVerts& a, const QuadFaceVerts& b);
bool operator<(const QuadFaceVerts& a, const QuadFaceVerts& b);

struct RefineStats {
	double refineTime, extractTime;
	emInt cells;
	size_t fileSize;
};
inline emInt getTriRotation(const TriFaceVerts &localTri, 
const exa_set<TriFaceVerts> &remote,emInt nDivs){

	//emInt Lvert0 = localTri.getRemoteIndices(0);
	//emInt Lvert1 = localTri.getRemoteIndices(1);
	//emInt Lvert2 = localTri.getRemoteIndices(2);

	emInt global[3]=
	{
		localTri.getGlobalCorner(0), 
		localTri.getGlobalCorner(1), 
		localTri.getGlobalCorner(2)
	};

	TriFaceVerts TF(nDivs,global); 
	TF.setCompare(true); 
	//TriFaceVerts TF(nDivs,Lvert0,Lvert1,Lvert2);

	auto iterTris=remote.find(TF); 
	assert(iterTris!=remote.end()); 
	emInt vert0= localTri.getGlobalCorner(0); 
	emInt vert1= localTri.getGlobalCorner(1); 
	emInt vert2= localTri.getGlobalCorner(2); 
	// emInt globalIterTris [3]={
	// 	iterTris->getGlobalCorner(0),
	// 	iterTris->getGlobalCorner(1),
	// 	iterTris->getGlobalCorner(2)

	// };
	int rotCase = 0;
		for (int cc = 0; cc < 3; cc++) {
			if (vert0 == iterTris->getGlobalCorner(cc)) {
				if (vert1 == iterTris->getGlobalCorner((cc+1)%3)){
					assert(vert2 == iterTris->getGlobalCorner((cc+2)%3));
					rotCase = cc+1;
				}
				else {
					assert(vert1 == iterTris->getGlobalCorner((cc+2)%3));
					assert(vert2 == iterTris->getGlobalCorner((cc+1)%3));
					rotCase = -(cc+1);
				}
			}
		}
		assert(rotCase != 0);
		return rotCase; 

}
inline emInt getQuadRotation(const QuadFaceVerts &localQuad, 
const exa_set<QuadFaceVerts> &remote,emInt nDivs){

	// emInt Lvert0 = localQuad.getRemoteIndices(0);
	// emInt Lvert1 = localQuad.getRemoteIndices(1);
	// emInt Lvert2 = localQuad.getRemoteIndices(2);
	// emInt Lvert3 = localQuad.getRemoteIndices(3); 
	emInt global [4]=
	{
		localQuad.getGlobalCorner(0),
		localQuad.getGlobalCorner(1), 
		localQuad.getGlobalCorner(2), 
		localQuad.getGlobalCorner(3)
	}; 


	QuadFaceVerts QF (nDivs,global); 
	QF.setCompare(true); 
	//printQuads(remote,nDivs); 



	//QuadFaceVerts QF(nDivs,Lvert0,Lvert1,Lvert2,Lvert3);

	auto iterQuads=remote.find(QF); 

	assert(iterQuads!=remote.end()); 

	emInt vert0= localQuad.getGlobalCorner(0); 
	emInt vert1= localQuad.getGlobalCorner(1); 
	emInt vert2= localQuad.getGlobalCorner(2); 
	emInt vert3= localQuad.getGlobalCorner(3); 
	// emInt globalIterTris [4]={
	// 	iterQuads->getGlobalCorner(0),
	// 	iterQuads->getGlobalCorner(1),
	// 	iterQuads->getGlobalCorner(2),
	// 	iterQuads->getGlobalCorner(3)

	// };
	int rotCase = 0;
	
	for (int cc = 0; cc < 4; cc++)
	{
		if (vert0 == iterQuads->getGlobalCorner(cc)) 
		{
			if (vert1 == iterQuads->getGlobalCorner((cc+1)%4)) 
			{
				// Oriented forward; bdry quad
				assert(vert2 == iterQuads->getGlobalCorner((cc+2)%4));
				assert(vert3 == iterQuads->getGlobalCorner((cc+3)%4));
				rotCase = cc+1;
			}
			else 
			{
				assert(vert1 == iterQuads->getGlobalCorner((cc+3)%4));
				assert(vert2 == iterQuads->getGlobalCorner((cc+2)%4));
				assert(vert3 == iterQuads->getGlobalCorner((cc+1)%4));
				rotCase = -(cc+1);
			}
		}
	}
	assert(rotCase != 0);

	return rotCase; 

}; 

inline void matchTri(const TriFaceVerts &localTri, 
const emInt rotation, const emInt nDivs, 
const exa_set<TriFaceVerts> &remoteTriSet
,std::unordered_map<emInt, emInt> &localRemote){
	

	// emInt vert0 = localTri.getRemoteIndices(0);
	// emInt vert1 = localTri.getRemoteIndices(1);
	// emInt vert2 = localTri.getRemoteIndices(2);

	emInt global[3]=
	{
		localTri.getGlobalCorner(0), 
		localTri.getGlobalCorner(1), 
		localTri.getGlobalCorner(2)
	}; 

	TriFaceVerts TF(nDivs,global); 
	TF.setCompare(true); 


	//TriFaceVerts TF(nDivs,vert0,vert1,vert2);

	auto itRemote= remoteTriSet.find(TF); 
	assert(itRemote!=remoteTriSet.end()); 
	assert(localTri.getPartid()==itRemote->getRemoteId()); 
	assert(localTri.getRemoteId()==itRemote->getPartid()); 
	// for(auto i=0; i<3; i++){
	// 	assert(localTri.getCorner(i)==
	// 	itRemote->getRemoteIndices(i)); 
	// 	assert(itRemote->getCorner(i)==localTri.getRemoteIndices(i)); 
	// }


	for (int ii = 0; ii <= nDivs ; ii++) 
	{
	 	for (int jj = 0; jj <= nDivs-ii ; jj++) 
		{

			int trueI; 
			int trueJ; 
			itRemote->getTrueIJ(ii,jj,trueI,trueJ,rotation); 
			
			emInt vertLocal=localTri.getIntVertInd(trueI,trueJ); 
			emInt vertRemote=itRemote->getIntVertInd(ii,jj); 
			localRemote.insert({vertLocal,vertRemote}); 
			
			// std::cout<<"ii: "<<ii<<" jj: "<<jj<<"--- ( "<<
			// vertLocal<<" "<<
			// vertRemote<<" )"
			// <<"-> ("<<local->getX(vertLocal)<<", "<<
			// local->getY(vertLocal)<<", "<<local->getZ(vertLocal)<<" ) "<<" ( "<<
			// remote->getX(vertRemote)<<", "<<remote->getY(vertRemote)<<", "<<
			// remote->getZ(vertRemote)<<" ) "
			//   <<std::endl; 
 			
		}
	}
	//std::cout<<std::endl; 

}
inline void matchQuad(const QuadFaceVerts &localQuad, 
const emInt rotation, const emInt nDivs, 
const exa_set<QuadFaceVerts> &remoteQuadSet,
std::unordered_map<emInt, emInt> &localRemote){
	//exa_set<TriFaceVerts> remoteTriSet= remote->getRefinedPartTris();

	// emInt vert0 = localQuad.getRemoteIndices(0);
	// emInt vert1 = localQuad.getRemoteIndices(1);
	// emInt vert2 = localQuad.getRemoteIndices(2);
	// emInt vert3 = localQuad.getRemoteIndices(3); 

	emInt global [4]=
	{
		localQuad.getGlobalCorner(0), 
		localQuad.getGlobalCorner(1), 
		localQuad.getGlobalCorner(2), 
		localQuad.getGlobalCorner(3)
	}; 
	QuadFaceVerts QF(nDivs,global); 
	QF.setCompare(true); 
	

	//QuadFaceVerts QF(nDivs,vert0,vert1,vert2,vert3);
	

	auto itRemote= remoteQuadSet.find(QF); 
	assert(itRemote!=remoteQuadSet.end()); 
	assert(localQuad.getPartid()==itRemote->getRemoteId()); 
	assert(localQuad.getRemoteId()==itRemote->getPartid()); 
	// for(auto i=0; i<4; i++){
	// 	assert(localQuad.getCorner(i)==
	// 	itRemote->getRemoteIndices(i)); 
	// 	assert(itRemote->getCorner(i)==localQuad.getRemoteIndices(i)); 
	// }
	for (int ii = 0; ii <= nDivs ; ii++) 
	{
	 	for (int jj = 0; jj <= nDivs ; jj++) 
		{

			int trueI; 
			int trueJ; 
			itRemote->getTrueIJ(ii,jj,trueI,trueJ,rotation); 
			emInt vertLocal=localQuad.getIntVertInd(ii,jj); 
			emInt vertRemote=itRemote->getIntVertInd(trueI,trueJ); 
			//emInt vertLocal  = localQuad.getIntVertInd(trueI,trueJ); 
			//emInt vertRemote = itRemote->getIntVertInd(ii,jj);
			//std::cout<<vertLocal<<" "<<vertRemote<<std::endl;
			localRemote.insert({vertLocal,vertRemote});  
			
			// std::cout<<"ii: "<<ii<<" jj: "<<jj<<"--- ( "<<
			// vertLocal<<" "<<
			// vertRemote<<" )"
			// <<"-> ("<<local->getX(vertLocal)<<", "<<
			// local->getY(vertLocal)<<", "<<local->getZ(vertLocal)<<" ) "<<" ( "<<
			// remote->getX(vertRemote)<<", "<<remote->getY(vertRemote)<<", "<<
			// remote->getZ(vertRemote)<<" ) "
			//   <<std::endl; 

	 			
		}
	}	
}
inline 
void findRotationAndMatchTris (
const TriFaceVerts &localTri, 
const emInt nDivs, 
const exa_set<TriFaceVerts> &remoteTriSet
,std::unordered_map<emInt, emInt> &localRemote)
{

	emInt global[3]=
	{
		localTri.getGlobalCorner(0), 
		localTri.getGlobalCorner(1), 
		localTri.getGlobalCorner(2)
	};

	TriFaceVerts TF(nDivs,global); 
	TF.setCompare(true); 
	

	auto iterTris=remoteTriSet.find(TF); 
	assert(iterTris!=remoteTriSet.end());
	assert(localTri.getPartid()  == iterTris->getRemoteId()); 
	assert(localTri.getRemoteId()== iterTris->getPartid());  

	emInt vert0= localTri.getGlobalCorner(0); 
	emInt vert1= localTri.getGlobalCorner(1); 
	emInt vert2= localTri.getGlobalCorner(2); 

	int rotCase = 0;
		for (int cc = 0; cc < 3; cc++) {
			if (vert0 == iterTris->getGlobalCorner(cc)) {
				if (vert1 == iterTris->getGlobalCorner((cc+1)%3)){
					assert(vert2 == iterTris->getGlobalCorner((cc+2)%3));
					rotCase = cc+1;
				}
				else {
					assert(vert1 == iterTris->getGlobalCorner((cc+2)%3));
					assert(vert2 == iterTris->getGlobalCorner((cc+1)%3));
					rotCase = -(cc+1);
				}
			}
		}
		assert(rotCase != 0);
	
	// Once found totation, go a haed to match tris 

	for (int ii = 0; ii <= nDivs ; ii++) 
	{
	 	for (int jj = 0; jj <= nDivs-ii ; jj++) 
		{

			int trueI; 
			int trueJ; 
			iterTris->getTrueIJ(ii,jj,trueI,trueJ,rotCase); 
			
			emInt vertLocal=localTri.getIntVertInd(trueI,trueJ); 
			emInt vertRemote=iterTris->getIntVertInd(ii,jj); 
			localRemote.insert({vertLocal,vertRemote}); 
			

 			
		}
	}
 
}

inline
void findRotationAndMatchQuads(
const QuadFaceVerts &localQuad, 
const emInt nDivs, 
const exa_set<QuadFaceVerts> &remoteQuadSet,
std::unordered_map<emInt, emInt> &localRemote)
{

	emInt global [4]=
	{
		localQuad.getGlobalCorner(0),
		localQuad.getGlobalCorner(1), 
		localQuad.getGlobalCorner(2), 
		localQuad.getGlobalCorner(3)
	}; 


	QuadFaceVerts QF (nDivs,global); 
	QF.setCompare(true); 

	auto iterQuads=remoteQuadSet.find(QF); 

	assert(iterQuads!=remoteQuadSet.end());
	assert(localQuad.getPartid()  == iterQuads->getRemoteId()); 
	assert(localQuad.getRemoteId()== iterQuads->getPartid());  

	emInt vert0= localQuad.getGlobalCorner(0); 
	emInt vert1= localQuad.getGlobalCorner(1); 
	emInt vert2= localQuad.getGlobalCorner(2); 
	emInt vert3= localQuad.getGlobalCorner(3); 

	int rotCase = 0;
	
	for (int cc = 0; cc < 4; cc++)
	{
		if (vert0 == iterQuads->getGlobalCorner(cc)) 
		{
			if (vert1 == iterQuads->getGlobalCorner((cc+1)%4)) 
			{
				// Oriented forward; bdry quad
				assert(vert2 == iterQuads->getGlobalCorner((cc+2)%4));
				assert(vert3 == iterQuads->getGlobalCorner((cc+3)%4));
				rotCase = cc+1;
			}
			else 
			{
				assert(vert1 == iterQuads->getGlobalCorner((cc+3)%4));
				assert(vert2 == iterQuads->getGlobalCorner((cc+2)%4));
				assert(vert3 == iterQuads->getGlobalCorner((cc+1)%4));
				rotCase = -(cc+1);
			}
		}
	}
	assert(rotCase != 0);

	

	for (int ii = 0; ii <= nDivs ; ii++) 
	{
	 	for (int jj = 0; jj <= nDivs ; jj++) 
		{

			int trueI; 
			int trueJ; 
			iterQuads->getTrueIJ(ii,jj,trueI,trueJ,rotCase); 
			emInt vertLocal=localQuad.getIntVertInd(ii,jj); 
			emInt vertRemote=iterQuads->getIntVertInd(trueI,trueJ); 
			
			localRemote.insert({vertLocal,vertRemote});  

		}
	}	
}
template <typename T>
inline void SetToVector(const std::unordered_set<T>& sourceSet, 
std::vector<T>& destinationVector) {
    destinationVector.clear();
	// Change to the assignment 
   	for (const auto& element : sourceSet) {
        destinationVector.push_back(element);
    }
}

template <typename T>
inline void vectorToSet(const std::vector<T>& sourceVector, 
std::unordered_set<T>& destinationSet) {
    destinationSet.clear();
  
    for (const auto& element : sourceVector) {
        destinationSet.insert(element);
    }
	assert(destinationSet.size()==sourceVector.size()); 
}
struct hashFunctionCell2Cell 
{
    std::size_t operator()(const std::pair<emInt, emInt>& p) const 
	{
        // Combine the hash values of the pair's components
        std::size_t hashValue = std::hash<emInt>()(p.first);
        hashValue ^= std::hash<emInt>()(p.second) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
        return hashValue;
    }
};



struct hashFunctionFace2Cell 
{
    std::size_t operator()(const std::vector<int>& s) const {
        // Implement a custom hash function for std::set<int>
        // You might want to combine the hash values of individual elements in the set
        // to create a hash value for the entire set.
        // Example:
        std::size_t hash_value = 0;
        for (int element : s) {
            hash_value ^= std::hash<int>()(element) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
        }
        return hash_value;
    }
};


struct pairHash {
    template <class T1, class T2>
    std::size_t operator () (std::pair<T1,T2> const& pair) const {
        auto first = std::min(pair.first, pair.second);
        auto second = std::max(pair.first, pair.second);

        std::size_t h1 = std::hash<T1>{}(first);
        std::size_t h2 = std::hash<T2>{}(second);

        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return h1 ^ h2;  
    }
};


using setTri         			 = std::set<TriFaceVerts>; 
using setQuad                    = std::set<QuadFaceVerts>; 
using hashTri                    = std::unordered_set<TriFaceVerts>; 
using hashQuad                   = std::unordered_set<QuadFaceVerts>; 
using vecHashTri                 = std::vector<hashTri>; 
using vecHashQuad                = std::vector<hashQuad>;
using vecTri                     = std::vector<TriFaceVerts>; 
using vecQuad                    = std::vector<QuadFaceVerts>; 
using vecVecTri                  = std::vector<vecTri>  ; 
using vecVecQuad                 = std::vector<vecQuad> ; 
using intToVecTri		         = std::map<int,vecTri> ; 
using intToVecQuad               = std::map<int,vecQuad>; 
using TableTri2TableIndex2Index	 = std::unordered_map< TriFaceVerts , std::unordered_map<emInt,emInt>>;
using TableQuad2TableIndex2Index = std::unordered_map< QuadFaceVerts, std::unordered_map<emInt,emInt>>;
using multimpFace2Cell			 = std::unordered_multimap < std::vector<emInt>, std::pair<emInt,emInt>, hashFunctionFace2Cell>;
using TableCell2Cell			 = std::unordered_map < std::pair<emInt,emInt>, std::set<emInt>, hashFunctionCell2Cell>;
void
inline buildTrisMap(hashTri const& tris, std::map<int, vecTri> &remoteTotris,
std::set<int> &neighbors)
{

	for(auto it= tris.begin(); it!=tris.end(); it++ )
	{
		int remoteId= it->getRemoteId(); 
		neighbors.insert(remoteId); 
		remoteTotris[remoteId].push_back(*it); 

	}	
}
void
inline buildQuadsMap(hashQuad const& quads, std::map<int, vecQuad> &remoteToquads,
std::set<int> &neighbors)
{
	
	for(auto it= quads.begin(); it!=quads.end(); it++ )
	{
		int remoteId= it->getRemoteId(); 
		neighbors.insert(remoteId); 
		remoteToquads[remoteId].push_back(*it); 

	}
}
BOOST_SERIALIZATION_ASSUME_ABSTRACT(FaceVerts)

inline 
void printMultiMap(const std::multimap<std::set<emInt>, std::pair<emInt, emInt>> &map) 
{
    for (const auto &entry : map) 
	{
        const auto &key   = entry.first;
        const auto &value = entry.second;

        // Print the key (set of emInt)
        std::cout << "Key: { ";
        for (const auto &elem : key) {
            std::cout << elem << " ";
        }
        std::cout << "} ";

        // Print the value (pair of emInt and char*)
        std::cout << "Value: {" << value.first << ", " << value.second << "}\n";
    }
}
inline 
void printTriFaceVerts(const std::vector<TriFaceVerts>& tris) 
{
	for (const auto& tri : tris) 
	{

		std::cout << "Global corners: [" << tri.getGlobalCorner(0) << ", " << tri.getGlobalCorner(1) << ", " << tri.getGlobalCorner(2) << "]" << std::endl;
		std::cout << "Part ID: " << tri.getPartid() << std::endl;
		std::cout << "Remote Part ID: " << tri.getRemoteId() << std::endl;
		std::cout << "Is True: " << std::boolalpha << tri.getGlobalCompare() << std::endl;
		std::cout << std::endl;
	}
}
inline 
void preMatchingPartBdryTris(const emInt numDivs,const setTri& partBdryTris, vecVecTri &tris)
{
	std::size_t k = 0;
	for (auto itr = partBdryTris.begin(); itr != partBdryTris.end(); itr++)
	{
		k++;
		auto next = std::next(itr, 1);
		if (k != partBdryTris.size())
		{
			if (
				itr->getSortedGlobal(0) == next->getSortedGlobal(0) &&

				itr->getSortedGlobal(1) == next->getSortedGlobal(1) &&
				itr->getSortedGlobal(2) == next->getSortedGlobal(2)
				)
			{

				emInt global[3] = {itr->getGlobalCorner(0),
								itr->getGlobalCorner(1), itr->getGlobalCorner(2)};
				emInt globalNext[3] = {next->getGlobalCorner(0),
									next->getGlobalCorner(1), next->getGlobalCorner(2)};

				TriFaceVerts tripart(numDivs, global, itr->getPartid(),
									next->getPartid(), true);

				TriFaceVerts tripartNext(numDivs, globalNext, next->getPartid(), itr->getPartid(), true);

				tris[itr->getPartid()].push_back(tripart);
				tris[next->getPartid()].push_back(tripartNext);
			}
		}
	}
}


inline
void preMatchingPartBdryQuads(const emInt numDivs,const setQuad& partBdryQuads, vecVecQuad &quads)
{
	std::size_t kquad = 0;
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

			emInt v0SortedGlobal = next->getSortedGlobal(0);
			emInt v1SortedGlobal = next->getSortedGlobal(1);
			emInt v2SortedGlobal = next->getSortedGlobal(2);
			emInt v3SortedGlobal = next->getSortedGlobal(3);

			emInt v0Global_ = itr->getGlobalCorner(0);
			emInt v1Global_ = itr->getGlobalCorner(1);
			emInt v2Global_ = itr->getGlobalCorner(2);
			emInt v3Global_ = itr->getGlobalCorner(3);

			emInt partid_ = itr->getPartid();

			emInt v0SortedGlobal_ = itr->getSortedGlobal(0);
			emInt v1SortedGlobal_ = itr->getSortedGlobal(1);
			emInt v2SortedGlobal_ = itr->getSortedGlobal(2);
			emInt v3SortedGlobal_ = itr->getSortedGlobal(3);

			if (v0SortedGlobal_ == v0SortedGlobal &&
				v1SortedGlobal == v1SortedGlobal_ &&
				v2SortedGlobal == v2SortedGlobal_ &&
				v3SortedGlobal == v3SortedGlobal_)
			{
				emInt global[4] = {v0Global, v1Global, v2Global, v3Global};
				emInt global_[4] = {v0Global_, v1Global_, v2Global_, v3Global_};

				QuadFaceVerts quadpart(numDivs, global, partid, partid_, true);
				QuadFaceVerts quadpart_(numDivs, global_, partid_, partid, true);
				quads[partid].push_back(quadpart);
				quads[partid_].push_back(quadpart_);
			}
		}
	}
}


inline
void TestPartFaceMatching( const std::size_t nPart, 
const vecHashTri  &HashTri, 
const vecHashQuad &HashQuad,
const vecVecTri   &vecTris, 
const vecVecQuad  &vecquads)
{
	assert(HashTri.size()==nPart);
	assert(vecTris.size()==nPart);
	assert(HashQuad.size()==nPart);
	assert(vecquads.size()==nPart);
	for(auto i=0; i<nPart; i++)
	{
		assert(vecTris[i].size()==HashTri[i].size()); 
		assert(vecquads[i].size()==HashQuad[i].size());
		for(auto itri: HashTri[i])
		{
			auto find= std::find(vecTris[i].begin(),vecTris[i].end(),itri);
			assert(find!=vecTris[i].end());
			
		}
		for(auto iquad: HashQuad[i])
		{
			auto find= std::find(vecquads[i].begin(),vecquads[i].end(),iquad);
			if(find==vecquads[i].end())
			{
				assert(find!=vecquads[i].end());
			}
		}

	}
	std::cout<<"Successful Testing Part Face Matching"<<std::endl;
};
struct timeResults
{
	double calculatedTotal; 
	double total; 
	double read; 
	double extract; 
	double refine;
	double sendtris;
	double sendquads;
	double recvtris;	
	double recvquads;
	double matchtris;
	double matchquads; 
	double syncTri; 
	double syncQuad;
	double serial; 
	double partfacematching; 
	double partition;
	double faceExchange; 
	// double calculatedTotal()
	// {
	// 	total=serial+extract+refine+sendtris+sendquads+recvtris
	// 	+recvquads+matchtris+matchquads+syncTri+syncQuad; 
	// 	return total; 
	// }

	void printTimeResults() 
	{
		std::cout << "Read: " << read << std::endl;
		std::cout << "Partition: " << partition << std::endl;
		std::cout << "Part face matching: " << partfacematching << std::endl;
		std::cout << "Serial: " << serial << std::endl;
		std::cout << "Extract: " << extract << std::endl;
		std::cout << "Refine: " << refine << std::endl;
		std::cout << "Send tris: " << sendtris << std::endl;		
		std::cout << "Send quads: " << sendquads << std::endl;
		std::cout << "Receive tris: " << recvtris << std::endl;
		std::cout << "Receive quads: " << recvquads << std::endl;
		std::cout << "Sync tris: " << syncTri << std::endl;
		std::cout << "Sync quads: " << syncQuad << std::endl;
		std::cout << "Match tris: " << matchtris << std::endl;
		std::cout << "Match quads: " << matchquads << std::endl;
		std::cout << "Total time: " << total << std::endl;
		
	}

};

#endif /* SRC_EXA_DEFS_H_ */
