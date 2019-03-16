
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <fstream>
#include <iostream>
#include <list>
#include <vector>
#include <regex>
#include <v8.h>

#include "pair.h"
#include "common.h"

using namespace std;
using namespace boost::asio;

io_service service;
vector<Pair> comVec;    //result of where
string comStr;  //result of combine
string result;  //response of server
vector<list<Pair> > vectorData; //data structure
boost::thread_group group;  //use for thread pool
boost::mutex whereMutex;    //use for where()
boost::ptr_vector<boost::mutex> lockerMutexs;   //store all mutexs of lists
v8::Handle<v8::Context> context;
string reduceRegex; //the parameter of reduce()

#define MEM_FN(x)       boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x,y)    boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)  boost::bind(&self_type::x, shared_from_this(),y,z)

class talk_to_client;
talk_to_client *UnwrapClass(v8::Handle<v8::Object> jso);
struct talk_to_client : public boost::enable_shared_from_this<talk_to_client>, boost::noncopyable {
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

    // hash key to certain list
    int hashKey(string key){
        std::hash<std::string> h;
        size_t n = h(key);
        int listNum = n%65536;
        return listNum;
    }

    string get(string key){
        int listNum = hashKey(key);
        boost::mutex::scoped_lock lock(lockerMutexs.at(listNum));    // lock the list
        list<Pair> thisList = vectorData.at(listNum);
        for(auto it=thisList.begin(); it!=thisList.end(); ++it){
            Pair pair = *it;
            if(pair.getKey()==key){
                return pair.getValue();
            }
        }
        return "";
    }

    bool del(string key){
        int listNum = hashKey(key);
        boost::mutex::scoped_lock lock(lockerMutexs.at(listNum));    // lock the list
        list<Pair> thisList = vectorData.at(listNum);
        for(auto it=thisList.begin(); it!=thisList.end(); ++it){
            Pair pair = *it;
            if(pair.getKey()==key){
                thisList.erase(it);
                return true;
            }
        }
        return false;
    }

    void put(Pair _pair){
        string key = _pair.getKey();
        int listNum = hashKey(key);
        boost::mutex::scoped_lock lock(lockerMutexs.at(listNum));    // lock the list
        list<Pair> thisList = vectorData.at(listNum);
        for(auto it=thisList.begin(); it!=thisList.end(); ++it){
            Pair pair = *it;
            if(pair.getKey()==key){
                thisList.erase(it);
                thisList.push_back(_pair);
            }
        }
        thisList.push_back(_pair);
    }

    //reduce's combine will invoke this
    void combine(){
        boost::mutex::scoped_lock lock(whereMutex);
        for(int i=0; i<comVec.size(); i++){
            comStr = comStr + "," + comVec.at(i).getValue();
        }
    }

    //reduce's filter will invoke this
    void where(string str){
        //lock where() because this function change a global variable
        boost::mutex::scoped_lock lock(whereMutex);
        regex e(str);
        for(int i=0; i<vectorData.size(); i++){
            boost::mutex::scoped_lock lock(lockerMutexs.at(i));    // lock the list
            list<Pair> thisList = vectorData.at(i);
            for(auto it=thisList.begin(); it!=thisList.end(); ++it){
                Pair pair = *it;
                string key = pair.getKey();
                if(regex_match (key, e)){
                    Pair copyedPair(pair);
                    comVec.push_back(copyedPair);
                }
            }
        }
    }

    static v8::Handle<v8::Value> filter(const v8::Arguments &args) { 
        v8::Locker locker;
        v8::HandleScope scope;
        talk_to_client *o = UnwrapClass(args.This());
        o->where(reduceRegex);
        return v8::Undefined();
    }

    static v8::Handle<v8::Value> combine(const v8::Arguments &args) { 
        v8::Locker locker;
        v8::HandleScope scope;
        talk_to_client *o = UnwrapClass(args.This());
        o->combine();
        return v8::Undefined();
    }

    // operation on data structure
    void operate_db(string msg){
        vector<string> vStr;
        boost::split( vStr, msg, boost::is_any_of( "\n" ), boost::token_compress_on);
        string first = vStr.at(0);
        if(first=="PUT"){
            string key = vStr.at(2);
            string value = vStr.at(4);
            Pair pair = Pair(key, value);
            put(pair);
            result = "OK\n";
        }
        else if(first=="GET"){
            string key = vStr.at(2);
            string value = get(key);
            if(value==""){
                result = "ERROR\n";
            }
            else{
                int length = result.length();
                result = "OK\n" + to_string(length) + "\n" + value + "\n";
            }
        }
        else if(first=="DEL"){
            string key = vStr.at(2);
            bool done = del(key);
            if(done==true){
                result = "OK\n";
            }
            else{
                result = "ERROR\n";
            }
        }
        else if(first=="WHERE"){
            where(vStr.at(2));
            if(comVec.size()==0){
                result = "ERROR\n";
            }
            else{
                result = "OK\n" + to_string(comVec.size()) + "\n";
                for(int i=0; i<comVec.size(); i++){
                    Pair pair = comVec.at(i);
                    int keyLength = pair.getKey().length();
                    int valueLength = pair.getValue().length();
                    result += to_string(keyLength) + "\n" + pair.getKey() + "\n" + to_string(valueLength) + "\n" + pair.getValue() + "\n";
                }
            }
        }
        else if(first=="REDUCE"){
            string filterJs = vStr.at(2);
            string combineJs = vStr.at(4);
            // I change the protocol a little bit.. line 6 is regex for filter in reduce.
            reduceRegex = vStr.at(5);
            load_run_script(filterJs, context);
            load_run_script(combineJs, context);
            if(comVec.size()==0){
                result = "ERROR\n";
            }
            else{
                int length = comStr.length();
                result = "OK\n" + to_string(length) + "\n" + comStr + "\n";
            }
            comStr = "";
        }
    }

private:
    void do_read() {
        async_read(sock_, buffer(read_buffer_), 
                   MEM_FN2(read_complete,_1,_2), MEM_FN2(on_read,_1,_2));
    }
    void on_read(const error_code & err, size_t bytes) {
        if ( !err) {
            string msg(read_buffer_, bytes);
            //since I wanna bind a member function, I need to add "this"
            group.create_thread(boost::bind(&talk_to_client::operate_db, this, msg));
        }
        do_write(result);
        stop();
    }
    void do_write(const std::string & msg) {
        std::copy(msg.begin(), msg.end(), write_buffer_);
        sock_.async_write_some( buffer(write_buffer_, msg.size()), 
                                MEM_FN2(on_write,_1,_2));
    }
    void on_write(const error_code & err, size_t bytes) {
        do_read();
    }
    
    size_t read_complete(const boost::system::error_code & err, size_t bytes) {
        if ( err) return 0;
        // bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
        // // we read one-by-one until we get to enter, no buffering
        // return found ? 0 : 1;
        return 1;
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

/**
* UnwrapCppInt is a helper function that takes a JavaScript version of an 
* object and tries to get it as a CppInt.
*/
talk_to_client *UnwrapClass(v8::Handle<v8::Object> jsObject) { 
    v8::Handle<v8::External> pointer = 
    v8::Handle<v8::External>::Cast(jsObject->GetInternalField(0)); 
    return static_cast<talk_to_client *>(pointer->Value());
}

/**
* Attach a name to a JavaScript object, so that the name can be used to call a 
* function in C++ code, as if it were a method of the object
*/
void create_js_method(v8::Handle<v8::Object> js_object, 
    v8::Handle<v8::Value> js_meth_name, 
    v8::InvocationCallback cpp_func) {
    v8::Locker lock;
    v8::HandleScope handle_scope;
    // NB: optional additional parameter of v8::ReadOnly
    js_object->Set(js_meth_name, v8::FunctionTemplate::New(cpp_func)->GetFunction()); 
}

/**
 * Create a JavaScript wrapper around a C++ object
 */
v8::Handle<v8::Object> create_js_object(talk_to_client *instance) { 
    v8::HandleScope scope;
    // make an empty object, set it so we can put a C++ object in it 
    v8::Handle<v8::ObjectTemplate> base_tpl = v8::ObjectTemplate::New(); 
    base_tpl->SetInternalFieldCount(1);

    // Create the actual template object. It's persistent, so our C++ object 
    // doesn't get reclaimed
    v8::Persistent<v8::ObjectTemplate> real_tpl = 
    v8::Persistent<v8::ObjectTemplate>::New(base_tpl);

    // Allocate JS object, wrap the C++ instance with it
    v8::Handle<v8::Object> result = real_tpl->NewInstance(); 
    result->SetInternalField(0, v8::External::New(instance));

    // Return the js object, make sure it doesn't get reclaimed by the scope
    return scope.Close(result);
}


int main(int argc, char* argv[]) {

    for(int i=0; i<65536; i++){
        list<Pair> list;
        vectorData.push_back(list);
        lockerMutexs.push_back(new boost::mutex);
    }

    v8::Locker locker;
    v8::HandleScope handle_scope;
    v8::Handle<v8::ObjectTemplate> globalTemplate = v8::ObjectTemplate::New();

    context = v8::Context::New(NULL, globalTemplate); 
    v8::Context::Scope context_scope(context);
    talk_to_client *clientJs = new talk_to_client();
    v8::Handle<v8::Object> js_server = create_js_object(clientJs);

    create_js_method(js_server, v8::String::New("filter"), 
        v8::InvocationCallback(talk_to_client::filter)); 
    create_js_method(js_server, v8::String::New("combine"), 
        v8::InvocationCallback(talk_to_client::combine));
    
    context->Global()->Set(v8::String::New("server"), js_server);

    talk_to_client::ptr client = talk_to_client::new_();
    acceptor.async_accept(client->sock(), boost::bind(handle_accept,client,_1));
    service.run();
}

