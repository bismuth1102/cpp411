
#include <iostream>
#include <fstream>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>

#include "cs.h"

using namespace std;
using namespace boost::asio;

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

io_service service;
mongocxx::instance inst{};

#define MEM_FN(x)       boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x,y)    boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)  boost::bind(&self_type::x, shared_from_this(),y,z)

class server : public boost::enable_shared_from_this<server>, boost::noncopyable {
    typedef server self_type;
    server() : sock_(service), started_(false) {}
public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<server> ptr;

    void start() {
        started_ = true;
        do_read();
    }
    static ptr new_() {
        ptr new_(new server);
        return new_;
    }
    void stop() {
        if ( !started_) return;
        started_ = false;
        sock_.close();
    }
    ip::tcp::socket & sock() { return sock_;}
private:

    void do_read() {
        async_read(sock_, buffer(read_buffer_), 
                   MEM_FN2(read_complete,_1,_2), MEM_FN2(on_read,_1,_2));
    }
    void on_read(const error_code & err, size_t bytes) {
        if ( !err) {
            string msg(read_buffer_, bytes);
            
            vector<string> vStr;
            boost::split( vStr, msg, boost::is_any_of( "`" ), boost::token_compress_on);
            string key = vStr[0];
            string value = vStr[1];
            store(key, value);
            stop();
            
        }
        
    }
    
    size_t read_complete(const boost::system::error_code & err, size_t bytes) {
        if ( err) return 0;
        bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
        // we read one-by-one until we get to enter, no buffering
        return found ? 0 : 1;
    }
private:
    ip::tcp::socket sock_;
    enum { max_msg = 1024 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    bool started_;
};

ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8001));

void handle_accept(server::ptr client, const boost::system::error_code & err) {
    client->start();
    server::ptr new_client = server::new_();
    acceptor.async_accept(new_client->sock(), boost::bind(handle_accept,new_client,_1));
}


int main(int argc, char* argv[]) {
    server::ptr client = server::new_();
    acceptor.async_accept(client->sock(), boost::bind(handle_accept,client,_1));
    service.run();
}


void store(string key, string value){
    string username = "vancleecheng";
    string pass = "951102ljc";
    mongocxx::client conn {
        mongocxx::uri{"mongodb://"+username+":"+pass+"@ds229878.mlab.com:29878/cpp5"} 
    };

    auto collection = conn["cpp5"]["project"];

    bsoncxx::builder::stream::document document{};
    document << "key" << key;
    document << "value" << value;
    core::v1::optional<mongocxx::v_noabi::result::insert_one> result = collection.insert_one(document.view());

    cout << "done" << endl;
    
}
