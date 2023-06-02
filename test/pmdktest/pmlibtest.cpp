#include <iostream>
#include <libpmem.h>
#include <string>
#include <memory.h>

int main() {
  std::string s = "/mnt/pmem0/testfile";
  const char* path = s.c_str();
  size_t FILE_LEN = 1024;
  size_t mapped_len;
  int is_pmemp;
  char* fileaddr;

  // Use mmap create a file mapping
  if((fileaddr = (char*)pmem_map_file(path, FILE_LEN, PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmemp)) == nullptr) {
    std::cout << "null addr quit\n";
    return 1;
  }

  pmem_memset(fileaddr, 0, FILE_LEN, 0);

  // Write some data
  pmem_memcpy(fileaddr, "abcdefg", FILE_LEN, 0);
  // strcpy(fileaddr, "abcdefg");

  // // No persist, direct unmap
  // pmem_unmap(file, FILE_LEN);

  // persist and unmap
  pmem_persist(fileaddr, FILE_LEN);
  pmem_unmap(fileaddr, FILE_LEN);

  // // read file
  if((fileaddr = (char*)pmem_map_file(path, FILE_LEN, PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmemp)) == nullptr) {
    std::cout << "null addr quit\n";
    return 1;
  }

  char* res = new char[FILE_LEN+1];
  pmem_memcpy(res, fileaddr, FILE_LEN, 0);
  std::cout << "Read form PM: " << res << std::endl;
}