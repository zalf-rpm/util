
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
