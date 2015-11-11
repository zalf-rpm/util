#include <cstdlib>
#include <iostream>
#include <string>

#include "helper.h"

using namespace Tools;
using namespace std;

bool Tools::satob(const std::string& s, bool def)
{
  if(s.empty())
    return def;

  auto start = toLower(s).at(0);

  switch(start)
  {
  case 't': case '1':
    return true;
  case 'f': case '0':
    return false;
  default:
    return def;
  }
}

string Tools::fixSystemSeparator(std::string path)
{
#ifdef WIN32
	auto pos = path.find("/");
	while(pos != string::npos)
	{
		path.replace(pos, 1, "\\");
		pos = path.find("/", pos + 1);
	}
#endif
	return path;
}

void Tools::ensureDirExists(std::string& path)
{
#ifdef WIN32
  string mkdir("mkdir ");
#else
  string mkdir("mkdir -p ");
#endif

	string fullCmd = fixSystemSeparator(mkdir + path);
	
	int res = system(fullCmd.c_str());
	//cout << "res: " << res << endl;
}
