
using namespace std;

class Pair{
public:
	Pair(string _key, string _value);
	string getValue(){return value;};
	string getKey(){return key;};
	
private:
	string key;
	string value;
};