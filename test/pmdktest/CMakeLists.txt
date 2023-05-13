project(pmtest)
add_executable(pmtest "")
target_sources(pmtest
  PRIVATE
    "pmtest.h"
    "pmtest.cpp"
)
target_link_libraries(pmtest pmempool pmemobj pthread stdc++fs)