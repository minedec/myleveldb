CURRENT_DIR=$(cd "$(dirname "$0")";pwd)

cd $CURRENT_DIR/build
cmake -DCMAKE_BUILD_TYPE=DEBUG $CURRENT_DIR/build/..
cmake --build $CURRENT_DIR/build
cd $CURRENT_DIR/..
g++ -g $CURRENT_DIR/leveldb_test.cpp -o $CURRENT_DIR/leveldbtest $CURRENT_DIR/build/libleveldb.a -I$CURRENT_DIR/include -lnuma -lpthread