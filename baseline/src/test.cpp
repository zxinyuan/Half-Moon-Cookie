#include <emp-tool/emp-tool.h>
#include <emp-ot/ot.h>
#include "AES.h"
#include "TLSH.h"
#include "ssdeep.h"
#include "sdhash.h"
#include "Simhash.h"
#include <emp-sh2pc/emp-sh2pc.h>
#include "NTL/ZZ_pXFactoring.h"
#include "NTL/ZZ_p.h"
#include "NTL/vec_ZZ_p.h"
#include "NTL/ZZ.h"
#include<chrono>
#include "NTL/matrix.h"
#include "NTL/vec_vec_ZZ_p.h"
#include "NTL/BasicThreadPool.h"
#include<vector>
#include<numeric>
#include<algorithm>
#include <thread>
#include<future>
#include<iostream>
#include <cstdlib>
#include<string>
#include <fstream>
#if defined(__linux__)
        #include <sys/time.h>
        #include <sys/resource.h>
#elif defined(__APPLE__)
        #include <unistd.h>
        #include <sys/resource.h>
        #include <mach/mach.h>
#endif

using namespace std;
using namespace emp;
using namespace chrono;
using namespace NTL;

#define TEST_TLSH 1
#define TEST_SSDEEP 0
#define TEST_SDHASH 0

int party, port;
NetIO * netio;

void setup() {
	// usleep(100);
	netio =  new emp::NetIO(party == emp::ALICE ? nullptr : "127.0.0.1", port, true);
	emp::setup_semi_honest(netio, party, 1024);
}

void done() {
	delete netio;
	finalize_semi_honest();
}


void ham(int n) {
	Integer a(n, 0, ALICE);
	Integer b(n, 0, BOB);
	Integer c = a^b;
	Integer d = c.hamming_weight();
	d.reveal<string>();
}


int main(int argc, char** argv) {
  std::ifstream lfile("../files/1.txt");
  if(!lfile) // test the file open.
  {
    std::cout<<"Error opening output file"<< std::endl;
    system("pause");
    return -1;
  }

  std::vector<int> ascii_encodings;
  char ch;
  while (lfile.get(ch)) {
    ascii_encodings.push_back(static_cast<unsigned char>(ch));  // ensure positive values
  }
  std::vector<int> ascii_files;
  for (int i=0; i<1; i++) {
    for (int j=0; j<ascii_encodings.size(); j++) {
      ascii_files.push_back(ascii_encodings[j]);
    }
  }

  if (TEST_TLSH) {
    auto start_time = high_resolution_clock::now();

    parse_party_and_port(argv, &party, &port);
    
    setup();

    vector<Integer> file = intvec_to_emp(ascii_files);
    init_v_table();

    vector<Integer> histogram;

    for (size_t i=0; i<BUCKETS; i++) {
      histogram.push_back(Integer(BITLEN, 0, ALICE));
    }

    for (size_t i=5; i < file.size(); i++) {
      vector<Integer> window(file.begin()+(i-5), file.begin()+i);
      histogram = sliding_window_hash(window, histogram);
    }
    vector<Integer> quartiles = get_quartiles(histogram);
    vector<Integer> header = digest_header(file, quartiles);
    vector<Integer> body = digest_body(histogram, quartiles);

    int blocklist_size = 100;

    for (int i=0; i<blocklist_size*(header.size()+body.size()); i++) {
      ham(1);
    }

    __m128i key;
    __m128i iv;
    for (size_t i = 0; i < 16; ++i) {
      ((uint8_t *)(&key))[i] = (1337 * i) % 255;
      ((uint8_t *)(&iv))[i] = (31 * i) % 253;
    }
    Integer AES_ret = AES(body, key, iv);

    if(party == ALICE) 
    {
      std::cout << "Client EmbedMap OT Cost: " << netio->counter << "bytes" << endl;
    }

    if(party == BOB) 
    {
      std::cout << "Server EmbedMap OT Cost: " << netio->counter << "bytes" << endl;
    }

    std::cout << "Number of AND gates used EmbedMap: "
    << CircuitExecution::circ_exec->num_and() << std::endl;

    done();
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    if(party == ALICE)
    {
      std::cout << "Client EmbedMap time: " << duration.count() << "ms" << endl; 
    }
    if(party == BOB)
    {
      std::cout << "Server EmbedMap time: " << duration.count() << "ms" << endl; 
    }
  }

  if (TEST_SSDEEP) {
    auto start_time = high_resolution_clock::now();

    parse_party_and_port(argv, &party, &port);
    setup();

    vector<Integer> file = intvec_to_emp(ascii_encodings);
    init_base64_table();
    int block_size = 64;  
    vector<Integer> hash1 = ssdeep_simulated(file, block_size);
    vector<Integer> hash2 = ssdeep_simulated(file, block_size * 2);

    if(party == ALICE) 
    {
      std::cout << "Client OT Cost setup: " << netio->counter << "bytes" << endl;
    }

    if(party == BOB) 
    {
      std::cout << "Server OT Cost setup: " << netio->counter << "bytes" << endl;
    }

    std::cout << "Number of AND gates used setup: "
    << CircuitExecution::circ_exec->num_and() << std::endl;

    done();
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    if(party == ALICE)
    {
      std::cout << "Client EmbedMap time setup: " << duration.count() << "ms" << endl; 
    }
    if(party == BOB)
    {
      std::cout << "Server EmbedMap time setup: " << duration.count() << "ms" << endl; 
    }
  }

  if (TEST_SDHASH) {
    auto start_time = high_resolution_clock::now();

    parse_party_and_port(argv, &party, &port);
    setup();

    vector<Integer> file = intvec_to_emp(ascii_encodings);
    vector<Integer> ret = sdhash_digest(file);

    int blocklist_size = 100;

    for (int i=0; i<blocklist_size*(ret.size()); i++) {
      ham(1);
    }

    __m128i key;
    __m128i iv;
    for (size_t i = 0; i < 16; ++i) {
      ((uint8_t *)(&key))[i] = (1337 * i) % 255;
      ((uint8_t *)(&iv))[i] = (31 * i) % 253;
    }
    Integer AES_ret = AES(ret, key, iv);

    

    if(party == ALICE) 
    {
      std::cout << "Client OT Cost setup: " << netio->counter << "bytes" << endl;
    }

    if(party == BOB) 
    {
      std::cout << "Server OT Cost setup: " << netio->counter << "bytes" << endl;
    }

    std::cout << "Number of AND gates used setup: "
    << CircuitExecution::circ_exec->num_and() << std::endl;

    done();
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    if(party == ALICE)
    {
      std::cout << "Client EmbedMap time setup: " << duration.count() << "ms" << endl; 
    }
    if(party == BOB)
    {
      std::cout << "Server EmbedMap time setup: " << duration.count() << "ms" << endl; 
    }
  }


  return 0;
}