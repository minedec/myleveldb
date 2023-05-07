project(numatest)
add_executable(numatest "")
target_sources(numatest
  PRIVATE
    "threadpool.h" 
    "threadpool.cpp"
    "numatest.cpp"
)
target_link_libraries(numatest pthread numa)