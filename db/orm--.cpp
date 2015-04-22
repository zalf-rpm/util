#include <sstream>
#include <algorithm>
#include <iostream>
#include <map>
#include <cstdio>
#include <mutex>

#include "orm--.h"
#include "tools/algorithms.h"

using namespace Db;
using namespace std;
using namespace Tools;

string Identifiable::toString(const string& /*indent*/, bool /*detailed*/) const
{
	ostringstream s;
	s << id << ": " << name;
	return s.str();
}

string Describable::toString(const string& indent, bool /*detailed*/) const
{
	ostringstream s;
	s << Identifiable::toString(indent) << ", " << description;
	return s.str();
}
