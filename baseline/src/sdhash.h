#ifndef SDHASH_H
#define SDHASH_H
#include <emp-tool/emp-tool.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"
#include <cmath>
#include "emp-sh2pc/emp-sh2pc.h"
#include "util.h"

using namespace NTL;
using namespace std;
using namespace emp;



// Shannon entropy (per-window) in bits/byte; normalized to [0,1000] by H/log2(l) with l=64
Integer normalized_entropy_0to1000(vector<Integer> input){
    vector<Integer> freq;
    for (size_t i=0; i<256; i++) {
      freq.push_back(Integer(BITLEN, 0, ALICE));
    }
    for (size_t i=0; i<input.size(); i++) {
      oblivious_write(freq, input[i]);
    }
    Integer H = Integer(BITLEN, 0, ALICE);
    for (size_t i=0; i<256; i++) {
      // H = If(freq[i] == Integer(BITLEN, 0, ALICE), H, H - (freq[i]/Integer(BITLEN, len, ALICE))*Integer(BITLEN, std::log2(freq[i]/Integer(BITLEN, len, ALICE)), ALICE));
      H = If(freq[i] == Integer(BITLEN, 0, ALICE), H, H - (freq[i]/Integer(BITLEN, input.size(), ALICE)));
    }
    Integer norm = H / Integer(BITLEN, 6, ALICE);
    return norm;
}


// Build digest from bytes
Integer leftmost(vector<Integer> input){    
    oblivious_sort(input, 64);
    return input[0];
}

void sha3(){

  uint8_t input[64];
  uint8_t output_bytes[11];
  uint8_t output_bytes2[11];
  for (size_t i = 0; i < 7; ++i) {
    input[i] = i % 7;
  }
  emp::sha3_256(output_bytes, input, 64);

  emp::Integer integers[64];
  for (int64_t i = 0; i < 64; ++i) {
    integers[i] = Integer(8, i % 200, BOB);
  }

  emp::Integer output = Integer(10, 11, BOB);

  SHA3_256_Calculator sha3_256_calculator = SHA3_256_Calculator();
  sha3_256_calculator.sha3_256(&output, integers, 64);

}

vector<Integer> sdhash_digest(vector<Integer> file){
  vector<Integer> ret;
  for (int i=0; i<file.size()-64; i++) {
    vector<Integer> input;
    for (int j=i; j<i+64; j++) {
      input.push_back(file[j]);
    }
    ret.push_back(normalized_entropy_0to1000(input));
  }

  Integer left = leftmost(ret);
  for (int i=0; i<file.size()-64; i++) {
    sha3();
  }

    
  return ret;
    
}


#endif