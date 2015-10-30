#include <cstdlib>

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

void Tools::ensureDirExists(std::string& path)
{
#ifdef WIN32
  string mkdir("mkdir -p ");
#else
  string mkdir("mkdir ");
#endif

 system((mkdir + path).c_str());
}
