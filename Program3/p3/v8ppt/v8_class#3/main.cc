#include "common.h" 
#include "print.h" 
#include <iostream> 
#include <v8.h>

using namespace std;
// Forward declare the object that we will be using in this example
class CppInt;
// Forward declare method that gets CppInt from JavaScript wrapper 
CppInt *UnwrapCppInt(v8::Handle<v8::Object> jso);
/** CppInt is a class that wraps an integer */
struct CppInt {
/** a value. It's public to save us some effort later */ 
	int val = 0;
	string test = "aa";
	vector<string> str_v;
/** empty constructor */
	CppInt() {}
/** empty destructor */
	~CppInt() {}
private:
/** inc() increments the integer in this class instance */
	int inc() { ++val; return val;}
/** show() prints the value in this class instance */
	void show() { std::cout << "The value is " << val << std::endl; }

	void showstr() { std::cout << "The string is " << test << std::endl; }

	void showstr_v() { 
		str_v.push_back("aaa");
		str_v.push_back("bbb");
		cout << "The string vector is shown below " << endl;
		for (int i = 0; i < str_v.size(); ++i)
		{
			cout << i << ": " << str_v[i] << endl;
		}
	}

public:
/**
* When JavaScript code calls "inc()", we will end up here. It's static, 
* because /this/ is a parameter in args.
*/
	static v8::Handle<v8::Value> js_inc(const v8::Arguments &args) { 
		v8::Locker locker; 
		v8::HandleScope scope;
// get the this pointer, call the method, and we're done 
		CppInt *o = UnwrapCppInt(args.This());
		o->inc();
		return v8::Undefined();
	}
/**
* When JavaScript code calls "show()", we will end up here. It's static, * because /this/ is a parameter in args.
*/
	static v8::Handle<v8::Value> js_show(const v8::Arguments &args) { 
		v8::Locker locker; 
		v8::HandleScope scope;
		CppInt *o = UnwrapCppInt(args.This());
		o->show();
		return v8::Undefined();
	}

	static v8::Handle<v8::Value> js_showstr(const v8::Arguments &args) { 
		v8::Locker locker; 
		v8::HandleScope scope;
		CppInt *o = UnwrapCppInt(args.This());
		o->showstr();
		return v8::Undefined();
	}

	static v8::Handle<v8::Value> js_showstr_v(const v8::Arguments &args) { 
		v8::Locker locker; 
		v8::HandleScope scope;
		CppInt *o = UnwrapCppInt(args.This());
		o->showstr_v();
		return v8::Undefined();
	}
};

/**
* UnwrapCppInt is a helper function that takes a JavaScript version of an 
* object and tries to get it as a CppInt.
*/
CppInt *UnwrapCppInt(v8::Handle<v8::Object> jsObject) { 
	v8::Handle<v8::External> pointer = 
	v8::Handle<v8::External>::Cast(jsObject->GetInternalField(0)); 
	return static_cast<CppInt *>(pointer->Value());
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
v8::Handle<v8::Object> create_js_object(CppInt *instance) { 
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

int main(int argc, char **argv) {
	v8::Locker locker; 
	v8::HandleScope handle_scope; 
	v8::Handle<v8::ObjectTemplate> globalTemplate = v8::ObjectTemplate::New();

  	// add print_message to the global template
	// print = print message
	globalTemplate->Set(v8::String::New("print"), 
		v8::FunctionTemplate::New(print_message));

// add a new number called "version" to the global template... it's readonly
	globalTemplate->Set(v8::String::New("version"), v8::Number::New(1.1),v8::ReadOnly); 

// Create the context
	v8::Handle<v8::Context> context = v8::Context::New(NULL, globalTemplate);

// If we want to put a C++ object into the context, we need to open a context 
// scope (RAII):
	v8::Context::Scope context_scope(context);

// Create our object, and make a JavaScript wrapper around it
	CppInt *cpp_int = new CppInt();
	v8::Handle<v8::Object> js_int = create_js_object(cpp_int);

// Tell JavaScript about the inc and show methods of the object:
	create_js_method(js_int, v8::String::New("inc"), 
		v8::InvocationCallback(CppInt::js_inc)); 
	create_js_method(js_int, v8::String::New("show"), 
		v8::InvocationCallback(CppInt::js_show));
	create_js_method(js_int, v8::String::New("showstr"), 
		v8::InvocationCallback(CppInt::js_showstr));
	create_js_method(js_int, v8::String::New("showstr_v"), 
		v8::InvocationCallback(CppInt::js_showstr_v));

// Put the wrapped object into the global scope
// NB: optional 3rd parameter of v8::ReadOnly 
	context->Global()->Set(v8::String::New("int"), js_int);

// Run the script in the context, and look at cpp_int when the script is done
	if (argc > 1) {
		load_run_script(std::string(argv[1]), context);
		std::cout << "From C++, value is " << cpp_int->val << std::endl;
	} else {
		std::cout << "Error: you must provide a JavaScript file name as the first "
		<< "argument" << std::endl;
	}
}