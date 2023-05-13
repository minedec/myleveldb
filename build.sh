cd ./build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cd ../
g++ -g leveldb_test.cpp -o leveldbtest build/libleveldb.a -Iinclude -lnuma -lpthread -lsnappy
