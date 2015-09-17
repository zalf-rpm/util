#include "json11-helper.h"

using namespace Tools;
using namespace std;

std::vector<double> Tools::double_vector(const json11::Json& j, double def)
{
  std::vector<double> ds;
  for(json11::Json v : j.array_items())
    ds.push_back(v.is_number() ? v.number_value() : def);
  return ds;
}

std::vector<double> Tools::double_vector(const json11::Json& object, const std::string& key, double def)
{
  if(object.has_shape({{key, json11::Json::ARRAY}}, string()))
    return double_vector(object[key], def);
  return std::vector<double>();
}

std::vector<int> Tools::int_vector(const json11::Json& j, int def)
{
  std::vector<int> is;
  for(json11::Json v : j.array_items())
    is.push_back(v.is_number() ? v.int_value() : def);
  return is;
}

std::vector<int> Tools::int_vector(const json11::Json& object, const std::string& key, int def)
{
  if(object.has_shape({{key, json11::Json::ARRAY}}, string()))
    return int_vector(object[key], def);
  return std::vector<int>();
}

std::vector<bool> Tools::bool_vector(const json11::Json& j, bool def)
{
  std::vector<bool> bs;
  for(json11::Json v : j.array_items())
    bs.push_back(v.is_bool() ? v.bool_value() : def);
  return bs;
}

std::vector<bool> Tools::bool_vector(const json11::Json& object, const std::string& key, bool def)
{
  if(object.has_shape({{key, json11::Json::ARRAY}}, string()))
    return bool_vector(object[key], def);
  return std::vector<bool>();
}

std::vector<std::string> Tools::string_vector(const json11::Json& j, const std::string& def)
{
  std::vector<string> ss;
  for(json11::Json v : j.array_items())
    ss.push_back(v.is_string() ? v.string_value() : def);
  return ss;
}

std::vector<std::string> Tools::string_vector(const json11::Json& object, const std::string& key, const std::string& def)
{
  if(object.has_shape({{key, json11::Json::ARRAY}}, string()))
    return string_vector(object[key], def, "");
  return std::vector<string>();
}

void Tools::set_int_value(int& var, const json11::Json& j, const std::string& key, int def)
{
  string err;

  if(j.has_shape({{key, json11::Json::NUMBER}}, err))
    var = j["key"].int_value();
  else if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j["key"];
    if(a.array_items().size() > 0)
    {
      auto v = a[0];
      if(v.is_number())
        var = v.int_value();
    }
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
  {
    auto o = j["key"];
    if(o.has_shape({{"value", json11::Json::NUMBER}}, err))
      var = o["value"].int_value();
  }

  var = def;
}

int Tools::int_value(const json11::Json& j, const std::string& key, int def)
{
  int res(def);
  set_int_value(res, j, key, def);
  return res;
}

void Tools::set_number_value(double& var, const json11::Json& j, const std::string& key, double def)
{
  string err;

  if(j.has_shape({{key, json11::Json::NUMBER}}, err))
    var = j["key"].number_value();
  else if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j["key"];
    if(a.array_items().size() > 0)
    {
      auto v = a[0];
      if(v.is_number())
        var = v.number_value();
    }
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
  {
    auto o = j["key"];
    if(o.has_shape({{"value", json11::Json::NUMBER}}, err))
      var = o["value"].number_value();
  }

  var = def;
}

double Tools::number_value(const json11::Json& j, const std::string& key, double def)
{
  double res(def);
  set_number_value(res, j, key, def);
  return res;
}

void Tools::set_bool_value(bool& var, const json11::Json& j, const std::string& key, bool def)
{
  string err;

  if(j.has_shape({{key, json11::Json::BOOL}}, err))
    var = j["key"].bool_value();
  else if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j["key"];
    if(a.array_items().size() > 0)
    {
      auto v = a[0];
      if(v.is_bool())
        var = v.bool_value();
    }
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
  {
    auto o = j["key"];
    if(o.has_shape({{"value", json11::Json::BOOL}}, err))
      var = o["value"].bool_value();
  }

  var = def;
}

bool Tools::bool_value(const json11::Json& j, const std::string& key, bool def)
{
  bool res(def);
  set_bool_value(res, j, key, def);
  return res;
}

void Tools::set_string_value(string& var, const json11::Json& j, const std::string& key, const string& def)
{
  string err;

  if(j.has_shape({{key, json11::Json::STRING}}, err))
    var = j["key"].string_value();
  else if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j["key"];
    if(a.array_items().size() > 0)
    {
      auto v = a[0];
      if(v.is_string())
        var = v.string_value();
    }
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
  {
    auto o = j["key"];
    if(o.has_shape({{"value", json11::Json::STRING}}, err))
      var = o["value"].string_value();
  }

  var = def;
}

string Tools::string_value(const json11::Json& j, const std::string& key, const string& def)
{
  string res(def);
  set_string_value(res, j, key, def);
  return res;
}
