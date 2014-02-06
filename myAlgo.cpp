#include "myAlgo.h"

BTNode::BTNode(void *pVal)
    : m_pVal(pVal), m_pS(0), m_pL(0)
{}

BTNode::~BTNode()
{
    delete m_pS;
    delete m_pL;
    m_pVal = 0;
}

BinTree::BinTree()
    : m_pN(0)
    , m_cmpFuc(0)
    , m_prtFuc(0)
{}

BinTree::~BinTree()
{
	m_cmpFuc = 0;
    delete m_pN;
}

void BinTree::Clear(int (*compar)(const void *, const void *))
{
    m_cmpFuc = compar;
    delete m_pN;
}

/*
void *BinTree::Find(void *pVal)
{
	MyAssert(!m_cmpFuc);

	BTNode **ppNode = &m_pN;
	while (1) {
		if (!*ppNode) {
			return 0;
		} else {
			int cVal = m_cmpFuc(pVal, (*ppNode)->m_pVal);
			if (cVal == 0) {
				return (*ppNode)->m_pVal;
			} else if (cVal < 0) {	
				ppNode = &((*ppNode)->m_pS);
			} else {
				ppNode = &((*ppNode)->m_pL);
			}
		}
	}
	return 0;
}
*/

void *BinTree::Insert(void *pVal)
{
	MyAssert(m_cmpFuc != 0);

	BTNode **ppNode = &m_pN;
	while (1) {
		if (!*ppNode) {
			*ppNode = new BTNode(pVal);
			return (*ppNode)->m_pVal;
		} else {
			int cVal = m_cmpFuc(pVal, (*ppNode)->m_pVal);
			if (cVal == 0) {
				return (*ppNode)->m_pVal;
			} else if (cVal < 0) {	
				ppNode = &((*ppNode)->m_pS);
			} else {
				ppNode = &((*ppNode)->m_pL);
			}
		}
	}
	return 0;
}

void BinTree::SetPrint(void (*prt)(const void *))
{
	m_prtFuc = prt;
}

void BinTree::PrintOne(BTNode *pN)
{
	if (pN) {
		PrintOne(pN->m_pS);
		m_prtFuc(pN->m_pVal);
		PrintOne(pN->m_pL);
	} else {}
}
void BinTree::PrintAll()
{
	MyAssert(m_prtFuc != 0);
	PrintOne(m_pN);
	cout << endl;
}