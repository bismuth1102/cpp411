
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <iostream>
#include "stringBox.h"

using namespace std;
using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams


pplx::task<http_response> make_task_request(http_client & client, method mtd, string msg, json::value const & jvalue)
{
   return client.request(mtd, conversions::to_string_t(msg), jvalue);
}

void make_request(http_client & client, method mtd, string msg, json::value const & jvalue)
{
   make_task_request(client, mtd, msg, jvalue)
        .then([](http_response response)
        {
            if (response.status_code() == status_codes::OK)
                cout << "status: OK" << endl;
            else
                cout << "status: not ok" << endl;
        })
        .wait();
}

v8::Handle<v8::Value> print_message(const v8::Arguments &args){

    v8::Locker locker;
    v8::HandleScope scope;
    
    v8::String::Utf8Value message(args[0]->ToString()); 
    cout << *message << endl;

    http_client client(U("http://172.17.0.2:34568/test"));

    auto nullvalue = json::value::null();
    make_request(client, methods::GET, *message, nullvalue);

    return scope.Close(v8::Boolean::New(true));
}

