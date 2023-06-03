#include <iostream>
#include <libpmem.h>
#include <string>
#include <memory.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

int main() {
  std::string s = "/mnt/pmem0/dbtest/testfile1";
  const char* path = s.c_str();
  size_t FILE_LEN = 1024 * 1024 * 4;
  size_t mapped_len;
  int is_pmemp;
  char* fileaddr;
  int offset = 0;
  const char* str = "abcdefg\n";

  // step1 : direct use mmap create file
  // Use mmap create a file mapping
  if((fileaddr = (char*)pmem_map_file(path, FILE_LEN, PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmemp)) == nullptr) {
    std::cout << "null addr quit\n";
    return 1;
  }

  pmem_memset(fileaddr, 0, FILE_LEN, 0);

  // Write some data
  pmem_memcpy(fileaddr, str, strlen(str), 0);
  offset += strlen(str);
  pmem_memcpy(fileaddr + offset, str, strlen(str), 0);

  // persist and unmap
  pmem_persist(fileaddr, FILE_LEN);
  pmem_unmap(fileaddr, FILE_LEN);

  // read file
  if((fileaddr = (char*)pmem_map_file(path, FILE_LEN, PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmemp)) == nullptr) {
    std::cout << "null addr quit\n";
    return 1;
  }

  char* res = new char[FILE_LEN+1];
  pmem_memcpy(res, fileaddr, FILE_LEN, 0);
  std::cout << "Read form PM: " << res << std::endl;

  // step2: create a file descriptor and use mmap expand it
  s = "/mnt/pmem0/dbtest/testfile2";
  int fd = open(s.c_str(), O_TRUNC | O_RDWR | O_CREAT, 0644);
  if(fd < 0) {
    std::cout << "error fd\n";
    return 1;
  }
  fallocate(fd, 0, 0, 1);
  char* faddr2 = (char*)mmap(NULL, FILE_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  offset = 0;
  memcpy(faddr2, str, strlen(str));
  offset += strlen(str);
  memcpy(faddr2 + offset, str, strlen(str));
  // munmap(faddr2, strlen(str) * 2 + 1);
  ftruncate(fd, 128);
  
  // close(fd);

  //step3: use vector to store date and mmap
  s = "/mnt/pmem0/dbtest/testfile3";
  fd = open(s.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
  if(fd == -1) {
    std::cout << "error fd\n";
    return 1;
  }

  ftruncate(fd, 82445);
  char* faddr3 = (char*)mmap(NULL, FILE_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  std::vector<char> vec;
  vec.resize(FILE_LEN);
  std::string tmps = "";
  for(int i = 0; i < 50; i++) {
    tmps += str;
  }
  memcpy(&vec[0], tmps.c_str(), tmps.size());
  memcpy(faddr3, &vec[0], 82445);
  

  // ftruncate(fd, 824485);
  close(fd);

  //step4: use char array to store data and mmap
  s = "/mnt/pmem0/dbtest/testfile4";
  fd = open(s.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
  if(fd == -1) {
    std::cout << "error fd\n";
    return 1;
  }

  char* faddr4 = (char*)mmap(NULL, FILE_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  fallocate(fd, 0, 0, 1);
  char* veca = (char*)malloc(FILE_LEN);
  offset = 0;
  memcpy(faddr4, str, strlen(str));
  offset += strlen(str);
  memcpy(faddr4 + offset, str, strlen(str));
  offset += strlen(str);
  char* saddr = veca;
  // pmem_memcpy(faddr4, veca, offset, 0);

  ftruncate(fd, 128);
  close(fd);
}