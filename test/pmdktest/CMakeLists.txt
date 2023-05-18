project(pmtest)

link_directories("/usr/local/lib")

add_executable(pmtest "")
target_sources(pmtest
  PRIVATE
    "pmtest.cpp"
    "pmtest.h"
)

target_link_libraries(pmtest pmemobj)

add_executable(pmemobjtest pmemobjtest.cpp)
target_link_libraries(pmemobjtest pmem)