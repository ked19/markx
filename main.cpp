#include "myVector.h"
#include "myAlgo.h"
#include "define.h"
#include <iostream>
using namespace std;

int a[] = {0, 3, 2, 43, 3, 31, 44};
int Cmp(const void *pA, const void *pB)
{
	int *pIA = (int *)pA;
    int *pIB = (int *)pB;
    if (*pIA < *pIB) {
        return -1;
    } else if (*pIA > *pIB) {
        return 1;
    } else {
        return 0;
    }
}

void Prt(const void *pA)
{
	int *pIA = (int *)pA;
	cout << *pIA << " " << flush;
}

int main(int argc, char **argv)
{
		
	//MyVector mv("expt.vec");
	//MyVector mv("cyl.vec"); 
	MyVector mv("helix.vec");
	mv.BuildNet();
	mv.Pagerank();
	//mv.HITS();
	mv.SavePagerank();
	//mv.WriteFlovis();

	system("pause");
	return 0;
	

	/*
	BinTree bT;
	bT.Clear(Cmp);

	list<int> v;
	v.clear();
	//int a[] = {0, 3, 2, 43, 3, 31, 44};
	for (unsigned i = 0; i < 7; i++) {
		v.push_back(a[i]);
		//cout << v[v.size() - 1] << " " << flush;
		void *pA = bT.Insert(&v.back());
		int *pIA = (int *)pA;
		cout << *pIA << " " << flush;	

			bT.SetPrint(Prt);
	bT.PrintAll();	
	}
	cout << endl;

	bT.SetPrint(Prt);
	bT.PrintAll();	
	*/
}