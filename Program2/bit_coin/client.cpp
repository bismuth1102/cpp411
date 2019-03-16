
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Block.h"
#include "BlockChain.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std;
using namespace boost::asio;
using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

io_service service;

#define MEM_FN(x)       boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x,y)    boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)  boost::bind(&self_type::x, shared_from_this(),y,z)

class talk_to_svr : 
public boost::enable_shared_from_this<talk_to_svr>
                  , boost::noncopyable {
    typedef talk_to_svr self_type;
    talk_to_svr(const string & message) 
      : sock_(service), started_(true), message_(message) {}
    void start(ip::tcp::endpoint ep) {
        sock_.async_connect(ep, MEM_FN1(on_connect,_1));
    }
public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<talk_to_svr> ptr;

    static ptr start(ip::tcp::endpoint ep, const string & message) {
        ptr new_(new talk_to_svr(message));
        new_->start(ep);
        return new_;
    }
    void stop() {
        if ( !started_) return;
        started_ = false;
        sock_.close();
    }
    bool started() { return started_; }
private:
    void on_connect(const error_code & err) {
        // if ( !err)      do_write(message_ + "\n");
        if ( !err)      do_read();
        else            stop();
    }
    
    void do_read() {
        async_read(sock_, buffer(read_buffer_), 
                   MEM_FN2(read_complete,_1,_2), MEM_FN2(on_read,_1,_2));
    }
    void on_read(const error_code & err, size_t bytes) {
        string previousJson(read_buffer_, bytes);
        cout << previousJson << endl;
        ptree pt;
        std::istringstream is (previousJson);
        read_json (is, pt);
        string lastBlockHash = pt.get<string> ("lastBlockHash");
        int lastBlockIndex = stoi(pt.get<string> ("lastBlockIndex"));
        int nIndexIn = lastBlockIndex+1;
        Block newBlock = Block(nIndexIn, "a new block");
        newBlock.sPrevHash = lastBlockHash;
        newBlock.MineBlock(5);

        ptree pt2;
        pt2.put("id", "127.0.0.1");
        pt2.put("sHash", newBlock.sHash);
        pt2.put("sPrevHash", newBlock.sPrevHash);
        pt2.put("index", newBlock.getIndex());
        pt2.put("nonce", newBlock.getNonce());
        pt2.put("time", newBlock.getTime());
        pt2.put("data", newBlock.getData());
        std::ostringstream buf;
        write_json (buf, pt2, false);
        string json = buf.str();
        cout << json << endl;

        do_write(json + "\n");
    }
    void do_write(const string & msg) {
        if ( !started() ) return;
        std::copy(msg.begin(), msg.end(), write_buffer_);
        sock_.async_write_some( buffer(write_buffer_, msg.size()), 
                                MEM_FN2(on_write,_1,_2));
    }
    void on_write(const error_code & err, size_t bytes) {
        // boost::this_thread::sleep( boost::posix_time::millisec(1500));
        // do_read();
        stop();
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
    string message_;
};

int main(int argc, char* argv[]) {
    // connect several clients
    ip::tcp::endpoint ep( ip::address::from_string("127.0.0.1"), 8001);
    // char* messages[] = { "John says hi", "so does James", "Lucy just got home", 0 };
    // for ( char ** message = messages; *message; ++message) {
        // talk_to_svr::start(ep, *message);
        // boost::this_thread::sleep( boost::posix_time::millisec(100));
    // }
    string block = "";
    talk_to_svr::start(ep, block);
    service.run();
}
