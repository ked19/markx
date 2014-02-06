#ifndef _VECTOR_DATA_H
#define _VECTOR_DATA_H

#include "define.h"
#include "layer3D.h"
#include "myMath.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>

using namespace std;

class VectorFile
{
public:
	VectorFile(string f);
	~VectorFile();

	//const VolumeData& GetSclVolVal() const;
	const VectorData& GetOrgVecVal() const;
	
	unsigned GetUSize() const;
	Vect3D<unsigned> GetDimStep() const;

	Vect4D<unsigned> GetOrgDim() const;
	//Vect4D<unsigned> GetSclDim() const;
	Vect3D<DATA> GetOrgStep() const;
	//Vect3D<DATA> GetSclStep() const;

private:
	void LoadRecord();

	//void GenSclVolume();
	void GetFType();

	Vect4D<unsigned> m_vDim;
	Vect3D<DATA> m_vStep;
	Vect3D<unsigned> m_vDimStep;

	string m_format;
	unsigned m_unitSize;
	string m_fTplate;
	string m_fType;

	VectorData *m_pVdOrg;
	VectorData *m_pVdScl;
};

#endif