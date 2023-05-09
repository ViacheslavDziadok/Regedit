#include "regedit.hpp"

using namespace std;


string getVal(const aa::regedit::value& val) {
	string result;
	switch (val.type()) {
	case aa::regedit::type::sz: result = val.read<aa::regedit::type::sz>(); break;
	case aa::regedit::type::expand_sz: result = val.read<aa::regedit::type::expand_sz>(); break;
	case aa::regedit::type::dword: result = std::to_string(val.read<aa::regedit::type::dword>()); break;
	case aa::regedit::type::dword_big_endian: result = val.read<aa::regedit::type::dword_big_endian>(); break;
	case aa::regedit::type::qword: result = val.read<aa::regedit::type::qword>(); break;
	case aa::regedit::type::multi_sz: {
		vector<string> values = val.read<aa::regedit::type::multi_sz>();
		for (vector<string>::iterator vec_it = values.begin(); vec_it != values.end(); ++vec_it)
			result += *vec_it + "\n";
		break;
	}
	case aa::regedit::type::none: result = "none"; break;
	default: result = "unsupported type";
	}
	return result;
}

// recursive print of keys and subkeys
void printKeys(aa::regedit& reg, string indent = "") {
	cout << indent + "  " << "values (" << reg.values.size() << ") :" << endl;
	for (aa::regedit::values::iterator it_val = reg.values.begin(); it_val != reg.values.end(); ++it_val) {
		string val_type = aa::regedit::type_to_string(it_val->second.type());
		cout << indent + "    " << it_val->first << " (" << val_type << "): "
			<< getVal(it_val->second) << endl;
	}

	cout << indent + "  " << "inner keys (" << reg.size() << ") :" << endl;

	for (aa::regedit::iterator it = reg.begin(); it != reg.end(); ++it) {
		cout << indent + "  " << it->first << " (" << it->second.size() << ")" << endl;

		if (it->second.size() >= 0) {
			aa::regedit reg_inner = reg[it->first];
			printKeys(reg_inner, indent + "  ");
		}
	}
}

void createTests() {

	// aa::regedit is very similar to a std::map

	aa::regedit reg(aa::regedit::hkey::current_user, "Software", true);

	reg = reg["1test_key"];

	for (int i = 0; i < 10; i++)
		reg.insert("key " + std::to_string(i));
	reg["key 0"].emplace("skey 0");
	reg["key 0"]["skey 1"];
	reg["key 0"]["skey 1"]["sskey 0"];

	cout << "HKEY_CURRENT_USER\\Software\\_test_key:" << endl;
	cout << "outer_keys:" << endl;

	reg.values["sz_val"].write<aa::regedit::type::sz>("test_sz_string_value");
	reg.values.insert("dw_val").first->second.write<aa::regedit::type::dword>(12345);
	reg["key 0"].values["test_inner_1"].write<aa::regedit::type::sz>("inner_1_sz_string_value");
	reg["key 0"]["skey 1"].values["test_inner_2"].write<aa::regedit::type::sz>("inner_2_sz_string_value");
	reg["key 0"]["skey 1"]["sskey 0"].values["test_inner_3"].write<aa::regedit::type::sz>("inner_3_sz_string_value");

	printKeys(reg);
}

void findTestKey() {
	aa::regedit reg(HKEY_CURRENT_USER, "Software", true);
	aa::regedit::iterator it = reg.find("1test_key");
	if (it != reg.end()) {
		cout << "found" << endl;
	}
	else {
		cout << "not found" << endl;
	}
}

void eraseTestKey() {
	aa::regedit reg(HKEY_CURRENT_USER, "Software", true);
	reg.erase("1test_key");
}


void insertKey(string skey, string snew_key, HKEY hkey = HKEY_CURRENT_USER, bool write_permission = true) {
	aa::regedit reg(hkey, skey, write_permission);
	reg.insert(snew_key);
}

void showKey(string skey = "Software", HKEY hkey = HKEY_CURRENT_USER, bool write_permission = true) {
	aa::regedit reg(hkey, skey, write_permission);
	cout << "HKEY_CURRENT_USER\\Software:" << endl;
	cout << "  subkeys (" << reg.size() << ") :" << endl;
	for (aa::regedit::iterator it = reg.begin(); it != reg.end(); ++it)
		cout << "    " << it->first << " (" << it->second.size() << ")" << endl;

	cout << "  values (" << reg.values.size() << ") :" << endl;
	for (aa::regedit::values::iterator it = reg.values.begin(); it != reg.values.end(); ++it)
		cout << "    " << it->first << " (" << aa::regedit::type_to_string(it->second.type()) << ")" << endl;
}

void findValue(HKEY hkey, string key, bool write_permission) {
	aa::regedit reg(hkey, key, write_permission);
	aa::regedit::values::iterator it = reg.values.find("dw_val");
	if (it != reg.values.end()) {
		cout << "found" << endl;
	}
	else {
		cout << "not found" << endl;
	}
}

void showValue(HKEY hkey, string key, bool write_permission) {
	aa::regedit reg(hkey, key, write_permission);
	aa::regedit::values::iterator it = reg.values.find("dw_val");
	if (it != reg.values.end()) {
		cout << "found" << endl;
		cout << "value: " << it->second.read<aa::regedit::type::dword>() << endl;
	}
	else {
		cout << "not found" << endl;
	}
}

void getValue(HKEY hkey, string key, bool write_permission) {
	aa::regedit reg(hkey, key, write_permission);
	aa::regedit::value val = reg.values["dw_val"];
	cout << "value: " << val.read<aa::regedit::type::dword>() << endl;
}

void setValue(HKEY hkey, string key, bool write_permission) {
	aa::regedit reg(hkey, key, write_permission);
	reg.values.insert("dw_val").first->second.write<aa::regedit::type::dword>(54321);
}

void eraseValue(HKEY hkey, string key, bool write_permission) {
	aa::regedit reg(hkey, key, write_permission);
	reg.values.erase("dw_val");
}

void clearKey(HKEY hkey, string key, bool write_permission) {
	aa::regedit reg(hkey, key, write_permission);
	reg.clear();
}

void getSize(HKEY hkey, string key, bool write_permission) {
	aa::regedit reg(hkey, key, write_permission);
	cout << "size: " << reg.size() << endl;
}

int testValues() {
	HKEY hkey = HKEY_CURRENT_USER;
	string key = "Software";
	string subkey = key + "\\1test_key";
	bool write_permission = true;
	createTests();
	findTestKey();
	//eraseValue(hkey, subkey, write_permission);
	setValue(hkey, subkey, write_permission);
	findValue(hkey, subkey, write_permission);
	//eraseTestKey();
	return 0;
}


// list all names of public functions with their descriptions, parameters and return values

// regedit::insert() - insert a subkey to the current key

// regedit::erase() - erase a subkey from the current key

// regedit::find() - find a subkey in the current key

// regedit::is_open() - check if the current key is open

// regedit::close() - close the current key

// regedit::open() - open a key

// regedit::operator[]() - get a subkey

// regedit::empty() - check if the current key is empty

// regedit::size() - get the number of subkeys

// regedit::swap() - swap two regedit objects

// regedit::clear() - clear the current key

// regedit::emplace() - insert a subkey to the current key

// regedit::value::write() - write a value to the current value

// regedit::value::read() - read a value from the current value

// regedit::value::swap() - swap two values

// regedit::value::empty() - check if the current value is empty

// regedit::value::type() - get the type of the current value

// regedit::value::size() - get the size of the current value