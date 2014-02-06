#ifndef _MY_VECTOR_H
#define _MY_VECTOR_H

#include "define.h"
#include "vectorData.h"
#include "myMath.h"
#include "layer.h"
#include "layerOperation.h"
#include "myAlgo.h"
#include <iostream>
#include <sstream>
#include <omp.h>

using namespace std;

class VNode
{
public:
    VNode(Vect3D<unsigned> idx, unsigned count = 1);
    ~VNode();

    Vect3D<unsigned> m_idx;
    unsigned m_count;
};

class MyVector
{
public:
    MyVector(string f);
    ~MyVector();

    void BuildNet();

    void HITS();
    void SaveHITS() const;

    void Pagerank();
    void SavePagerank() const;

    void WriteVsfplot() const;
    void WriteFlovis() const;

private:
    void FindNode(Vect2D<int> &nIdx, Vect2D<DATA> &r, Vect2D<DATA> vLoc);
    void FindNode(Vect3D<int> &nIdx, Vect3D<DATA> &r, Vect3D<DATA> vLoc);

    void AddNode(BinTree &tSort, Vect3D<unsigned> nowI, Vect3D<unsigned> nextI, list<VNode> ***pppI);

    Vect3D<unsigned> m_vDim;
    Vect3D<DATA> m_vStep;

    Vect3D<bool> m_bMax;

    list<VNode> ***m_pppIdx;
    list<VNode> ***m_pppOut;
	list<VNode> ***m_pppFor;
    Vect3D<unsigned> m_vIdxDim;

    Layer2D *m_pLyrAuth;
    Layer2D *m_pLyrHubs;

    string m_fName;
    VectorFile *m_pVecF;
};

#endif