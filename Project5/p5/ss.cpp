
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/filestream.h>
#include <iostream>
#include "stringBox.h"

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>

#include <jsoncpp/json/json.h>

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace concurrency;
using namespace http::experimental::listener;

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

mongocxx::instance inst{};

string connectDB(string term){
    
    string username = "vancleecheng";
    string pass = "951102ljc";
    mongocxx::client conn {
        mongocxx::uri{"mongodb://"+username+":"+pass+"@ds229878.mlab.com:29878/cpp5"} 
    };

    auto collection = conn["cpp5"]["project"];

    Json::Reader reader;
    Json::Value root;

    auto cursor = collection.find({});

    for (auto &&doc : cursor){
        string jsonStr = bsoncxx::to_json(doc);

        if(reader.parse(jsonStr, root)){
            string key = root["key"].asString();
            if(key == term){
                return root["value"].asString();
            }
        }
        
    }
}


template<typename _CharType>
pplx::task<streams::streambuf<_CharType>> OPENSTR_R(const utility::string_t &name)
{
#if !defined(__cplusplus_winrt)
    return streams::file_buffer<_CharType>::open(name, std::ios_base::in);
#else
    auto file = pplx::create_task(
        KnownFolders::DocumentsLibrary->GetFileAsync(ref new Platform::String(name.c_str()))).get();

    return streams::file_buffer<_CharType>::open(file, std::ios_base::in);
#endif
}


void handle_get(http_request message) {
   
    // cout << "Method: " << message.method() << std::endl;
 	// cout << "URI: " << http::uri::decode(message.relative_uri().path()) << std::endl;
 	// cout << "Query: " << http::uri::decode(message.relative_uri().query()) << std::endl;

    string term = http::uri::decode(message.relative_uri().query());
    cout << "term: " << term << endl;
    string result = connectDB(term);
    cout << "result: " << result << endl;

    // StringBox payload = StringBox();
    // payload.string = result;

    // message.reply(status_codes::OK, payload.AsJSON());

    streams::basic_istream<uint8_t> fistream = OPENSTR_R<uint8_t>(U("a.html")).get();

    http_response response(200);
    response.set_body(fistream);

    message.reply(response);

    // message.extract_json()
    //         .then([=](json::value value)
    //     {
    //         const std::string term = (StringBox::FromJSON(value.as_object()).string);
    //         cout << "term: " << term << endl;
    //         string result = connectDB(term);
    //         cout << "result: " << result << endl;

    //         StringBox payload = StringBox();
    //         payload.string = result;
            
    //         // auto answer = json::value::object();
    //         // answer["value"] = json::value::string(result);
    //         // cout << answer.serialize() << endl;

    //         message.reply(status_codes::OK, payload.AsJSON());
    //     });
            // .wait();
 
}


int main(){
	
	string_t address = U("http://172.17.0.2:34568/test");
    http_listener listener(address);
    listener.support(methods::GET, handle_get);
    listener.open().wait();

    std::cout << "Press ENTER to exit." << std::endl;

    std::string line;
    std::getline(std::cin, line);

    listener.close().wait();

    return 0;
}
