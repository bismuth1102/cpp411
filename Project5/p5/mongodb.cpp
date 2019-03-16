#include <cstdint>
#include <iostream>
#include <vector>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>

#include <jsoncpp/json/json.h>

using namespace std;

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

int main(int argc, char* argv[]) {

    mongocxx::instance inst{};
    string username = "vancleecheng";
    string pass = "951102ljc";
    mongocxx::client conn {
        mongocxx::uri{"mongodb://"+username+":"+pass+"@ds229878.mlab.com:29878/cpp5"} 
    };

    auto collection = conn["cpp5"]["project"];

    bsoncxx::builder::stream::document document{};
    document << "key" << argv[1];
    document << "value" << argv[2];
    core::v1::optional<mongocxx::v_noabi::result::insert_one> result = collection.insert_one(document.view());

    auto cursor = collection.find({});

    Json::Reader reader;
    Json::Value root;

    // for (auto &&doc : cursor){
    //     string jsonStr = bsoncxx::to_json(doc);

    //     if(reader.parse(jsonStr, root)){
    //         string key = root["key"].asString();
    //         if(key == argv[1]){
    //             cout << root["value"].asString() << endl;
    //         }
    //     }
        
    // }

}