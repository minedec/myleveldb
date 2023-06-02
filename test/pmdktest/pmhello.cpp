#include "pmtest.h"
#include <error.h>

inline bool file_exists(const std::string & name) {
  std::ifstream f(name.c_str());
  return f.good();
}

static void show_usage(string name) {
  cerr << "usage: " << name << "lkjlkj;l" << endl;
}

void write_hello_string(char *input, char *path) {
  pobj::pool<root> pop;
  pobj::persistent_ptr<root> pool;

  pop = pobj::pool<root>::create(path, LAYOUT, PMEMOBJ_MIN_POOL, S_IRUSR|S_IWUSR);
  pool = pop.root();

  pobj::make_persistent_atomic<Hello> (pop, pool->hello, input);

  cout << endl << "\nWrite the " << pool->hello->get_hello_msg() << " string to persistent-memory." << endl;

  pop.close();
  return;
}

void read_hello_string(char* input, char* path) {
  pobj::pool<root> pop;
  pobj::persistent_ptr<root> pool;

  pop = pobj::pool<root>::open(path, LAYOUT);
  pool = pop.root();

  cout << endl << "\nRead the " << pool->hello->get_hello_msg() << " string from persistent-memory." << endl;

  pop.close();
  return;
}

int main(int argc, char* argv[]) {
  pobj::pool<root> pop;
  pobj::persistent_ptr<root> pool;

  if(argc < 3) {
    return 1;
  }

  char *path = argv[2];
  char input[max_msg_size] = "Hello Persistent Memory";

  if(strcmp(argv[1], "-w") == 0) {
    write_hello_string(input, path);
  } else if(strcmp(argv[1], "-r") == 0) {
    read_hello_string(input, path);
  } else {
    exit(1);
  }

  return 0;
}

