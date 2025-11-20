#ifndef AES_H
#define AES_H
#include "NTL/ZZ.h"
#include "constants.h"
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"
#include "emp-tool/emp-tool.h"


using namespace NTL;
using namespace std;
using namespace emp;


# define SHA256_DIGEST_LENGTH    32


bool not_in(int val, vector<int> vec) {
  // input: an integer value and an integer vector
  // output: whether the vector does not contain the value
  for (int i=0; i<vec.size(); i++) {
    if (val == vec[i])
      return false;
  }
  return true;
}


Integer set_diff(vec_ZZ_p A, vec_ZZ_p B, int num_fragments) {

  vector<Integer> SIM_A, SIM_B;
  for (size_t i=0; i<num_fragments; i++) {
    SIM_A.push_back(Integer(32, conv<int>(A[i]), ALICE));
    SIM_B.push_back(Integer(32, conv<int>(B[i]), BOB));
  }

  // calculate set difference
  int counter = 0;
  vector<int> temp;
  int A_length = SIM_A.size();
  int B_length = SIM_B.size();
  for (int i=0; i<A_length; i++) {
    for (int j=0; j<B_length; j++) {
      Bit res = A[i] == B[j];
      if ((res.reveal<bool>()) && (not_in(j,temp))) {
        counter += 1;
        temp.push_back(j);
        break;
      }
    }
  }
  Integer ret = Integer(32, A_length + B_length - 2*counter, PUBLIC);
  // cout << ret << "\n";
  return ret;
}


Integer AES(vector<Integer> LSH_input, __m128i key, __m128i iv) {
  int size = LSH_input.size();
  uint8_t input[size];

  for (size_t i=0; i<size; i++) {
    input[i] = LSH_input[i].reveal<int32_t>(ALICE);
  }

  emp::AES_128_CTR_Calculator aes_128_ctr_calculator = emp::AES_128_CTR_Calculator();
  emp::Integer input_integer = emp::Integer(size * 8, input, emp::ALICE);
  emp::Integer output_integer = emp::Integer(size * 8, input, emp::PUBLIC);
  emp::Integer iv_integer = emp::Integer(128, &iv, emp::BOB);
  emp::Integer key_integer = emp::Integer(128, &key, emp::BOB);

  aes_128_ctr_calculator.aes_128_ctr(&(key_integer[0].bit),
                                     &(iv_integer[0].bit),
                                     &(input_integer[0].bit),
                                     &(output_integer[0].bit),
                                     size * 8,
                                     emp::PUBLIC,
                                     77777);

  return output_integer;
}




#endif