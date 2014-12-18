#include "vectorData.h"

VectorFile::VectorFile(string f)
    : m_vDim(0, 0, 0, 0), m_vStep(0, 0, 0), m_vDimStep(0, 0, 0)
    , m_pVdOrg(0), m_pVdScl(0)
{
    string line;
    stringstream ss;

    cout << "read .vec" << endl;
    ifstream inF(f.c_str());

    getline(inF, line);
    ss.clear();
    ss << line.substr( line.find(":") + 1 );
    ss >> m_vDim.m_r >> m_vDim.m_g >> m_vDim.m_b >> m_vDim.m_a;
    cout << "dimension:\t" << m_vDim.m_r << " " << m_vDim.m_g << " " << m_vDim.m_b << " " << m_vDim.m_a << endl;

    //******************************************

    getline(inF, line);
    ss.clear();
    ss << line.substr( line.find(":") + 1 );
    ss >> m_vStep.m_x >> m_vStep.m_y >> m_vStep.m_z;
    cout << "step:\t" << m_vStep.m_x << " " << m_vStep.m_y << " " << m_vStep.m_z << endl;

    //**********************************************

    getline(inF, line);
    ss.clear();
    ss << line.substr( line.find(":") + 1 );
    ss >> m_vDimStep.m_x >> m_vDimStep.m_y >> m_vDimStep.m_z;
    cout << "dimStep:\t" << m_vDimStep.m_x << " " << m_vDimStep.m_y << " " << m_vDimStep.m_z << endl;

    //*******************************************

    getline(inF, line);
    ss.clear();
    ss << line.substr( line.find(":") + 1 );
    ss >> m_format;
    cout << "format:\t" << m_format << endl;

    //**********************************************

    getline(inF, line);
    ss.clear();
    ss << line.substr( line.find(":") + 1 );
    ss >> m_unitSize;
    cout << "unitSize:\t" << m_unitSize << endl;

    //**********************************************

    getline(inF, line);
    ss.clear();
    ss << line.substr( line.find(":") + 1 );
    ss >> m_fTplate;
    cout << "template:\t" << m_fTplate << endl;
    inF.close();
    cout << "read .vec ok" << endl;

    //**********************************************

    LoadRecord();

    DATA maxLen = 0;
    for (unsigned z = 0; z < m_vDim.m_z; z++) {
        for (unsigned y = 0; y < m_vDim.m_y; y++) {
            for (unsigned x = 0; x < m_vDim.m_x; x++) {
                MyAssert(m_vDim.m_c <= 3);
                DATA aV[3];
                for (unsigned c = 0; c < m_vDim.m_c; c++) {
                    aV[c] = m_pVdOrg->CellVal(x, y, z, c);
                }
                DATA len = myMath.LenNV(aV, m_vDim.m_c);
                if (maxLen < len) {
                    maxLen = len;
                } else {}
            }
        }
    }
    for (unsigned z = 0; z < m_vDim.m_z; z++) {
        for (unsigned y = 0; y < m_vDim.m_y; y++) {
            for (unsigned x = 0; x < m_vDim.m_x; x++) {
                for (unsigned c = 0; c < m_vDim.m_c; c++) {
                    m_pVdOrg->CellRef(x, y, z, c) = 
                        m_pVdOrg->CellVal(x, y, z, c) / maxLen;
                }
            }
        }
    }    
}

VectorFile::~VectorFile()
{
    delete m_pVdOrg;
}

//*************************************************************************************************

void VectorFile::GetFType()
{
    size_t tLoc = m_fTplate.find_last_of(".");
    if (tLoc != m_fTplate.npos) {
        string t = m_fTplate.substr(tLoc + 1);
        if (t.compare("txt") == 0) {
            size_t tL2 = m_fTplate.find_last_of("_");
            m_fType = m_fTplate.substr(tL2 + 1, tLoc - tL2 - 1);
        } else if (t.compare("raw") == 0) {
            m_fType = t;
        } else {
            MyAssert(0);
        }
    } else {
        MyAssert(0);
    }
    cout << "fType: " << m_fType << endl;
}

unsigned VectorFile::GetUSize() const
{
    return m_unitSize;
}
Vect3D<unsigned> VectorFile::GetDimStep() const
{
    return m_vDimStep;
}

Vect4D<unsigned> VectorFile::GetOrgDim() const
{
    return m_vDim;
}
/*
Vect4D<unsigned> VectorFile::GetSclDim() const
{
    return m_pVdScl->GetDim();
}
*/

Vect3D<DATA> VectorFile::GetOrgStep() const
{
    return m_vStep;
}
/*
Vect3D<DATA> VectorFile::GetSclStep() const
{
    return Vect3D<DATA>(
        m_vStep.m_x*m_vDimStep.m_x,
        m_vStep.m_y*m_vDimStep.m_y,
        m_vStep.m_z*m_vDimStep.m_z);
}
*/

void VectorFile::LoadRecord()
{
    string line;
    stringstream ss;

    cout << "LoadRecord" << endl;
    delete m_pVdOrg;
    Layer3D l3dOrg(m_vDim.m_r, m_vDim.m_g, m_vDim.m_b, m_vDim.m_a);
    m_pVdOrg = new VectorData(l3dOrg, m_vStep);

    string sfName;
    ifstream fSlice;

    GetFType();

    for (unsigned count = 0; ; count++) {
        if (m_fType.compare("uv") == 0) {
            if (count == 0) {
                size_t pos = m_fTplate.find_last_of('_');
                sfName = m_fTplate.substr(0, pos + 1) + "u.txt";
                cout << "open " << sfName << endl;
                fSlice.open(sfName);
            } else if (count == 1) {
                size_t pos = sfName.find_last_of('u');
                sfName.replace(pos, 1, "v");
                fSlice.close();
                cout << "open " << sfName << endl;
                fSlice.open(sfName);
            } else if (count == 2) {
                fSlice.close();
                break;
            } else {
                assert(0);
            }
        } else if (m_fType.compare("raw") == 0) {
        	if (count == 0) {
	        	sfName = m_fTplate;
	        	cout << "open " << sfName << endl;
	        	fSlice.open(sfName, ios::binary);
	        } else if (count == 1) {
	        	fSlice.close();
	        	break;
	        } else {
	        	MyAssert(0);
	        }
        } else {
            assert(0);
        }

        if (m_fType.compare("uv") == 0) {
        	getline(fSlice, line);
        	getline(fSlice, line);
        } else {}

        for (unsigned z = 1; z <= m_vDim.m_b; z++) {
            if (m_vDim.m_b > 1) {
                PrintCount(z - 1, m_vDim.m_b - 1);
            } else {}

            for (unsigned y = 0; y < m_vDim.m_g; y++) {
                if (m_vDim.m_b == 1) {
                    PrintCount(y, m_vDim.m_g - 1);
                } else {}

                for (unsigned x = 0; x < m_vDim.m_r; x++) {
                    if (m_format.compare("float") == 0) {
                    	if (m_fType.compare("uv") == 0) {
	                        if (m_unitSize == 0) {
	                            DATA v;
	                            fSlice >> v;
	                            l3dOrg.CellRef(x, y, z - 1, count) = v;
	                        } else {
	                            assert(0);
                       		}
                       	} else if (m_fType.compare("raw") == 0) {
                       		if (m_unitSize == 4) {
                       			float aV[3];
								fSlice.read((char *)aV, sizeof(float) * m_vDim.m_a);
                       			l3dOrg.CellRef(x, y, z - 1, 0) = aV[0];
                       			l3dOrg.CellRef(x, y, z - 1, 1) = aV[1];
								if (m_vDim.m_a == 3) {
                       				l3dOrg.CellRef(x, y, z - 1, 2) = aV[2];
								} else {}
                       		} else {
                       			MyAssert(0);
                       		}
                       	} else {}
                    } else {
                        assert(0);
                    }
                } // x
            } // y
        } // z
    } // count

    cout << "LoadRecord ok" << endl;
}

const VectorData &VectorFile::GetOrgVecVal() const
{
    return *m_pVdOrg;
}