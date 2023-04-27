cd ./build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
cd ..
g++ -g leveldb_test.cpp -o leveldbtest test/libleveldb.a  -I../include -lpthread
