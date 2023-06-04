CURRENT_DIR=$(cd "$(dirname "$0")";pwd)

cd $CURRENT_DIR/build
cmake -DCMAKE_BUILD_TYPE=Debug $CURRENT_DIR/build/..
cmake --build $CURRENT_DIR/build
cd $CURRENT_DIR/..
export LD_LIBRARY_PATH=/usr/local/lib/:$LD_LIBRARY_PATH
g++ -g $CURRENT_DIR/leveldb_test.cpp -o $CURRENT_DIR/leveldbtest  -I$CURRENT_DIR/include $CURRENT_DIR/build/libleveldb.a  -lnuma -lpthread -lsnappy -L/usr/local/lib -lpmem