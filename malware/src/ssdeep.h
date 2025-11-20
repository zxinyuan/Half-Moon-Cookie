#ifndef SSDEEP_H
#define SSDEEP_H
#include <emp-tool/emp-tool.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"
#include <cmath>
#include "util.h"


using namespace NTL;
using namespace std;
using namespace emp;


const int ROLLING_WINDOW = 7;
const int BASE = 257;
const int MOD = 1 << 23;
// const int BITLEN = 32;


Integer rolling_hash(vector<Integer> window) {
    Integer hash = Integer(BITLEN, 0, ALICE);
    for (size_t i=0; i<window.size(); i++) {
        hash = (hash * Integer(BITLEN, BASE, ALICE) + window[i]);
        hash = mod(hash, MOD);
    }
    return hash;
}

std::string base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static vector<Integer> base64_table_emp;

void init_base64_table() {
    for (char c: base64_table) {
        base64_table_emp.push_back(Integer(BITLEN, static_cast<unsigned char>(c), ALICE));
    }
}

Integer hash_byte_to_base64(Integer h) {
    return oblivious_read(base64_table_emp, mod(h, 64));
}

void hash_in_circuit(){

  uint8_t input[7];
  uint8_t output_bytes[32];
  uint8_t output_bytes2[32];
  for (size_t i = 0; i < 7; ++i) {
    input[i] = i % 7;
  }
  emp::sha3_256(output_bytes, input, 7);

  emp::Integer integers[7];
  for (int64_t i = 0; i < 7; ++i) {
    integers[i] = Integer(8, i % 200, emp::PUBLIC);
  }

  emp::Integer output = Integer(10, 32, emp::PUBLIC);

  SHA3_256_Calculator sha3_256_calculator = SHA3_256_Calculator();
  sha3_256_calculator.sha3_256(&output, integers, 7);

}

Integer hash_chunk(vector<Integer> chunk) {
    hash_in_circuit();
    Integer hash = Integer(BITLEN, 0, ALICE);
    for (size_t i=0; i<chunk.size(); i++) {
        hash = ((hash << 5) + hash) ^ chunk[i];
    }
    return hash_byte_to_base64(hash);
}

vector<Integer> ssdeep_simulated(vector<Integer> input, int block_size = 64) {
    vector<Integer> rolling;
    vector<Integer> chunk;
    vector<Integer> result;

    for (size_t i = 0; i < input.size(); i++) {
        rolling.push_back(input[i]);
        chunk.push_back(input[i]);
        if (rolling.size() > ROLLING_WINDOW) rolling.erase(rolling.begin());

        Bit cond1(rolling.size() == ROLLING_WINDOW, ALICE);
        Bit cond2 = mod(rolling_hash(rolling), block_size) == Integer(BITLEN, block_size - 1, ALICE);
        Bit cond = cond1&cond2;

        result.push_back(If(cond, hash_chunk(chunk), Integer(BITLEN, 0, ALICE)));

    }

    if (!chunk.empty()) {
        result.push_back(hash_chunk(chunk)); // last chunk
    }

    return result;
}

#endif