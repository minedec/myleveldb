#include <iostream>
#include "../include/leveldb/slice.h"

class Mem {
  public:
    uint64_t a;
    uint64_t b;
    leveldb::Slice* s = nullptr;
};

Mem* m = nullptr;

void fun1(uint64_t *p) {
  m->a = 1;
  m->b = 2;
  m->s = new leveldb::Slice("abc");
  std::cout << "fun1 " << m->s->ToString() << std::endl;
}

int main() {
  m = new Mem();
  uint64_t p = 0;
  fun1(&p);
  if(m->s != nullptr) {
    std::cout << m->s->ToString() << std::endl;
  }
}
