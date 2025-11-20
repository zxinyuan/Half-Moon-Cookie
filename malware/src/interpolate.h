#include "emp-tool/emp-tool.h"
#include <iostream>
#include <typeinfo>
#include <vector>
#include <new>
#include <time.h>
#include <fstream>
#include <smmintrin.h>
#include <bitset>
using namespace std;
using namespace emp;

# define SHA256_DIGEST_LENGTH    32
# define bitsize    16
# define maxLen    32


void mul128_0(__m128i a, __m128i b, __m128i *res1, __m128i *res2) {
	__m128i tmp3, tmp4, tmp5, tmp6;
	tmp3 = _mm_clmulepi64_si128(a, b, 0x00);
	tmp4 = _mm_clmulepi64_si128(a, b, 0x10);
	tmp5 = _mm_clmulepi64_si128(a, b, 0x01);
	tmp6 = _mm_clmulepi64_si128(a, b, 0x11);

	tmp4 = _mm_xor_si128(tmp4, tmp5);
	tmp5 = _mm_slli_si128(tmp4, 8);
	tmp4 = _mm_srli_si128(tmp4, 8);
	tmp3 = _mm_xor_si128(tmp3, tmp5);
	tmp6 = _mm_xor_si128(tmp6, tmp4);
	// initial mul now in tmp3, tmp6
	*res1 = tmp3;
	*res2 = tmp6;
}


void mul128_1(__m128i a, __m128i b, __m128i *res1, __m128i *res2) {
	__m128i tmp3, tmp4, tmp5, tmp6;
	tmp3 = _mm_clmulepi64_si128(a, b, 0x00);
	tmp6 = _mm_clmulepi64_si128(a, b, 0x11);
	tmp4 = _mm_shuffle_epi32(a,78);
	tmp5 = _mm_shuffle_epi32(b,78);
	tmp4 = _mm_xor_si128(tmp4, a);
	tmp5 = _mm_xor_si128(tmp5, b);
	tmp4 = _mm_clmulepi64_si128(tmp4, tmp5, 0x00);
	tmp4 = _mm_xor_si128(tmp4, tmp3);
	tmp4 = _mm_xor_si128(tmp4, tmp6);
	tmp5 = _mm_slli_si128(tmp4, 8);
	tmp4 = _mm_srli_si128(tmp4, 8);
	*res1 = _mm_xor_si128(tmp3, tmp5);
	*res2 = _mm_xor_si128(tmp6, tmp4);
}


__m128i reflect_xmm(__m128i X) {
	__m128i tmp1,tmp2;
	__m128i AND_MASK =
		_mm_set_epi32(0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f);
	__m128i LOWER_MASK =
		_mm_set_epi32(0x0f070b03, 0x0d050901, 0x0e060a02, 0x0c040800);
	__m128i HIGHER_MASK =
		_mm_set_epi32(0xf070b030, 0xd0509010, 0xe060a020, 0xc0408000);
	__m128i BSWAP_MASK = _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
	tmp2 = _mm_srli_epi16(X, 4);
	tmp1 = _mm_and_si128(X, AND_MASK);
	tmp2 = _mm_and_si128(tmp2, AND_MASK);
	tmp1 = _mm_shuffle_epi8(HIGHER_MASK ,tmp1); tmp2 = _mm_shuffle_epi8(LOWER_MASK ,tmp2); tmp1 = _mm_xor_si128(tmp1, tmp2);
	return _mm_shuffle_epi8(tmp1, BSWAP_MASK);
}


void gfmul_0(__m128i a, __m128i b, __m128i *res){
	__m128i tmp3, tmp4, tmp5, tmp6,
			  tmp7, tmp8, tmp9, tmp10, tmp11, tmp12;
	__m128i XMMMASK = _mm_setr_epi32(0xffffffff, 0x0, 0x0, 0x0);
	tmp3 = _mm_clmulepi64_si128(a, b, 0x00);
	tmp6 = _mm_clmulepi64_si128(a, b, 0x11);
	tmp4 = _mm_shuffle_epi32(a,78);
	tmp5 = _mm_shuffle_epi32(b,78);
	tmp4 = _mm_xor_si128(tmp4, a);
	tmp5 = _mm_xor_si128(tmp5, b);
	tmp4 = _mm_clmulepi64_si128(tmp4, tmp5, 0x00);
	tmp4 = _mm_xor_si128(tmp4, tmp3);
	tmp4 = _mm_xor_si128(tmp4, tmp6);
	tmp5 = _mm_slli_si128(tmp4, 8);
	tmp4 = _mm_srli_si128(tmp4, 8);
	tmp3 = _mm_xor_si128(tmp3, tmp5);
	tmp6 = _mm_xor_si128(tmp6, tmp4);
	tmp7 = _mm_srli_epi32(tmp6, 31);
	tmp8 = _mm_srli_epi32(tmp6, 30);
	tmp9 = _mm_srli_epi32(tmp6, 25);
	tmp7 = _mm_xor_si128(tmp7, tmp8);
	tmp7 = _mm_xor_si128(tmp7, tmp9);
	tmp8 = _mm_shuffle_epi32(tmp7, 147);

	tmp7 = _mm_and_si128(XMMMASK, tmp8);
	tmp8 = _mm_andnot_si128(XMMMASK, tmp8);
	tmp3 = _mm_xor_si128(tmp3, tmp8);
	tmp6 = _mm_xor_si128(tmp6, tmp7);
	tmp10 = _mm_slli_epi32(tmp6, 1);
	tmp3 = _mm_xor_si128(tmp3, tmp10);
	tmp11 = _mm_slli_epi32(tmp6, 2);
	tmp3 = _mm_xor_si128(tmp3, tmp11);
	tmp12 = _mm_slli_epi32(tmp6, 7);
	tmp3 = _mm_xor_si128(tmp3, tmp12);

	*res = _mm_xor_si128(tmp3, tmp6);
}


void gfmul_1(__m128i a, __m128i b, __m128i *res){
	__m128i tmp3, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11, tmp12;
	__m128i XMMMASK = _mm_setr_epi32(0xffffffff, 0x0, 0x0, 0x0);
	mul128_0(a, b, &tmp3, &tmp6);
	tmp7 = _mm_srli_epi32(tmp6, 31);
	tmp8 = _mm_srli_epi32(tmp6, 30);
	tmp9 = _mm_srli_epi32(tmp6, 25);
	tmp7 = _mm_xor_si128(tmp7, tmp8);
	tmp7 = _mm_xor_si128(tmp7, tmp9);
	tmp8 = _mm_shuffle_epi32(tmp7, 147);

	tmp7 = _mm_and_si128(XMMMASK, tmp8);
	tmp8 = _mm_andnot_si128(XMMMASK, tmp8);
	tmp3 = _mm_xor_si128(tmp3, tmp8);
	tmp6 = _mm_xor_si128(tmp6, tmp7);
	tmp10 = _mm_slli_epi32(tmp6, 1);
	tmp3 = _mm_xor_si128(tmp3, tmp10);
	tmp11 = _mm_slli_epi32(tmp6, 2);
	tmp3 = _mm_xor_si128(tmp3, tmp11);
	tmp12 = _mm_slli_epi32(tmp6, 7);
	tmp3 = _mm_xor_si128(tmp3, tmp12);

	*res = _mm_xor_si128(tmp3, tmp6);
}


vector<Bit> zero_bits_128() {
  vector<Bit> result;
  for (int b=0; b<128; b++) {
    result.push_back(Bit(0, ALICE));
  }
  return result;
}


vector<Bit> one_8() {
  vector<Bit> result;
  for (int b=0; b<7; b++) {
    result.push_back(Bit(0, ALICE));
  }
  result.push_back(Bit(1, ALICE));
  return result;
}


uint64_t bit_to_int(vector<Bit> bit_array) {
  uint64_t ret = 0;
  int i = 0;
  if (bit_array.size() > 64) {
    i = 64;
  }
  while (i<bit_array.size()) {
    bool bit = bit_array[i].reveal<bool>();
    if (bit) {
      ret |= bit << (63-i);
    }
    i++;
  }
  return ret;
}


vector<vector<Bit>> Px(vector<vector<Bit>> ESP) {
  vector<vector<Bit>> ret;
  for (int i=0; i<maxLen; i++) {
    ret.push_back(zero_bits_128());
  }
  int z_length = ESP[0].size();
  for (int b=0; b<z_length; b++) {
    ret[0][128-z_length+b] = ESP[0][b];
  }
  ret[1][127] = Bit(1, ALICE);

  uint64_t esp, curr;
  for (int i=1; i<ESP.size(); i++) {
    esp = bit_to_int(ESP[i]);
    if (esp != 0) {
      vector<vector<Bit>> temp;
      //Bit **temp = new Bit*[maxLen];
      for (int j=0; j<maxLen; j++) {
        curr = bit_to_int(ret[j]);
        if (curr != 0) {
          block res[4];
          block a = makeBlock(0, curr);
          block b = makeBlock(0, esp);
          gfmul_0(a,b,res);
          gfmul_1(a,b,res+1);
          if(cmpBlock(res, res+1, 1) == false) {
            cout << "incorrect multiplication" << endl;
            abort();
          }
          block final = _mm_xor_si128(a, res[0]);
          //cout << final;
          //cout << "\n";
          int* data = (int*)&final;
          vector<Bit> result;
          for (int d=0; d<2; d++) {
            for (int b=63; b>=0; b--) {
              result.push_back(Bit((data[d] >> b) & 1, ALICE));
            }
          }
          temp.push_back(result);
        }
        else {
          temp.push_back(zero_bits_128());
        }
      }

      for (int j=ret.size()-1; j>0; j--) {
        ret[j] = ret[j-1];
      }

      ret[0] = zero_bits_128();

      for (int i=0; i<ret.size(); i++) {
        for (int j=0; j<128; j++) {
          ret[i][j] ^= temp[i][j];
        }
      }
    } 
  } 
  return ret;
}


vector<vector<Bit>> Gx(vector<vector<Bit>> Px, vector<vector<Bit>> Rcx, vector<vector<Bit>> Wx) {
  vector<vector<Bit>> temp;
  for (int i=0; i<2*maxLen; i++) {
    temp.push_back(zero_bits_128());
  }

  uint64_t curr_rcx;
  uint64_t curr_px;
  for (int i=0; i<maxLen; i++) {
    curr_rcx = bit_to_int(Rcx[i]);
    if (curr_rcx == 0) {
      continue;
    }
    for (int j=0; j<maxLen; j++) {
      curr_px = bit_to_int(Px[j]);
      if (curr_px == 0) {
        continue;
      }
      block res[4];
      block a = makeBlock(0, curr_rcx);
      block b = makeBlock(0, curr_px);
      gfmul_0(a,b,res);
      gfmul_1(a,b,res+1);
      block final = _mm_xor_si128(b, res[0]);
      int* data = (int*)&final;
      vector<Bit> result;
      for (int d=0; d<2; d++) {
        for (int b=63; b>=0; b--) {
          result.push_back(Bit((data[d] >> b) & 1, ALICE));
        }
      }
      for (int k=0; k<128; k++) {
        temp[i+j][k] ^= result[k];
      }
    }
  }

  for (int k=0; k<2*maxLen; k++) {
    for (int b=0; b<128; b++) {
      temp[k][b] ^= Wx[k%8][b];
    }
    // for (int i=0; i<128; i++) {
    //   cout << temp[k][i].reveal(PUBLIC);
    // }
    // cout << "\n";
  }
  return temp;
}


vector<vector<Bit>> rand_poly(int length) {
  vector<vector<Bit>> ret;
  
  PRG prg;
  bool rand_block[8];
  
  for (int i=0; i<length; i++) {
    prg.random_bool(rand_block, 8);
    vector<Bit> res;
    for (int j=0; j<8; j++) {
      res.push_back(Bit(rand_block[j], BOB));
    }
    ret.push_back(res);
  }
  return ret;
}


vector<vector<Bit>> f(vector<vector<Bit>> S_w) {
  vector<vector<Bit>> ret;
  vector<vector<Bit>> X = rand_poly(maxLen);
  assert(X.size()==S_w.size());
  for (int i=0; i<X.size(); i++) {
    vector<Bit> x_k = X[i];
    vector<Bit> e_w = one_8();
    for (int j=0; j<S_w.size(); j++) {
      vector<Bit> temp;
      for (int k=0; k<S_w[j].size(); k++) {
        temp.push_back(x_k[k]^S_w[j][k]);
      }
      uint64_t curr_ew = bit_to_int(e_w);
      uint64_t curr_temp = bit_to_int(temp);
      block res[4];
      block a = makeBlock(0, curr_ew);
      block b = makeBlock(0, curr_temp);
      gfmul_0(a,b,res);
      gfmul_1(a,b,res+1);
      block final = _mm_xor_si128(b, res[0]);
      int* data = (int*)&final;
      vector<Bit> result;
      for (int d=0; d<2; d++) {
        for (int b=63; b>=0; b--) {
          result.push_back(Bit((data[d] >> b) & 1, ALICE));
        }
      }
      e_w = result;
    }
    ret.push_back(e_w);
  }
  return ret;
}