
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <iostream>
#include <list>

#include "Block.h"
#include "BlockChain.h"
#include "server.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace std;
using namespace boost::asio;
using namespace boost::posix_time;
using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

io_service service;
static Blockchain chain;
static list<string> mylist;

#define MEM_FN(x)       boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x,y)    boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)  boost::bind(&self_type::x, shared_from_this(),y,z)

class talk_to_client : public boost::enable_shared_from_this<talk_to_client>, boost::noncopyable {
    typedef talk_to_client self_type;
    talk_to_client() : sock_(service), started_(false) {}
public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<talk_to_client> ptr;

    void start() {
        started_ = true;
        do_read();
    }
    static ptr new_() {
        ptr new_(new talk_to_client);
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
        // cout << err << endl;
        if ( !err) {
            string msg(read_buffer_, bytes);
            cout << msg << endl;
            mylist.push_back(msg);
        }
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
};

ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8001));

void handle_accept(talk_to_client::ptr client, const boost::system::error_code & err) {
    client->start();
    talk_to_client::ptr new_client = talk_to_client::new_();
    acceptor.async_accept(new_client->sock(), boost::bind(handle_accept,new_client,_1));
}

void thread(){
    boost::asio::io_service io;
    boost::asio::deadline_timer t(io, boost::posix_time::seconds(60));
    int count = 0;
    // t.async_wait(boost::bind(Print, boost::asio::placeholders::error, &t, &count));
    t.async_wait(boost::bind(timer, boost::asio::placeholders::error, &t));
    io.run();
}


void timer(const boost::system::error_code &ec,
        boost::asio::deadline_timer* pt){

    string data = "";
    list<string>::iterator rit = mylist.begin();
    list<string>::iterator rend = mylist.end();
    for(;rit != rend;++rit)
        data.append(*rit);
    mylist.clear();

    Block lastBlock = chain._GetLastBlock();
    string lastBlockHash = lastBlock.sHash;
    int lastBlockIndex = lastBlock.getIndex();
    int nIndexIn = lastBlockIndex+1;
    Block newBlock = Block(nIndexIn, data);
    newBlock.sPrevHash = lastBlockHash;
    newBlock.MineBlock(4);
    chain.AddBlock(newBlock);

    ptree tree;
    tree.put("sHash", newBlock.sHash);
    tree.put("sPrevHash", newBlock.sPrevHash);
    tree.put("index", newBlock.getIndex());
    tree.put("nonce", newBlock.getNonce());
    tree.put("time", newBlock.getTime());
    tree.put("data", newBlock.getData());
    std::ostringstream buf;
    write_json (buf, tree, false);
    std::string json = buf.str();
    
    ofstream outfile;
    outfile.open("save.txt", ios::out|ios::app);
    outfile << json;
    outfile.close();

    pt->expires_at(pt->expires_at() + boost::posix_time::seconds(60)) ;
 
    pt->async_wait(boost::bind(timer, boost::asio::placeholders::error, pt));
}

int main(int argc, char* argv[]) {
    chain = Blockchain();

    char data1[1000];
    char data2[1000];
    ifstream in1;
    ifstream in2;
    in1.open("save.txt");
    in2.open("save.txt");
    in2.getline (data2,1000);
    while(!in2.eof()){
        in1.getline (data1,1000);
        in2.getline (data2,1000);
    }
    ptree pt;
    std::istringstream is (data1);
    read_json (is, pt);
    string sHash = pt.get<string> ("sHash");
    int index = pt.get<int> ("index");
    string sPrevHash = pt.get<string> ("sPrevHash");
    int nonce = pt.get<int> ("nonce");
    time_t time = pt.get<time_t> ("time");
    string data = pt.get<string> ("data");
    Block newBlock = Block(index, data, nonce, time, sHash, sPrevHash);
    chain.AddBlock(newBlock);
    
    boost::thread t(thread);

    talk_to_client::ptr client = talk_to_client::new_();
    acceptor.async_accept(client->sock(), boost::bind(handle_accept,client,_1));
    service.run();
}

