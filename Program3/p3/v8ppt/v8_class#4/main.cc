#include "common.h"
#include "print.h"
#include <iostream>
#include <math.h>
#include <v8.h>

// Forward declaration
class XY;

/** Unwrap an XY object from a request for a field access */
XY *UnwrapXY(const v8::AccessorInfo &info) {
// Get the wrapped C++ pointer
	v8::Local<v8::External> external = 
	v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0)); 
// Cast the value back to the desired type
	return static_cast<XY *>(external->Value());
}

/** XY is a pair type with two fields: x and y, both of type double */
struct XY {
  /** constructor sets initial values of x and y */
	XY(double _x, double _y) : x(_x), y(_y) {}
  /** empty destructor */
	~XY() {}
  /** The x value held by instances of this type */
	double x;
  /** The y value held by instances of this type */
	double y;
private:
  /** multiply both values by a scaling factor */
	void mul(const double scale) { 
		x *= scale;
		y *= scale;
	}
/** return the straight-line distance from (0,0) to (x,y) */
	double length() { return sqrt(x * x + y * y); }

public:
/**
* In JavaScript, we will support a "field access" to read .length. It's 
* actually a call to a getter, so we need to set up a field accessor 
*/
	static v8::Handle<v8::Value> js_get_length(v8::Local<v8::String> prop, 
		const v8::AccessorInfo &info) {
		v8::Locker lock;
		v8::HandleScope scope;
    // If we can get a C++ pointer, call length() on it and return it
		XY *thisXY = UnwrapXY(info);
		if (thisXY != NULL) {
			return scope.Close(v8::Number::New(thisXY->length())); 
		}
		return v8::Undefined();
	}

  /** We don't actually allow setting the length, so this will be a no-op */
	static void js_set_length(v8::Local<v8::String> prop, 
		v8::Local<v8::Value> value,
		const v8::AccessorInfo &info) {
		return; 
	}

	/** multiply() is a method call in JavaScript, like the stuff in v8-c.cc */
	static v8::Handle<v8::Value> js_mul(const v8::Arguments &args) 
	{ 
		v8::Locker lock;
		v8::HandleScope scope;
		// Unpack the args
		if (args.Length() == 1 && args[0]->IsNumber()) {
		// NB: be sure not to let lifecycle events happen to self! 
			v8::Local<v8::Object> self = args.Holder();
			XY *thisXY = static_cast<XY *>( 
				v8::Local<v8::External>::Cast(self->GetInternalField(0))->Value()); 
			if (thisXY != NULL) {
				thisXY->mul(args[0]->NumberValue());
				return scope.Close(self); 
			}
		}
		return v8::Undefined();
	}
	
};

/**
* Since we can't have a static constructor in a C++ class, and we need to refer 
* to static methods when creating JavaScript wrappers around C++ methods, we
* need a special C++ wrapper for constructing XY objects
*/
	v8::Handle<v8::Value> XYConstructor(const v8::Arguments &args) { 
		v8::Locker lock;
		v8::HandleScope scope;
// make sure we have two numbers
		if (!args[0].IsEmpty() && args[0]->IsNumber() && !args[1].IsEmpty() && 
			args[1]->IsNumber()) {
    // Create a c++ instance of this type,
			XY *xy = new XY(args[0]->NumberValue(), args[1]->NumberValue()); 
// Create a JS version and store the pointer inside 
		v8::Persistent<v8::Object> self = v8::Persistent<v8::Object>::New(args.Holder()); 
		self->SetInternalField(0, v8::External::New(xy));
    // Don't lose self when scope closes!
		return scope.Close(self); }
  // Invalid args, so return undefined
		return v8::Undefined();
	}

/**
* Add the XY class definition to a context template, so that we can access XY 
* from any context made from that template.
*/
	void expose_XY(v8::Handle<v8::ObjectTemplate> global) {
		v8::Locker lock; 
		v8::HandleScope scope;
// In old JavaScript, an object is actually a function, so we need a function 
// template for our class. We start the template using the constructor 
		v8::Persistent<v8::FunctionTemplate> XYTemplate = 
		v8::Persistent<v8::FunctionTemplate>::New( 
			v8::FunctionTemplate::New(XYConstructor));
// Set the name of the class in the template 
		XYTemplate->SetClassName(v8::String::New("XY"));
// Get the instance template, so we can add methods, getters, and setters 
		v8::Handle<v8::ObjectTemplate> XYIT = XYTemplate->InstanceTemplate(); 
	XYIT->SetInternalFieldCount(1); // for /this/ pointer 
	XYIT->Set(v8::String::New("mul"),
		v8::FunctionTemplate::New(v8::InvocationCallback(XY::js_mul))); 
	XYIT->SetAccessor(v8::String::New("length"), v8::AccessorGetter(XY::js_get_length),
		v8::AccessorSetter(XY::js_set_length));
  // Put it in the global scope as XY
	global->Set(v8::String::New("XY"), XYTemplate); 
}

int main(int argc, char **argv) {
	v8::Locker locker;
	v8::HandleScope handle_scope;
	v8::Handle<v8::ObjectTemplate> globalTemplate = v8::ObjectTemplate::New();

  	// add print_message to the global template
	globalTemplate->Set(v8::String::New("print"), 
		v8::FunctionTemplate::New(print_message)); 

	// add vector2 to the global template 
	expose_XY(globalTemplate);

  	// Create the context, run the code
	v8::Handle<v8::Context> context = v8::Context::New(NULL, globalTemplate); 
	if (argc > 1) {
		load_run_script(std::string(argv[1]), context);
	} else {
		std::cout << "Error: you must provide a JavaScript file name as the first " << "argument" << std::endl;
	} 
}

