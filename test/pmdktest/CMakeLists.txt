project(pmtest)

link_directories("/usr/local/lib")
add_executable(pmhello "")
target_sources(pmhello
  PRIVATE
    "pmhello.cpp"
    "pmhello.h"
)
target_link_libraries(pmhello pmemobj)

add_executable(pmlibtest pmlibtest.cpp)
target_link_libraries(pmlibtest pmem)