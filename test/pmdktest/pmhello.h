#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/make_persistent_atomic.hpp>
#include <libpmemobj++/p.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pool.hpp>

#include <fstream>
#include <iostream>
#include <string.h>

#define max_msg_size 30
#define LAYOUT ""

using namespace std;
namespace pobj = pmem::obj;

class Hello {
  private:
    char msg[max_msg_size] = {0};
  
  public:
    Hello(char* input) {
      snprintf(msg, sizeof(msg), "%s", input);
    }

    char* get_hello_msg() {
      return msg;
    }
};

struct root {
  pobj::persistent_ptr<Hello> hello;
};
