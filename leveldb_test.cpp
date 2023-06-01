#include <assert.h>
#include <string.h>
#include "leveldb/db.h"
#include <iostream>
#include <unistd.h>
#include <string>

int main(){
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    std::string path(getcwd(NULL, 0));
    path += "/../dbtest";
    leveldb::Status status = leveldb::DB::Open(options,path, &db);
    assert(status.ok());
    std::cout << "db path:" << path << std::endl;

    std::cout << "leveldb_test start" << std::endl;

    //write key1,value1
    std::string key="key";
    std::string value = "value";

    status = db->Put(leveldb::WriteOptions(), key,value);
    assert(status.ok());

    for(int i = 0; i < 10000; i++) {
        std::string key = "key";
        key += std::to_string(random());
        std::string value = "value";
        value += std::to_string(random());
        status = db->Put(leveldb::WriteOptions(), key,value);
        assert(status.ok());
    }

    status = db->Get(leveldb::ReadOptions(), key, &value);
    assert(status.ok());
    std::cout<< "key:" << key << " value:" << value<<std::endl;
    std::string key2 = "key2";
    
    //move the value under key to key2
    
    status = db->Put(leveldb::WriteOptions(),key2,value);
    assert(status.ok());
    status = db->Delete(leveldb::WriteOptions(), key);

    assert(status.ok());
    
    status = db->Get(leveldb::ReadOptions(),key2, &value);
    
    assert(status.ok());
    std::cout<<"key:" << key2 << " value:" <<value<<std::endl;
    
    status = db->Get(leveldb::ReadOptions(),key, &value);
    
    if(!status.ok()) std::cerr<<key<<"  "<<status.ToString()<<std::endl;
    else std::cout<< "key:" << key << " value:" <<value<<std::endl;

    std::cout << "leveldb_test end" << std::endl;
    
    delete db;
    return 0;
}