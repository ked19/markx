#include "myVector.h"

Vect3D<unsigned> resolute(5, 5, 5); //(20, 20, 20); //10, 10, 10);
unsigned INTERATE_NUM = 200; //100;
DATA INTERATE_LIM     = 1e-6;
DATA DAMP             = 0.85F;
//unsigned NET_PATH   = 20;
unsigned NET_STEP_MIN = 4; //1; // 10
unsigned NET_STEP_MAX = 30; //3; //30;
DATA NET_STEP_LEN     = 4.F; // 3.F; //0.2F;
bool B_NET_RND        = false; //false;
bool B_STEP_RND       = false; //true;
bool B_BACKWARD       = false; // false;
bool B_RNDINIT        = true; //true;
bool B_PR_8OUT        = false; //true;
bool B_PR_0OUT        = false;
bool B_VNORM          = false; //true;
unsigned PROP_WINDOW  = 2;
bool B_DUP_BOUND      = false; //false;
bool B_COUNT_REG      = true; //false;
Vect3D<unsigned> locWin(40, 40, 1);

VNode::VNode(Vect3D<unsigned> idx, unsigned count)
    : m_idx(idx.m_x, idx.m_y, idx.m_z), m_count(count)
{}

VNode::~VNode()
{}

MyVector::MyVector(string f)
    : m_vDim(0, 0, 0), m_vStep(0, 0, 0)
    , m_bMax(false, false, false)
    , m_pppIdx(0), m_pppOut(0), m_pppFor(0)
	, m_vIdxDim(0, 0, 0)
    , m_pLyrAuth(0), m_pLyrHubs(0)
{
    m_fName = f;
    m_fName = m_fName.substr(0, m_fName.find_last_of('.'));

    m_pVecF = new VectorFile(m_fName + ".vec");
    const VectorData &rVD = m_pVecF->GetOrgVecVal();
    m_vStep = rVD.GetStep();
    m_vDim = rVD.GetDim();

    if (m_vDim.m_z == 1) {
        resolute.m_z = 1;
    } else {}

    m_vIdxDim.m_x = m_vDim.m_x - 1;
    m_vIdxDim.m_y = m_vDim.m_y - 1;
    m_vIdxDim.m_z = (m_vDim.m_z >= 2) ? m_vDim.m_z - 1 : 1;
    m_pppIdx = new list<VNode> **[m_vIdxDim.m_z];
    m_pppOut = new list<VNode> **[m_vIdxDim.m_z];
	m_pppFor = new list<VNode> **[m_vIdxDim.m_z]; 
    for (unsigned z = 0; z < m_vIdxDim.m_z; z++) {
        m_pppIdx[z] = new list<VNode> *[m_vIdxDim.m_y];
        m_pppOut[z] = new list<VNode> *[m_vIdxDim.m_y];
		m_pppFor[z] = new list<VNode> *[m_vIdxDim.m_y];
        for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
            m_pppIdx[z][y] = new list<VNode>[m_vIdxDim.m_x];
            m_pppOut[z][y] = new list<VNode>[m_vIdxDim.m_x];
			m_pppFor[z][y] = new list<VNode>[m_vIdxDim.m_x];
            for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
                m_pppIdx[z][y][x].clear();
                m_pppOut[z][y][x].clear();
				m_pppFor[z][y][x].clear();
            }
        }
    }

    m_pLyrAuth = new Layer2D(m_vIdxDim.m_x, m_vIdxDim.m_y, m_vIdxDim.m_z);
    m_pLyrHubs = new Layer2D(m_vIdxDim.m_x, m_vIdxDim.m_y, m_vIdxDim.m_z);

    if (m_vDim.m_x > m_vDim.m_y) {
        if (m_vDim.m_z > m_vDim.m_x) {
            m_bMax.m_z = true;
        } else {
            m_bMax.m_x = true;
        }
    } else {
        if (m_vDim.m_z > m_vDim.m_y) {
            m_bMax.m_z = true;
        } else {
            m_bMax.m_y = true;
        }
    }
}

MyVector::~MyVector()
{
    for (unsigned z = 0; z < m_vIdxDim.m_z; z++) {
        for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
            for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
                m_pppIdx[z][y][x].clear();
                m_pppOut[z][y][x].clear();
            }
            delete []m_pppIdx[z][y];
            delete []m_pppOut[z][y];
        }
        delete []m_pppIdx[z];
        delete []m_pppOut[z];
    }
    delete []m_pppIdx;
    delete []m_pppOut;

    delete m_pLyrAuth;
    delete m_pLyrHubs;
}

//*************************************************************************************************

void MyVector::FindNode(Vect2D<int> &nIdx, Vect2D<DATA> &r, Vect2D<DATA> vLoc)
{
    Vect2D<DATA> idx(vLoc.m_x * m_vIdxDim.m_x, vLoc.m_y * m_vIdxDim.m_y);
    nIdx.m_x = (int)idx.m_x;
    nIdx.m_y = (int)idx.m_y;
    //cout << idx.m_x << " " << idx.m_y << " " << nIdx.m_x << " " << nIdx.m_y << endl;

    if (vLoc.m_x < 0) {
        nIdx.m_x--;
    } else {}
    if (vLoc.m_y < 0) {
        nIdx.m_y--;
    } else {}

    r.m_x = idx.m_x - nIdx.m_x;
    r.m_y = idx.m_y - nIdx.m_y;
}

void MyVector::FindNode(Vect3D<int> &nIdx, Vect3D<DATA> &r, Vect3D<DATA> vLoc)
{
     Vect3D<DATA> idx(vLoc.m_x * m_vIdxDim.m_x, 
                      vLoc.m_y * m_vIdxDim.m_y,
                      vLoc.m_z * m_vIdxDim.m_z);
    nIdx.m_x = (int)idx.m_x;
    nIdx.m_y = (int)idx.m_y;
    nIdx.m_z = (int)idx.m_z;
    //cout << idx.m_x << " " << idx.m_y << " " << nIdx.m_x << " " << nIdx.m_y << endl;

    if (vLoc.m_x < 0) {
        nIdx.m_x--;
    } else {}
    if (vLoc.m_y < 0) {
        nIdx.m_y--;
    } else {}
    if (vLoc.m_z < 0) {
        nIdx.m_z--;
    } else {}

    r.m_x = idx.m_x - nIdx.m_x;
    r.m_y = idx.m_y - nIdx.m_y;
    r.m_z = idx.m_z - nIdx.m_z;
}

void MyVector::AddNode(BinTree &tSort, Vect3D<unsigned> nowI, Vect3D<unsigned> nextI, list<VNode> ***pppI)
{
    if (nextI.m_x == nowI.m_x && 
        nextI.m_y == nowI.m_y && 
        nextI.m_z == nowI.m_z) {
        //return;
    } else {}

    //cout << nowI.m_x  << "," << nowI.m_y  << "," << nowI.m_z  << ":"
    //     << nextI.m_x << "," << nextI.m_y << "," << nextI.m_z << " ";
    /*
    list<VNode> &vVec = pppI[nowI.m_z][nowI.m_y][nowI.m_x];
    bool bExit = false;
    //for (unsigned i = 0; i < vVec.size(); i++) {
	for (list<VNode>::iterator it = vVec.begin();
		it != vVec.end();
		++it) {
        if (it->m_idx.m_x == nextI.m_x &&
            it->m_idx.m_y == nextI.m_y &&
            it->m_idx.m_z == nextI.m_z) {
            it->m_count++;
            bExit = true;
            break;
        } else {}
    }

    if (!bExit) {
        VNode node(nextI);
        vVec.push_back(node);
    } else {}
    */
	
    VNode nIn(nextI);
    list<VNode> &vVec = pppI[nowI.m_z][nowI.m_y][nowI.m_x];
    vVec.push_back(nIn);
    //VNode &nNew = vVec[vVec.size() - 1];
    VNode &nNew = vVec.back();
    void *pVoid = tSort.Insert((void *)&nNew);
    VNode *pVN = (VNode *)pVoid;
    if (pVN != &nNew) {
        vVec.pop_back();
        pVN->m_count++;
    } else {}
	
}

void MyVector::Pagerank()
{
    
    cout << "run Pagerank" << endl;
time_t tB = time(0);

    unsigned countAll;
    unsigned idxSize;
    if (m_vDim.m_z == 1) {
        countAll = (B_DUP_BOUND) ?
            (resolute.m_x + 2) * (resolute.m_y + 2) :
            (resolute.m_x)     * (resolute.m_y);
        idxSize = m_vIdxDim.m_x * m_vIdxDim.m_y;
    } else {
        countAll = (B_DUP_BOUND) ?
            (resolute.m_x + 2) * (resolute.m_y + 2) * (resolute.m_z + 2) :
            (resolute.m_x)     * (resolute.m_y)     * (resolute.m_z);
        idxSize = m_vIdxDim.m_x * m_vIdxDim.m_y * m_vIdxDim.m_z;
    }
    if (B_BACKWARD) {
        countAll *= 2;
    } else {}
    if (B_COUNT_REG) {
        countAll *= NET_STEP_MAX;
    } else {}

    Layer *pLyrOld = 0;
    Layer *pLyrNew = 0;
    for (unsigned h = 0; h < INTERATE_NUM; h++) {
        if (h == 0) {
            if (B_RNDINIT) {
                lyrOp.rnd.Gen(*m_pLyrAuth);
            } else {
                lyrOp.one.Gen(*m_pLyrAuth);
            }
            pLyrOld = m_pLyrAuth;
            pLyrNew = m_pLyrHubs;
        } else {
            Layer *pLyrTmp = pLyrOld;
            pLyrOld = pLyrNew;
            pLyrNew = pLyrTmp;
        }

        pLyrNew->CopyFrom(*pLyrOld);
        lyrOp.mul.Gen(*pLyrNew, 1.F - DAMP);

		
		DATA remain = 0;
		for (unsigned z = 0; z < m_vIdxDim.m_z; z++) {
            for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
                for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
                    list<VNode> &vFor = m_pppFor[z][y][x];
					list<VNode>::iterator it = vFor.begin();
					Vect3D<unsigned> idx = it->m_idx;
					MyAssert(idx.m_x == x &&
						     idx.m_y == y &&
							 idx.m_z == z);
					
					unsigned count = it->m_count;
					DATA rm = DAMP * pLyrOld->CellVal(x, y, z) 
                              * count / countAll / (idxSize - 1);
					remain += rm;
                    pLyrNew->CellRef(x, y, z) = pLyrNew->CellVal(x, y, z) - rm;
				} // x
			} // y
		} // z

		for (int z = 0; z < (int)m_vIdxDim.m_z; z++) {
            #pragma omp parallel for //num_threads(4)
            for (int y = 0; y < (int)m_vIdxDim.m_y; y++) {
                for (int x = 0; x < (int)m_vIdxDim.m_x; x++) {
                    list<VNode> &vFor = m_pppFor[z][y][x];
					list<VNode>::iterator it = vFor.begin();
					++it;
					for (; it != vFor.end(); ++it) {
						Vect3D<unsigned> idx = it->m_idx;
						//MyAssert(idx.m_x != x ||
						//		 idx.m_y != y ||
						//		 idx.m_z != z);
						unsigned count = it->m_count;
						                           
						pLyrNew->CellRef(x, y, z) =
							pLyrNew->CellVal(x, y, z) +
							DAMP * pLyrOld->CellVal(idx.m_x, idx.m_y, idx.m_z) * count / countAll;
					}
				} // x
			} // y
		} // z
		

		/*
        DATA remain = 0;
        for (unsigned z = 0; z < m_vIdxDim.m_z; z++) {
            for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
                for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {

                    list<VNode> &vIdx = m_pppIdx[z][y][x];
                    unsigned count = 0;
                    
                    //for (unsigned v = 0; v < vIdx.size(); v++) {
                    //    Vect3D<unsigned> idx = vIdx[v].m_idx;
                    //    unsigned cc = vIdx[v].m_count;
                    

                    for (list<VNode>::iterator it = vIdx.begin();
                         it != vIdx.end();
                         ++it) {
                        Vect3D<unsigned> idx = it->m_idx;
                        unsigned cc = it->m_count;

                        if (idx.m_x != x || idx.m_y != y || idx.m_z != z) {
                            pLyrNew->CellRef(idx.m_x, idx.m_y, idx.m_z) =
                                pLyrNew->CellVal(idx.m_x, idx.m_y, idx.m_z) +
                                DAMP * pLyrOld->CellVal(x, y, z) * cc / countAll;

                            count += cc;
                        } else {}
                    } // v

                    if (count < countAll) {
                        DATA rm = DAMP * pLyrOld->CellVal(x, y, z) 
                                  * (countAll - count) / countAll / (idxSize - 1);
                        pLyrNew->CellRef(x, y, z) = pLyrNew->CellVal(x, y, z) - rm;
                        remain += rm;
                    } else if (count > countAll) {
                        cout << "count > countAll..";
                        getchar();
                    }
                } // x
            } // y
        } // z
		*/
        //cout << remain << endl;
        lyrOp.add.Gen(*pLyrNew, remain);

        cout << h << " ";
        DATA aMin, aMax;
        lyrOp.rng.Gen(aMin, aMax, *pLyrNew);
        cout << "nRng: " << aMin << "-" << aMax << endl;
    } // h

    if (pLyrNew != m_pLyrAuth) {
        m_pLyrAuth->CopyFrom(*pLyrNew);
    } else {}

time_t tE = time(0);
cout << "time: " << tE - tB << endl;
    cout << "run Pagerank ok" << endl;
    
}

void MyVector::HITS()
{
    /*
    cout << "run HITS" << endl;

    lyrOp.one.Gen(*m_pLyrAuth);
    lyrOp.one.Gen(*m_pLyrHubs);

    for (unsigned h = 0; h < INTERATE_NUM; h++) {
        lyrOp.zero.Gen(*m_pLyrAuth);
        for (unsigned z = 0; z < m_vIdxDim.m_z; z++) {
            for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
                for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
                    list<VNode> &vIdx = m_pppIdx[z][y][x];
                    
                    for (list<VNode>::iterator it = vIdx.begin();
                         it != vIdx.end();
                         ++it) {
                        Vect3D<unsigned> idx = it->m_idx;
                        m_pLyrAuth->CellRef(idx.m_x, idx.m_y, idx.m_z) =
                            m_pLyrAuth->CellVal(idx.m_x, idx.m_y, idx.m_z) +
                            //m_pLyrHubs->CellVal(x, y, z);
                            it->m_count * m_pLyrHubs->CellVal(x, y, z);
                    }

                    list<VNode> &vOut = m_pppOut[z][y][x];
                    
                    for (list<VNode>::iterator it = vOut.begin();
                         it != vOut.end();
                         ++it) {
                        Vect3D<unsigned> idx = it->m_idx;
                        m_pLyrAuth->CellRef(idx.m_x, idx.m_y, idx.m_z) =
                            m_pLyrAuth->CellVal(idx.m_x, idx.m_y, idx.m_z) +
                            //m_pLyrHubs->CellVal(x, y, z);
                            it->m_count * m_pLyrHubs->CellVal(x, y, z);
                    }
                } // x
            } // y
        } // z
        lyrOp.norm.Gen(*m_pLyrAuth, true);

        lyrOp.zero.Gen(*m_pLyrHubs);
        for (unsigned z = 0; z < m_vIdxDim.m_z; z++) {
            for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
                for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {

                    list<VNode> &vIdx = m_pppIdx[z][y][x];
                    


                    list<VNode> &vOut = m_pppOut[z][y][x];
                    for (unsigned v = 0; v < vOut.size(); v++) {
                        Vect3D<unsigned> idx = vOut[v].m_idx;
                        m_pLyrHubs->CellRef(x, y, z) =
                            m_pLyrHubs->CellVal(x, y, z) -
                            //m_pLyrAuth->CellVal(idx.m_x, idx.m_y, idx.m_z);
                            vOut[v].m_count * m_pLyrAuth->CellVal(idx.m_x, idx.m_y, idx.m_z);
                    }
                } // x
            } // y
        } // z
        lyrOp.norm.Gen(*m_pLyrHubs, true);

        cout << h << " ";
        DATA aMin, aMax, hMin, hMax;
        lyrOp.rng.Gen(aMin, aMax, *m_pLyrAuth);
        lyrOp.rng.Gen(hMin, hMax, *m_pLyrHubs);
        cout << "aRng: " << aMin << "-" << aMax << "; "
             << "hRng: " << hMin << "-" << hMax << endl;
    } // h

    cout << "run HITS ok" << endl;
    */
}

int SCmp(const void *pA, const void *pB)
{
    VNode *pVA = (VNode *)pA;
    VNode *pVB = (VNode *)pB;
    if (pVA->m_idx.m_z < pVB->m_idx.m_z) {
        return -1;
    } else if (pVA->m_idx.m_z > pVB->m_idx.m_z) {
        return 1;
    } else if (pVA->m_idx.m_y < pVB->m_idx.m_y) {
        return -1;
    } else if (pVA->m_idx.m_y > pVB->m_idx.m_y) {
        return 1;
    } else if (pVA->m_idx.m_x < pVB->m_idx.m_x) {
        return -1;
    } else if (pVA->m_idx.m_x > pVB->m_idx.m_x) {
        return 1;
    } else {
        return 0;
    }
}

void MyVector::BuildNet()
{
time_t tB = time(0);
    cout << "BuildNet" << endl;

    unsigned jNum = (B_BACKWARD) ? 2 : 1;
    unsigned xxL = (B_DUP_BOUND) ? 0 : 1; unsigned xxR = (B_DUP_BOUND) ? resolute.m_x + 1 : resolute.m_x;
    unsigned yyB = (B_DUP_BOUND) ? 0 : 1; unsigned yyT = (B_DUP_BOUND) ? resolute.m_y + 1 : resolute.m_y;
    unsigned zzN = (B_DUP_BOUND) ? 0 : 1; unsigned zzF = (B_DUP_BOUND) ? resolute.m_z + 1 : resolute.m_z;
    if (m_vDim.m_z == 1) {
        zzN = 0; 
        zzF = 0;
    } else {}

    const VectorData &rVD = m_pVecF->GetOrgVecVal();
    for (int z = 0; z < (int)m_vIdxDim.m_z; z++) {
        if (m_vDim.m_z > 1) {
            PrintCount(z, m_vIdxDim.m_z - 1);
            cout << ":" << endl;
        } else {}

        #pragma omp parallel for //num_threads(4)
        for (int y = 0; y < (int)m_vIdxDim.m_y; y++) {
            //if (m_vDim.m_z == 1) {
                PrintCount(y, m_vIdxDim.m_y - 1);
            //} else {}

            for (int x = 0; x < (int)m_vIdxDim.m_x; x++) {
                //PrintCount(x, m_vIdxDim.m_x - 1);

                BinTree tSort;
                tSort.Clear(SCmp);
                for (unsigned zz = zzN; zz <= zzF; zz++) {
                    for (unsigned yy = yyB; yy <= yyT; yy++) {
                        for (unsigned xx = xxL; xx <= xxR; xx++) {
                            DATA fX, fY, fZ;
                            if (B_NET_RND) {
                                fX = myMath.Rnd();
                                fY = myMath.Rnd();
                                fZ = myMath.Rnd();
                            } else {
                                fX = (DATA)(xx + 0.1F * (myMath.Rnd() - 0.5F)) / (resolute.m_x + 1);
                                fY = (DATA)(yy + 0.1F * (myMath.Rnd() - 0.5F)) / (resolute.m_y + 1);
                                fZ = (DATA)(zz + 0.1f * (myMath.Rnd() - 0.5F)) / (resolute.m_z + 1);
                            }

                            DATA xLoc = (x + fX) / m_vIdxDim.m_x;
                            DATA yLoc = (y + fY) / m_vIdxDim.m_y;
                            DATA zLoc = (z + fZ) / m_vIdxDim.m_z;

                            Vect3D<unsigned> nowNode(x, y, z);
                            Vect3D<int>      nxtNode(0, 0, 0);

                            for (unsigned j = 0; j < jNum; j++) {
                                unsigned stepLen = NET_STEP_MAX - NET_STEP_MIN;
                                unsigned nStep = (B_STEP_RND) ?
                                                 (unsigned)(NET_STEP_MIN + myMath.Rnd() * stepLen) :
                                                 NET_STEP_MAX;

                                vector<Vect3D<unsigned>> addNextNI;
                                addNextNI.clear();

                                bool bOutBound = false;
                                bool bAdd = false;
                                for (unsigned i = 0; i < nStep; i++) {

                                Vect3D<DATA> preV(0, 0, 0);
                                Vect3D<DATA> preL(0, 0, 0);
                                for (unsigned r = 0; r < 4; r++) {
                                //for (unsigned r = 0; r == 0; r++) {
                                    DATA aaaaN[2][2][2][3];
                                    for (unsigned zzz = 0; zzz <= 1; zzz++) {
                                        if (m_vDim.m_z == 1) {
                                            if (zzz == 1) {
                                                break;
                                            } else {}
                                        } else {}

                                        for (unsigned yyy = 0; yyy <=1; yyy++) {
                                            for (unsigned xxx = 0; xxx <= 1; xxx++) {
                                                aaaaN[zzz][yyy][xxx][0] = rVD.CellVal(nowNode.m_x + xxx, nowNode.m_y + yyy, nowNode.m_z + zzz, 0);
                                                aaaaN[zzz][yyy][xxx][1] = rVD.CellVal(nowNode.m_x + xxx, nowNode.m_y + yyy, nowNode.m_z + zzz, 1);
                                                if (m_vDim.m_z > 1) {
                                                    aaaaN[zzz][yyy][xxx][2] = rVD.CellVal(nowNode.m_x + xxx, nowNode.m_y + yyy, nowNode.m_z + zzz, 2);
                                                } else {}
                                            }
                                        }
                                    }

                                    DATA xV, yV, zV;
                                    if (m_vDim.m_z == 1) {
                                        xV = myMath.Interpolate_linear(
                                            aaaaN[0][0][0][0], aaaaN[0][0][1][0],
                                            aaaaN[0][1][0][0], aaaaN[0][1][1][0],
                                            fX, fY);
                                        yV = myMath.Interpolate_linear(
                                            aaaaN[0][0][0][1], aaaaN[0][0][1][1],
                                            aaaaN[0][1][0][1], aaaaN[0][1][1][1],
                                            fX, fY);
                                        zV = 0;
                                    } else {
                                        xV = myMath.Interpolate_linear(
                                            aaaaN[0][0][0][0], aaaaN[0][0][1][0],
                                            aaaaN[0][1][0][0], aaaaN[0][1][1][0],
                                            aaaaN[1][0][0][0], aaaaN[1][0][1][0],
                                            aaaaN[1][1][0][0], aaaaN[1][1][1][0],
                                            fX, fY, fZ);
                                        yV = myMath.Interpolate_linear(
                                            aaaaN[0][0][0][1], aaaaN[0][0][1][1],
                                            aaaaN[0][1][0][1], aaaaN[0][1][1][1],
                                            aaaaN[1][0][0][1], aaaaN[1][0][1][1],
                                            aaaaN[1][1][0][1], aaaaN[1][1][1][1],
                                            fX, fY, fZ);
                                        zV = myMath.Interpolate_linear(
                                            aaaaN[0][0][0][2], aaaaN[0][0][1][2],
                                            aaaaN[0][1][0][2], aaaaN[0][1][1][2],
                                            aaaaN[1][0][0][2], aaaaN[1][0][1][2],
                                            aaaaN[1][1][0][2], aaaaN[1][1][1][2],
                                            fX, fY, fZ);
                                    }

                                    if (B_VNORM) {
                                        DATA aV[] = {xV, yV, zV};
                                        myMath.Normal3V(aV);
                                        //cout << aV[0] << "," << aV[1] << "," << aV[2] << endl;
                                        //getchar();
                                        xV = aV[0] * NET_STEP_LEN;
                                        yV = aV[1] * NET_STEP_LEN;
                                        zV = aV[2] * NET_STEP_LEN;
                                    } else {
                                        xV *= NET_STEP_LEN;
                                        yV *= NET_STEP_LEN;
                                        zV *= NET_STEP_LEN;
                                    }

                                    xV = xV / m_vIdxDim.m_x; 
                                    yV = yV / m_vIdxDim.m_y; 
                                    zV = zV / m_vIdxDim.m_z;

                                    if (j == 1) {
                                        xV = -xV;
                                        yV = -yV;
                                        zV = -zV;
                                    } else {}

                                    if (r == 0) {
                                        preV.m_x = xV;
                                        preV.m_y = yV;
                                        preV.m_z = zV;
                                        xV *= 0.5F;
                                        yV *= 0.5F;
                                        zV *= 0.5F;
                                        preL.m_x = xLoc;
                                        preL.m_y = yLoc;
                                        preL.m_z = zLoc;
                                    } else if (r == 1) {
                                        preV.m_x += 2.F * xV;
                                        preV.m_y += 2.F * yV;
                                        preV.m_z += 2.F * zV;
                                        xV *= 0.5F;
                                        yV *= 0.5F;
                                        zV *= 0.5F;
                                        xLoc = preL.m_x;
                                        yLoc = preL.m_y;
                                        zLoc = preL.m_z;
                                    } else if (r == 2) {
                                        preV.m_x += 2.F * xV;
                                        preV.m_y += 2.F * yV;
                                        preV.m_z += 2.F * zV;
                                        xLoc = preL.m_x;
                                        yLoc = preL.m_y;
                                        zLoc = preL.m_z;
                                    } else if (r == 3) {
                                        preV.m_x += xV;
                                        preV.m_y += yV;
                                        preV.m_z += zV;
                                        xV = preV.m_x / 6.F;
                                        yV = preV.m_y / 6.F;
                                        zV = preV.m_z / 6.F;
                                        xLoc = preL.m_x;
                                        yLoc = preL.m_y;
                                        zLoc = preL.m_z;
                                    } else {
                                        MyAssert(0);
                                    }
                                    Vect3D<DATA> locNew(xLoc + xV, yLoc + yV, zLoc + zV);

                                    Vect3D<DATA> fNxt(0, 0, 0);
                                    FindNode(nxtNode, fNxt, locNew);
                                    if (nxtNode.m_x < 0 || nxtNode.m_x >= (int)m_vIdxDim.m_x ||
                                        nxtNode.m_y < 0 || nxtNode.m_y >= (int)m_vIdxDim.m_y ||
                                        nxtNode.m_z < 0 || nxtNode.m_z >= (int)m_vIdxDim.m_z) {
                                        bOutBound = true;
                                        break;
                                    } else {}

                                    Vect3D<unsigned> nowNI(x, y, z);
                                    Vect3D<unsigned> nextNI((unsigned)nxtNode.m_x,
                                                            (unsigned)nxtNode.m_y,
                                                            (unsigned)nxtNode.m_z);
                                    if (r == 3) {
                                        addNextNI.push_back(nextNI);
                                    } else {}

                                    if (B_COUNT_REG ||
                                        i == nStep - 1) {
                                    //if (i == nStep - 1) {
                                    if (r == 3) {
                                        unsigned xDiff = abs(nxtNode.m_x - (int)x);
                                        unsigned yDiff = abs(nxtNode.m_y - (int)y);
                                        unsigned zDiff = abs(nxtNode.m_z - (int)z);
                                        //if (xDiff > 1 || yDiff > 1 || zDiff > 1) {
                                        if (bAdd ||
                                            xDiff >= 1 || 
                                            yDiff >= 1 || 
                                            zDiff >= 1) {
                                            if (j == 0) {
                                                //AddNode(tSort, nowNI, nextNI, m_pppIdx);
                                                //cout << addNextNI.size() << " " << endl;
                                                for (unsigned v = 0; v < addNextNI.size(); v++) {
                                                    AddNode(tSort, nowNI, addNextNI[v], m_pppIdx);
                                                }
                                                addNextNI.clear();
                                                bAdd = true;
                                            } else if (j == 1) {
                                                //AddNode(tSort, nowNI, nextNI, m_pppOut);
                                                for (unsigned v = 0; v < addNextNI.size(); v++) {
                                                    AddNode(tSort, nowNI, addNextNI[v], m_pppOut);
                                                }
                                                addNextNI.clear();
                                                bAdd = true;
                                            } else {
                                                MyAssert(0);
                                            }
                                            //if (!B_COUNT_REG) {
                                            //    break;
                                            //} else {}
                                        } else {}
                                    } else {}
                                    } else {}

                                    xLoc = locNew.m_x;
                                    yLoc = locNew.m_y;
                                    zLoc = locNew.m_z;

                                    fX = fNxt.m_x;
                                    fY = fNxt.m_y;
                                    fZ = fNxt.m_z;
                                    //if (fX < 0 || fY < 0 || fZ < 0) {
                                    //    cout << fX << "," << fY << "," << fZ << ":" 
                                    //         << locNew.m_x << "," << locNew.m_y << "," << locNew.m_z;
                                    //    getchar();
                                    //} else {}
                                    MyAssert(fX >= 0 &&
                                             fY >= 0 &&
                                             fZ >= 0);

                                    nowNode.m_x = nxtNode.m_x;
                                    nowNode.m_y = nxtNode.m_y;
                                    nowNode.m_z = nxtNode.m_z;
                                } // r

                                if (bOutBound) {
                                    break;
                                } else {}
                                } // i
                            } // j    
                        } // xx
                    } // yy
                } // zz

                if (B_PR_8OUT) {
                    if (m_pppIdx[0][y][x].size() > 8) {
                        cout << endl << x << " " << y << ":" << endl;
                        list<VNode> &vIdx = m_pppIdx[0][y][x];
                        /*
                        for (unsigned i = 0; i < vIdx.size(); i++) {
                            cout << vIdx[i].m_idx.m_x << ","
                                 << vIdx[i].m_idx.m_y << " ";
                        }
                        */
                        for (list<VNode>::iterator it = vIdx.begin();
                             it != vIdx.end();
                             ++it) {
                            cout << it->m_idx.m_x << ","
                                 << it->m_idx.m_y << " ";
                        }
                        cout << endl;
                    } else {}
                } else {}

                if (B_PR_0OUT) {
                    if (m_pppIdx[0][y][x].size() == 0) {
                        cout << x << "," << y << " ";
                    } else {}
                } else {}
            } // x;
        } // y
    } // z

	unsigned countAll;
    DATA nLen = NET_STEP_LEN * NET_STEP_MAX;
    unsigned neiLen = (unsigned)nLen;
    if (neiLen != nLen) {
        neiLen++;
    } else {}
    unsigned neiSize = neiLen * neiLen;
    cout << "neighbor size: " << neiSize << endl;

    if (m_vDim.m_z > 1) {
        neiSize *= neiLen;
    } else {}
    if (m_vDim.m_z == 1) {
        countAll = (B_DUP_BOUND) ?
            (resolute.m_x + 2) * (resolute.m_y + 2) :
            (resolute.m_x)     * (resolute.m_y);
        //idxSize = m_vIdxDim.m_x * m_vIdxDim.m_y;
    } else {
        countAll = (B_DUP_BOUND) ?
            (resolute.m_x + 2) * (resolute.m_y + 2) * (resolute.m_z + 2) :
            (resolute.m_x)     * (resolute.m_y)     * (resolute.m_z);
        //idxSize = m_vIdxDim.m_x * m_vIdxDim.m_y * m_vIdxDim.m_z;
    }
    if (B_BACKWARD) {
        countAll *= 2;
    } else {}
    if (B_COUNT_REG) {
        countAll *= NET_STEP_MAX;
    } else {}

	for (unsigned z = 0; z < m_vIdxDim.m_z; z++) {
		PrintCount(z, m_vIdxDim.m_z - 1);
		for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
			for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
				unsigned countNow = 0;
				list<VNode> &vVec = m_pppIdx[z][y][x];
				MyAssert(vVec.size() <= neiSize);

				while(!vVec.empty()) {
					list<VNode>::iterator it = vVec.begin();
					Vect3D<unsigned> idx = it->m_idx;
					unsigned count = it->m_count;
					countNow += count;
					list<VNode> &vFor = m_pppFor[idx.m_z][idx.m_y][idx.m_x];
					
					VNode node(Vect3D<unsigned>(x, y, z), count);
					vFor.push_back(node);
					vVec.pop_front();
				} // it

				list<VNode> &vFor = m_pppFor[z][y][x];
				VNode node(Vect3D<unsigned>(x, y, z), countAll - countNow);
				vFor.push_front(node);
                MyAssert(vFor.size() <= neiSize);
			} // x
		} // y
	} // z

    cout << "BuildNet ok" << endl;
time_t tE = time(0);
cout << "time: " << tE - tB << endl;
}

int sFunc(const void *p1, const void *p2)
{
    if (*(float *)p1 < * (float *)p2) {
        return -1;
    }
    else if (*(float *)p1 > *(float *)p2) {
        return 1;
    } else {
        return 0;
    }
}
void MyVector::SavePagerank() const
{
    /*
    unsigned wSize = locWin.m_x * locWin.m_y;
    float *pW = new float[wSize];
    Mtx mtxW(m_vIdxDim.m_x, m_vIdxDim.m_y);
    for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
		cout << y << " ";
		if (y == m_vIdxDim.m_y - 1) {
			cout << endl;
		} else {}

        int yB = y - locWin.m_y / 2;
        unsigned yT = yB + locWin.m_y - 1;
        if (yB < 0) {
            yB = 0;
        } else {}
        if (yT >= m_vIdxDim.m_y) {
            yT = m_vIdxDim.m_y - 1;
        } else {}

        for(unsigned x = 0; x < m_vIdxDim.m_x; x++) {
            int xL = x - locWin.m_x / 2;
            unsigned xR = xL + locWin.m_x - 1;
            if(xL < 0) {
                xL = 0;
            } else {}
            if (xR >= (int)m_vIdxDim.m_y) {
                xR = m_vIdxDim.m_y - 1;
            } else {}

            unsigned wI = 0;
            float wV = 0;
            for (unsigned yy = (unsigned)yB; yy <= yT; yy++) {
                for (unsigned xx = (unsigned)xL; xx <= xR; xx++) {
                    pW[wI] = (float)m_pLyrAuth->CellVal(xx, yy, 0);
                    if (yy == y &&
                        xx == x) {
                        wV = pW[wI];
                    } else {}
                    wI++;
                }
            }
            qsort((void *)pW, wI, sizeof(float), sFunc);

			for (unsigned i = 0; i < wI; i++) {
				if (pW[i] == wV) {
					mtxW.CellRef(x, y) = i;
					break;
				} else {}
			}
        } // x
    } // y
    mtxOp.mul.Gen(mtxW, 255.F / (wSize - 1));
    imgIO.Write("pr_loc.bmp", MyImg(mtxW));
    */

/*
    unsigned sSize = m_vIdxDim.m_z * m_vIdxDim.m_y * m_vIdxDim.m_x;
    float *pS = new float[sSize];
    m_pLyrAuth->CopyTo_zLast(pS);
    qsort((void *)pS, sSize, sizeof(float), sFunc);
    
    Layer lyrS(m_vIdxDim.m_x, m_vIdxDim.m_y, m_vIdxDim.m_z);
    for (unsigned z = 0; z< m_vIdxDim.m_z; z++) {
        if (m_vIdxDim.m_z > 1) {
            PrintCount(z, m_vDim.m_z - 1);
        } else {}

        for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
            if (m_vIdxDim.m_z == 1) {
                PrintCount(y, m_vDim.m_y - 1);
            } else {}

            for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
                lyrS.CellRef(x, y, z) = sSize - 1;
                for (unsigned i = 1; i <= sSize - 1; i++) {
                    if (myMath.IsGreat(pS[i], m_pLyrAuth->CellVal(x, y, z))) {
                        lyrS.CellRef(x, y, z) = i - 1;
                        break;
                    } else {}
                } // i
            } // x
        } // y
    } // z
    DATA mx, mn;
    lyrOp.rng.Gen(mn, mx, lyrS);
    cout << "rng: " << mn << "-" << mx << endl;

    lyrOp.mul.Gen(lyrS, 255.F / (sSize - 1));
    lyrOp.saveImg.Gen(lyrS, m_fName + "_scl_", ".bmp");
*/
    /*
    Mtx mtxS(m_vIdxDim.m_x, m_vIdxDim.m_y);
    for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
        cout << y << " ";
        if (y == m_vIdxDim.m_y - 1) {
            cout << endl;
        } else {}

        for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
            mtxS.CellRef(x, y) = sSize - 1;
            for (unsigned i = 1; i <= sSize - 1; i++) {
                if (myMath.IsGreat(pS[i], m_pLyrAuth->CellVal(x, y, 0))) {
                    mtxS.CellRef(x, y) = i - 1;
                    break;
                } else {}
            }
        }
    }
    mtxOp.mul.Gen(mtxS, 255.F / (sSize - 1));
    imgIO.Write("pr_scl.bmp", MyImg(mtxS));
    */

    Layer lyrO(m_vIdxDim.m_x, m_vIdxDim.m_y, m_vIdxDim.m_z);
    lyrO.CopyFrom(*m_pLyrAuth);
    DATA vMin, vMax;
    lyrOp.rng.Gen(vMin, vMax, lyrO);
    cout << "rng: " << vMin << "-" << vMax << endl;
    lyrOp.sub.Gen(lyrO, vMin);
    lyrOp.mul.Gen(lyrO, 255.F / (vMax - vMin) * 10.F);
	 for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
       for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
			if (lyrO.CellVal(x, y, 0) > 255.F) {
				lyrO.CellRef(x, y, 0) = 255.F;
			} else {}
		}
	  }
    lyrOp.saveImg.Gen(lyrO, m_fName + "_org_", ".bmp");
    lyrOp.rng.Gen(vMin, vMax, lyrO);
    cout << "rng: " << vMin << "-" << vMax << endl;

    /*
    Mtx mtxO(m_vIdxDim.m_x, m_vIdxDim.m_y);
    DATA vMin, vMax;
    lyrOp.rng.Gen(vMin, vMax, *m_pLyrAuth);
    for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
        for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
            mtxO.CellRef(x, y) = (m_pLyrAuth->CellVal(x, y, 0) - vMin) / (vMax - vMin) * 255.F;
        }
    }
    imgIO.Write("pr_org.bmp", MyImg(mtxO));    

    Layer lyrTmp(m_vIdxDim.m_x, m_vIdxDim.m_y, m_vIdxDim.m_z);
    for (unsigned z = 0; z < m_vIdxDim.m_z; z++) {
        for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
            for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
                //lyrTmp.CellRef(x, y, z) = log(m_pLyrAuth->CellVal(x, y, z) + 1.F);
                //lyrTmp.CellRef(x, y, z) = pow((DATA)m_pLyrAuth->CellVal(x, y, z), (DATA)0.3F);
                lyrTmp.CellRef(x, y, z) = mtxS.CellVal(x, y);
            }
        }
    }
    */

//    lyrOp.save.Gen(lyrS, m_fName + "_scl.lyr");
    lyrOp.save.Gen(lyrO, m_fName + "_org.lyr");
}

void MyVector::SaveHITS() const
{
    Layer lyrTmp(m_vIdxDim.m_x, m_vIdxDim.m_y, m_vIdxDim.m_z);
    for (unsigned z = 0; z < m_vIdxDim.m_z; z++) {
        for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
            for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
                DATA val = m_pLyrAuth->CellVal(x, y, z) - m_pLyrHubs->CellVal(x, y, z);
                lyrTmp.CellRef(x, y, z) = fabs(val);

                //m_pLyrAuth->CellRef(x, y, z) = pow(m_pLyrAuth->CellVal(x, y, z), 0.3);
                //m_pLyrHubs->CellRef(x, y, z) = pow(m_pLyrHubs->CellVal(x, y, z), 0.3);
                //lyrTmp.CellRef(x, y, z) = pow(lyrTmp.CellVal(x, y, z), 0.3);

                //m_pLyrAuth->CellRef(x, y, z) = log(m_pLyrAuth->CellVal(x, y, z) + 1.F);
                //m_pLyrHubs->CellRef(x, y, z) = log(m_pLyrHubs->CellVal(x, y, z) + 1.F);
                //lyrTmp.CellRef(x, y, z) = log(lyrTmp.CellVal(x, y, z) + 1.F);
            }
        }
    }
    //lyrOp.save.Gen(*m_pLyrAuth, m_fName+"_auth.lyr");
    //lyrOp.save.Gen(*m_pLyrHubs, m_fName+"_hubs.lyr");
    //lyrOp.save.Gen(lyrTmp, m_fName+"_diff.lyr");

    unsigned sSize = m_vIdxDim.m_z * m_vIdxDim.m_y * m_vIdxDim.m_x;
    float *pS = new float[sSize];

    m_pLyrAuth->CopyTo_zLast(pS);
    qsort((void *)pS, sSize, sizeof(float), sFunc);
    Mtx mtxS(m_vIdxDim.m_x, m_vIdxDim.m_y);

    for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
        cout << y << " ";
        if (y == m_vIdxDim.m_y - 1) {
            cout << endl;
        } else {}

        for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
            mtxS.CellRef(x, y) = sSize - 1;
            for (unsigned i = 1; i <= sSize - 1; i++) {
                if (myMath.IsGreat(pS[i], m_pLyrAuth->CellVal(x, y, 0))) {
                    mtxS.CellRef(x, y) = i - 1;
                    break;
                } else {}
            }
        }
    }
    mtxOp.mul.Gen(mtxS, 255.F / (sSize - 1));
    imgIO.Write("auth_scl.bmp", MyImg(mtxS));

    DATA vMin, vMax;
    lyrOp.rng.Gen(vMin, vMax, *m_pLyrAuth);
    for (unsigned y = 0; y < m_vIdxDim.m_y; y++) {
        for (unsigned x = 0; x < m_vIdxDim.m_x; x++) {
            mtxS.CellRef(x, y) = (m_pLyrAuth->CellVal(x, y, 0) - vMin) / (vMax - vMin) * 255.F;
        }
    }
    imgIO.Write("auth_org.bmp", MyImg(mtxS));

    /*
    m_pLyrHubs->CopyTo_zLast(pS);
    qsort((void*)pS, sSize, sizeof(float), sFunc);

    for(unsigned y=0; y<m_vIdxDim.m_y; y++)
    {
        cout << y << " ";
        if(y==m_vIdxDim.m_y-1)
        {
            cout << endl;
        }
        else {}

        for(unsigned x=0; x<m_vIdxDim.m_x; x++)
        {
            mtxS.CellRef(x, y) = sSize-1;
            for(unsigned i=1; i<=sSize-1; i++)
            {
                if(myMath.IsGreat(pS[i], m_pLyrHubs->CellVal(x, y, 0)))
                {
                    mtxS.CellRef(x, y) = i-1;
                    break;
                }
                else {}
            }
        }
    }
    mtxOp.mul.Gen(mtxS, 255.F/(sSize-1));
    imgIO.Write("Hubs.bmp", MyImg(mtxS));

    lyrTmp.CopyTo_zLast(pS);
    qsort((void*)pS, sSize, sizeof(float), sFunc);

    for(unsigned y=0; y<m_vIdxDim.m_y; y++)
    {
        cout << y << " ";
        if(y==m_vIdxDim.m_y-1)
        {
            cout << endl;
        }
        else {}

        for(unsigned x=0; x<m_vIdxDim.m_x; x++)
        {
            mtxS.CellRef(x, y) = sSize-1;
            for(unsigned i=1; i<=sSize-1; i++)
            {
                if(myMath.IsGreat(pS[i], lyrTmp.CellVal(x, y, 0)))
                {
                    mtxS.CellRef(x, y) = i-1;
                    break;
                }
                else {}
            }
        }
    }
    mtxOp.mul.Gen(mtxS, 255.F/(sSize-1));
    imgIO.Write("diff.bmp", MyImg(mtxS));
    */
}

void MyVector::WriteVsfplot() const
{
    const VectorData &vv = m_pVecF->GetOrgVecVal();
    ofstream outF("out.txt");
    for (unsigned y = 0; y < m_vDim.m_y; y++) {
        for (unsigned x = 0; x < m_vDim.m_x; x++) {
            outF << x << " "
                 << y << " "
                 << vv.CellVal(x, y, 0, 0) << " "
                 << vv.CellVal(x, y, 0, 1) << endl;
        }
    }
    outF.close();
}

void MyVector::WriteFlovis() const
{
	const unsigned ALEN = 40;

	const VectorData &vv = m_pVecF->GetOrgVecVal();
    ofstream outG("out.gri", ios::binary);
    ofstream outV("out.dat", ios::binary);

    int dX = (int)m_vDim.m_x;
    int dY = (int)m_vDim.m_y;
    int dZ = (int)m_vDim.m_z;
    int f  = 0;
    int t  = 0;
    float w = 0;
	stringstream ss; 
	ss.clear();
	ss << "SN4DB" << " "
	   << dX << " "
	   << dY << " "
       << dZ << " "
	   << f << " "
	   << t << " "
	   << w << " "; 
    char aArr[ALEN] = {0};
	ss.getline(aArr, ALEN);
	cout << aArr << endl; 
    outG.write(aArr, ALEN);

    for(unsigned z = 0; z < m_vDim.m_z; z++) {
	    for (unsigned y = 0; y < m_vDim.m_y; y++) {
	        for (unsigned x = 0; x < m_vDim.m_x; x++) {
	        	float xx = (float)x * 0.02f ;
	        	float yy = (float)y * 0.02f;
	        	float zz = (float)z; 

	        	outG.write((char *)&xx, sizeof(float));
	        	outG.write((char *)&yy, sizeof(float));
	        	outG.write((char *)&zz, sizeof(float));

	        	//float aA[] = {0.1, 0.1, 0};
				float aA[] = {(float)vv.CellVal(x, y, 0, 0),
							  (float)vv.CellVal(x, y, 0, 1),
							  0};
	        	outV.write((char *)&aA[0] , sizeof(float));
	        	outV.write((char *)&aA[1] , sizeof(float));
	        	outV.write((char *)&aA[2] , sizeof(float));
	        }
	    }
    }
    outG.close();
    outV.close();
}