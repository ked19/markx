#ifndef _MY_ALGO_H
#define _MY_ALGO_H

#include "define.h"
#include <iostream>
using namespace std;

class BTNode
{
public:
    BTNode(void *pVal);
    ~BTNode();

    void *m_pVal;
    BTNode *m_pS;
    BTNode *m_pL;
};

class BinTree
{
public:
    BinTree();
    ~BinTree();

    void Clear(int (*compar)(const void *, const void *));
    //void *Find(void *pVal);
    void *Insert(void *pVal);

    void SetPrint(void (*prt)(const void *));
    void PrintAll();

private:
	void PrintOne(BTNode *pN);

    BTNode *m_pN;
    int (*m_cmpFuc)(const void *, const void *);
    void (*m_prtFuc)(const void *);
};


#endif