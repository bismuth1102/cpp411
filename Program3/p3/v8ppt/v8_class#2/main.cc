#include "common.h" 
#include "print.h" 
#include <iostream> 
#include <v8.h>
int main(int argc, char **argv) {
	v8::Locker locker;
	v8::HandleScope handle_scope;
	v8::Handle<v8::ObjectTemplate> globalTemplate = v8::ObjectTemplate::New();
	// add the print_message() function to the global template as "print()", so 
	// that any context we make can call print() 
	globalTemplate->Set(v8::String::New("print"),
		v8::FunctionTemplate::New(print_message));
    // Create the context, run the script in it:
	v8::Handle<v8::Context> context = v8::Context::New(NULL, globalTemplate); 
	if (argc > 1) {
		load_run_script(std::string(argv[1]), context);
	} else {
		std::cout << "Error: you must provide a JavaScript file name as the first " << "argument" << std::endl;
	} 
}
