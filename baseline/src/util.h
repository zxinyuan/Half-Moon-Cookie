#ifndef UTILS_H
#define UTILS_H
#include <emp-tool/emp-tool.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"
#include <cmath>


using namespace NTL;
using namespace std;
using namespace emp;

const int BITLEN = 32;
const int BUCKETS = 256;
const int DIGEST_SIZE = 128;

Integer mod(const Integer& a, int m) {
  Integer div = a / Integer(BITLEN, m, ALICE);        // q = a / m
  Integer rem = a - div * Integer(BITLEN, m, ALICE);  // r = a - q*m
  return rem;  // r = a % m
}

vector<Integer> intvec_to_emp(vector<int> vec) {
  vector<Integer> ret;
  for (size_t i=0; i<vec.size(); i++) {
    ret.push_back(Integer(BITLEN, vec[i], ALICE));
  }
  return ret;
}

Integer xor_integer(const Integer& a, const Integer& b) {
  assert(a.size() == b.size());
  Integer result(a.size(), 0, PUBLIC);
  for (int i = 0; i < a.size(); ++i) {
      result[i] = a[i] ^ b[i];
  }
  return result;
}

Integer oblivious_read(vector<Integer> vec, const Integer& index) {
  Integer result(BITLEN, 0, PUBLIC);
  for (size_t i = 0; i < vec.size(); ++i) {
      Bit is_selected = index == Integer(BITLEN, i, ALICE);
      result = If(is_selected, vec[i], result);
  }
  return result;
}

// Direction constants
const bool ASCENDING = true;
const bool DESCENDING = false;

// Oblivious compare-and-swap
void compare_and_swap(Integer &a, Integer &b, bool dir) {
    Bit cond = (a > b);
    if (!dir) cond = !cond;
    Integer min_val = If(cond, b, a);
    Integer max_val = If(cond, a, b);
    a = min_val;
    b = max_val;
}

// Oblivious bitonic merge
void bitonic_merge(vector<Integer> arr, int low, int count, bool dir) {
    if (count <= 1) return;
    int k = count / 2;
    for (int i = low; i < low + k; ++i) {
        compare_and_swap(arr[i], arr[i + k], dir);
    }
    bitonic_merge(arr, low, k, dir);
    bitonic_merge(arr, low + k, k, dir);
}

// Bitonic sort (recursive)
void bitonic_sort(vector<Integer> arr, int low, int count, bool dir) {
    if (count <= 1) return;
    int k = count / 2;
    bitonic_sort(arr, low, k, ASCENDING);
    bitonic_sort(arr, low + k, k, DESCENDING);
    bitonic_merge(arr, low, count, dir);
}

// Sort wrapper (must be power of 2)
void oblivious_sort(vector<Integer> arr, int size) {
    if ((size & (size - 1)) != 0) {
        std::cerr << "Error: bitonic sort requires power-of-2 size." << std::endl;
        return;
    }
    bitonic_sort(arr, 0, size, ASCENDING);
}

Integer oblivious_read_v_table(const Integer& index, const Integer data[BUCKETS]) {
  Integer result(BITLEN, 0, ALICE);
  for (size_t i = 0; i < BUCKETS; ++i) {
      Bit is_selected = index == Integer(BITLEN, i, ALICE);
      result = If(is_selected, data[i], result);
  }
  return result;
}

void oblivious_write(vector<Integer> vec, const Integer& idx) {
  for (int i = 0; i < vec.size(); ++i) {
      Bit match = (idx == Integer(BITLEN, i, PUBLIC));
      vec[i] = If(match, vec[i]+Integer(BITLEN, 1, ALICE), vec[i]);
  }
}

#endif