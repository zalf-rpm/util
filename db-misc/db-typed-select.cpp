#include <cstdlib> 

#include "db/db-typed-select.h"

int Db::convert(char* s, Loki::Type2Type<int>){
	return atoi(s);
}
double Db::convert(char* s, Loki::Type2Type<double>){
	return atof(s);
}
