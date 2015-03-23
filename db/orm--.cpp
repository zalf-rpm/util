#include <sstream>
#include <algorithm>
#include <iostream>
#include <map>
#include <cstdio>

#include <boost/foreach.hpp>

#include "orm--.h"
#include "tools/algorithms.h"

#include "tools/use-stl-algo-boost-lambda.h"

#define LOKI_OBJECT_LEVEL_THREADING
#include "loki/Threads.h"

using namespace Db;
using namespace std;
using namespace boost;
using namespace Tools;

namespace
{
	struct L : public Loki::ObjectLevelLockable<L> {};

}

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
