
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


pplx::task<http_response> make_task_request(http_client & client, method mtd, json::value const & jvalue)
{
   return client.request(mtd, conversions::to_string_t("/get"), jvalue);
}

string make_request(http_client & client, method mtd, json::value const & jvalue)
{
   make_task_request(client, mtd, jvalue)
      .then([](http_response response)
      {
         if (response.status_code() == status_codes::OK)
         {
            return response.extract_json();
         }
         return pplx::task_from_result(json::value());
      })
      .then([](pplx::task<json::value> previousTask)
      {
         try
         {
            return StringBox::FromJSON(previousTask.get().as_object()).string;
         }
         catch (http_exception const & e)
         {
            cout << e.what() << endl;
         }
      })
      .wait();
}

int main( int argc, char* argv[] ){
    http_client client(U("http://172.17.0.2:34568/test"));

    StringBox payload = StringBox();
    payload.string = argv[1];

    // auto nullvalue = json::value::null();
    string result = make_request(client, methods::GET, payload.AsJSON());

    cout << result << endl;

    // std::cout << "Press ENTER to exit." << std::endl;

    // std::string line;
    // std::getline(std::cin, line);
    return 0;
}

