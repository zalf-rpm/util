#include "json11-helper.h"

using namespace Tools;
using namespace std;

std::vector<double> Tools::double_vectorD(const json11::Json& j, double def)
{
  if(j.is_array())
  {
    if(j.array_items().size() > 1 && j[1].is_string() && j[0].is_array())
      return double_vectorD(j[0], def);
    else
    {
      std::vector<double> is;
      for(json11::Json v : j.array_items())
        is.push_back(double_valueD(v, def));
      return is;
    }
  }
  else if(j.is_object())
  {
		string err;
    if(j.has_shape({{"value", json11::Json::ARRAY}}, err))
      return double_vectorD(j["value"], def);
  }

  return std::vector<double>();
}

std::vector<double> Tools::double_vectorD(const json11::Json& j, const std::string& key, double def)
{
	string err;
  if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string() && a[0].is_array())
      return double_vectorD(a[0], def);
    else
      return double_vectorD(a, def);
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
  {
    auto o = j[key];
    if(o.has_shape({{"value", json11::Json::ARRAY}}, err))
      return double_vectorD(o["value"], def);
  }

  return std::vector<double>();
}

//---------

std::vector<int> Tools::int_vectorD(const json11::Json& j, int def)
{
  if(j.is_array())
  {
    if(j.array_items().size() > 1 && j[1].is_string() && j[0].is_array())
      return int_vectorD(j[0], def);
    else
    {
      std::vector<int> is;
      for(json11::Json v : j.array_items())
        is.push_back(int_valueD(v, def));
      return is;
    }
  }
  else if(j.is_object())
  {
		string err;
    if(j.has_shape({{"value", json11::Json::ARRAY}}, err))
      return int_vectorD(j["value"], def);
  }

  return std::vector<int>();
}

std::vector<int> Tools::int_vectorD(const json11::Json& j, const std::string& key, int def)
{
	string err;
  if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string() && a[0].is_array())
      return int_vectorD(a[0], def);
    else
      return int_vectorD(a, def);
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
  {
    auto o = j[key];
    if(o.has_shape({{"value", json11::Json::ARRAY}}, err))
      return int_vectorD(o["value"], def);
  }

  return std::vector<int>();
}

//----------

std::vector<bool> Tools::bool_vectorD(const json11::Json& j, bool def)
{
  if(j.is_array())
  {
    if(j.array_items().size() > 1 && j[1].is_string() && j[0].is_array())
      return bool_vectorD(j[0], def);
    else
    {
      std::vector<bool> is;
      for(json11::Json v : j.array_items())
        is.push_back(bool_valueD(v, def));
      return is;
    }
  }
  else if(j.is_object())
  {
		string err;
    if(j.has_shape({{"value", json11::Json::ARRAY}}, err))
      return bool_vectorD(j["value"], def);
  }

  return std::vector<bool>();
}

std::vector<bool> Tools::bool_vectorD(const json11::Json& j, const std::string& key, bool def)
{
	string err;
  if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string() && a[0].is_array())
      return bool_vectorD(a[0], def);
    else
      return bool_vectorD(a, def);
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
  {
    auto o = j[key];
    if(o.has_shape({{"value", json11::Json::ARRAY}}, err))
      return bool_vectorD(o["value"], def);
  }

  return std::vector<bool>();
}

//------------------

std::vector<string> Tools::string_vectorD(const json11::Json& j, const string& def)
{
  if(j.is_array())
  {
    if(j.array_items().size() > 1 && j[1].is_string() && j[0].is_array())
      return string_vectorD(j[0], def);
    else
    {
      std::vector<string> is;
      for(json11::Json v : j.array_items())
        is.push_back(string_valueD(v, def));
      return is;
    }
  }
  else if(j.is_object())
  {
		string err;
    if(j.has_shape({{"value", json11::Json::ARRAY}}, err))
      return string_vectorD(j["value"], def);
  }

  return std::vector<string>();
}

std::vector<string> Tools::string_vectorD(const json11::Json& j, const std::string& key, const string& def)
{
	string err;
  if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string() && a[0].is_array())
      return string_vectorD(a[0], def);
    else
      return string_vectorD(a, def);
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
  {
    auto o = j[key];
    if(o.has_shape({{"value", json11::Json::ARRAY}}, err))
      return string_vectorD(o["value"], def);
  }

  return std::vector<string>();
}

//-------------------

void Tools::set_int_valueD(int& var, const json11::Json& j, const std::string& key, int def, bool useDefault)
{
  string err;
  if(j.has_shape({{key, json11::Json::NUMBER}}, err))
    var = j[key].int_value();
  else if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string())
    {
      auto v = a[0];
      if(v.is_number())
        var = v.int_value();
    }
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
    var = int_valueD(j[key], "value", def);
  else if(useDefault)
    var = def;
}

int Tools::int_valueD(const json11::Json& j, int def)
{
  if(j.is_number())
    return j.int_value();
  else if(j.is_array() && j.array_items().size() > 1 && j[1].is_string())
  {
    auto v = j[0];
    if(v.is_number())
      return v.int_value();
  }
  else if(j.is_object())
    return int_valueD(j, "value", def);

  return def;
}

int Tools::int_valueD(const json11::Json& j, const std::string& key, int def)
{
  int res(def);
  set_int_valueD(res, j, key, def);
  return res;
}

//-------------------

void Tools::set_double_valueD(double& var, const json11::Json& j, const std::string& key, double def, bool useDefault)
{
  string err;
  if(j.has_shape({{key, json11::Json::NUMBER}}, err))
    var = j[key].number_value();
  else if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string())
    {
      auto v = a[0];
      if(v.is_number())
        var = v.number_value();
    }
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
    var = double_valueD(j[key], "value", def);
  else if(useDefault)
    var = def;
}

double Tools::double_valueD(const json11::Json& j, double def)
{
  if(j.is_number())
    return j.number_value();
  else if(j.is_array() && j.array_items().size() > 1 && j[1].is_string())
  {
    auto v = j[0];
    if(v.is_number())
      return v.number_value();
  }
  else if(j.is_object())
    return double_valueD(j, "value", def);

  return def;
}

double Tools::double_valueD(const json11::Json& j, const std::string& key, double def)
{
  double res(def);
  set_double_valueD(res, j, key, def);
  return res;
}

//-------------------

void Tools::set_bool_valueD(bool& var, const json11::Json& j, const std::string& key, bool def, bool useDefault)
{
  string err;
  if(j.has_shape({{key, json11::Json::BOOL}}, err))
    var = j[key].bool_value();
  else if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string())
    {
      auto v = a[0];
      if(v.is_bool())
        var = v.bool_value();
    }
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
    var = bool_valueD(j[key], "value", def);
  else if(useDefault)
    var = def;
}

bool Tools::bool_valueD(const json11::Json& j, bool def)
{
  if(j.is_bool())
    return j.bool_value();
  else if(j.is_array() && j.array_items().size() > 1 && j[1].is_string())
  {
    auto v = j[0];
    if(v.is_bool())
      return v.bool_value();
  }
  else if(j.is_object())
    return bool_valueD(j, "value", def);

  return def;
}

bool Tools::bool_valueD(const json11::Json& j, const std::string& key, bool def)
{
  bool res(def);
  set_bool_valueD(res, j, key, def);
  return res;
}

//-------------------

void Tools::set_string_valueD(string& var, const json11::Json& j, const std::string& key, const string& def, bool useDefault)
{
  string err;
  if(j.has_shape({{key, json11::Json::STRING}}, err))
    var = j[key].string_value();
  else if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string())
    {
      auto v = a[0];
      if(v.is_string())
        var = v.string_value();
    }
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
    var = string_valueD(j[key], "value", def);
  else if(useDefault)
    var = def;
}

string Tools::string_valueD(const json11::Json& j, const string& def)
{
  if(j.is_string())
    return j.string_value();
  else if(j.is_array() && j.array_items().size() > 1 && j[1].is_string())
  {
    auto v = j[0];
    if(v.is_string())
      return v.string_value();
  }
  else if(j.is_object())
    return string_valueD(j, "value", def);

  return def;
}

string Tools::string_valueD(const json11::Json& j, const std::string& key, const string& def)
{
  string res(def);
  set_string_valueD(res, j, key, def);
  return res;
}
