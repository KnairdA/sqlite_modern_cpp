#include <iostream>

#include "sqlite_modern_cpp.h"

int main(int, char**) {
	try {
		// creates a database file 'dbfile.db' if it does not exists.
		sqlite::database db("dbfile.db");

		// executes the query and creates a 'user' table
		db <<
			"create table if not exists user ("
			"   age int,"
			"   name text,"
			"   weight real"
			");";

		// inserts a new user record.
		// binds the fields to '?' .
		// note that only types allowed for bindings are :
		//      int ,long, long long, float, double
		//      string , u16string
		// sqlite3 only supports utf8 and utf16 strings, you should use std::string for utf8 and std::u16string for utf16.
		// note that u"my text" is a utf16 string literal of type char16_t * .
		db << "insert into user (age,name,weight) values (?,?,?);"
			<< 20
			<< u"bob" // utf16 string
			<< 83.25f;

		db << u"insert into user (age,name,weight) values (?,?,?);" // utf16 query string
			<< 21
			<< "jack"
			<< 68.5;

		// slects from user table on a condition ( age > 18 ) and executes
		// the lambda for each row returned .
		db << "select age,name,weight from user where age > ? ;"
			<< 18
			>> [](int age, std::string name, double weight) {
			std::cout << age << ' ' << name << ' ' << weight << std::endl;
		};

		// selects the count(*) from user table
		// note that you can extract a single culumn single row result only to : int,long,long,float,double,string,u16string
		int count = 0;
		db << "select count(*) from user" >> count;
		std::cout << "std::cout : " << count << std::endl;

		// this also works and the returned value will be automatically converted to string
		std::string str_count;
		db << "select count(*) from user" >> str_count;
		std::cout << "scount : " << str_count << std::endl;
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}
