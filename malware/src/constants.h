#ifndef CONSTANTS_H
#define CONSTANTS_H
#include "NTL/ZZ.h"



using namespace NTL;


#define THREADS 2
int secParam = 100;
int n = 4*secParam;
int p = 2* secParam + 1;
int l = n - p;
int RSMaxLength = l/4;
int k = (l-1)/2 + 1;
int alphabet_size = 256;
typedef unsigned long ulong;

//const NTL::ZZ prime = NTL::ZZ(power(NTL::ZZ(2), 107) - 1);	

ZZ prime = NextPrime(power(ZZ(2), 127), 10);

#endif