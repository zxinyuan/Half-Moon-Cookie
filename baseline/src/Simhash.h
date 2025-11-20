#include <emp-sh2pc/emp-sh2pc.h>
#include <iostream>
#include <vector>
#include <random>
#include <cstdint>
#include <cassert>

using namespace emp;
using namespace std;

// --- Utility: sign-extend an Integer to a wider bit-length ---
static inline Integer sign_extend(const Integer &x, int new_len) {
    assert(new_len >= x.size());
    if (new_len == x.size()) return x;
    // Integer::resize(len, signed) exists in EMP; if not, emulate via manual extension
#ifdef EMP_WITH_RESIZE
    return Integer(x).resize(new_len, /*signed*/true);
#else
    // Manual sign extension: replicate sign bit into higher positions
    vector<Bit> bits = x.bits;
    Bit sign = bits.back();
    bits.reserve(new_len);
    for (int i = x.size(); i < new_len; ++i) bits.push_back(sign);
    return Integer(bits);
#endif
}

// Secure SimHash core. Inputs:
//  - hashes: N integers (32-bit each) representing feature hash values
//  - weights: N signed integers (wbits) representing weights (can be negative or positive)
//  - k: output fingerprint bits (default 64)
//  - wbits: bit-length of weights (default 16)
// Returns: k-bit Integer where bit i is 1 iff the signed sum for position i is >= 0
Integer simhash_gc(const vector<Integer>& hashes, const vector<Integer>& weights, int k = 64, int wbits = 16) {
    const size_t N = hashes.size();
    assert(N == weights.size());
    const int hbits = 32; // each feature hash is 32-bit

    // Choose safe accumulator width: 64 bits by default
    const int acc_bits = 64;

    vector<Bit> out_bits;
    out_bits.reserve(k);

    Integer zero(acc_bits, 0, PUBLIC);

    for (int i = 0; i < k; ++i) {
        Integer sum(acc_bits, 0, PUBLIC);
        for (size_t j = 0; j < N; ++j) {
            // Get bit i of hash_j; if i>=32, treat as 0 (you can also wrap i%32)
            Bit cond = (i < hbits) ? hashes[j][i] : Bit(false, PUBLIC);

            // Sign-extend weight into accumulator width
            Integer wext = sign_extend(weights[j], acc_bits);
            Integer neg_w = Integer(acc_bits, 0, PUBLIC) - wext; // two's complement negation

            // If cond==1 => +w, else => -w
            // EMP provides If(cond, a, b) that returns (cond ? a : b)
            Integer term = If(cond, wext, neg_w);
            sum = sum + term;
        }
        // Output bit is 1 if sum >= 0. For two's complement, sign bit==0 means non-negative.
        Bit nonneg = !sum[acc_bits - 1];
        out_bits.push_back(nonneg);
    }

    // Build k-bit Integer from bits (LSB-first ordering in EMP)
    return Integer(out_bits);
}

// Optional: cleartext reference for verification outside the circuit
static uint64_t simhash_clear(const vector<uint32_t>& hashes, const vector<int32_t>& weights, int k = 64) {
    const int hbits = 32;
    vector<long long> S(k, 0);
    for (size_t j = 0; j < hashes.size(); ++j) {
        uint32_t h = hashes[j];
        long long w = weights[j];
        for (int i = 0; i < k; ++i) {
            bool bit = (i < hbits) ? ((h >> i) & 1u) : false;
            S[i] += bit ? w : -w;
        }
    }
    uint64_t out = 0;
    for (int i = 0; i < k; ++i) if (S[i] >= 0) out |= (1ull << i);
    return out;
}

Integer Simhash(vector<Integer> hashes) {

    int N = 256;
    int k = 64;
    int wbits = 16;

    // Generate or load inputs. For demo: ALICE samples random hashes/weights; BOB inputs nothing.
    vector<uint32_t> h_clear(N);
    vector<int32_t> w_clear(N);

    std::mt19937_64 rng(12345);
    std::uniform_int_distribution<uint32_t> uh(0, 0xFFFFFFFFu);
    int32_t wmin = -(1 << (wbits - 1));
    int32_t wmax =  (1 << (wbits - 1)) - 1;
    std::uniform_int_distribution<int32_t> uw(wmin, wmax);
    for (int i = 0; i < N; ++i) {
        h_clear[i] = uh(rng);
        w_clear[i] = uw(rng);
    }

    vector<Integer> weights; weights.reserve(hashes.size());

    for (int i = 0; i < hashes.size(); ++i) {
        weights.emplace_back(wbits, (int64_t)w_clear[i], ALICE); // signed
    }

    // Run secure SimHash
    Integer sim_gc = simhash_gc(hashes, weights, k, wbits);

    return sim_gc;
}
