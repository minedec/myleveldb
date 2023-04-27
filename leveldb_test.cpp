#include <assert.h>
#include <string.h>
#include <leveldb/db.h>
#include <iostream>

int main(){
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    std::string path = "/home/minedec/testdb";
    leveldb::Status status = leveldb::DB::Open(options,path, &db);
    assert(status.ok());
    std::cout << "db path:" << path << std::endl;

    //write key1,value1
    std::string key="key";
    std::string value = "value";

    status = db->Put(leveldb::WriteOptions(), key,value);
    assert(status.ok());

    status = db->Get(leveldb::ReadOptions(), key, &value);
    assert(status.ok());
    std::cout<<value<<std::endl;
    std::string key2 = "key2";
    
    //move the value under key to key2
    
    status = db->Put(leveldb::WriteOptions(),key2,value);
    assert(status.ok());
    status = db->Delete(leveldb::WriteOptions(), key);

    assert(status.ok());
    
    status = db->Get(leveldb::ReadOptions(),key2, &value);
    
    assert(status.ok());
    std::cout<<key2<<"==="<<value<<std::endl;
    
    status = db->Get(leveldb::ReadOptions(),key, &value);
    
    if(!status.ok()) std::cerr<<key<<"  "<<status.ToString()<<std::endl;
    else std::cout<<key<<"==="<<value<<std::endl;
    
    delete db;
    return 0;
}