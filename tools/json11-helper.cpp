/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
Authors:
Michael Berg <michael.berg@zalf.de>

Maintainers:
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of
Landscape Systems Analysis at the ZALF.
Copyright (C) Leibniz Centre for Agricultural Landscape Research (ZALF)
*/

#include "json11-helper.h"

using namespace Tools;
using namespace std;
using namespace json11;

EResult<Json> Tools::readAndParseJsonFile(string path)
{
	auto r = readFile(path);
	if(r.success())
		return parseJsonString(r.result);
	return{Json(), r.errors};
}

EResult<Json> Tools::parseJsonString(string jsonString)
{
	string err;
	Json j = Json::parse(jsonString, err);
	if(!err.empty())
		return{j, string("Error parsing JSON object: '") + jsonString + "' "};
	return{j};
}

//-----------------------------------------------------------------------------

void Tools::set_double_vectorD(std::vector<double>& var,
                               const json11::Json& j,
                               const std::string& key,
                               const std::vector<double>& def,
                               double defaultValue,
                               bool useDefault)
{
  string err;
  if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string() && a[0].is_array())
      var = double_vectorD(a[0], def, defaultValue);
    else
      var = double_vectorD(a, def, defaultValue);
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
  {
    auto o = j[key];
    if(o.has_shape({{"value", json11::Json::ARRAY}}, err))
      var = double_vectorD(o["value"], def, defaultValue);
  }
  else if(useDefault)
    var = def;
}

std::vector<double> Tools::double_vectorD(const json11::Json& j,
                                          const std::vector<double>& def,
                                          double defaultValue)
{
  if(j.is_array())
  {
    if(j.array_items().size() > 1 && j[1].is_string() && j[0].is_array())
      return double_vectorD(j[0], def, defaultValue);
    else
    {
      std::vector<double> is;
      for(json11::Json v : j.array_items())
        is.push_back(double_valueD(v, defaultValue));
      return is;
    }
  }
  else if(j.is_object())
  {
		string err;
    if(j.has_shape({{"value", json11::Json::ARRAY}}, err))
      return double_vectorD(j["value"], def, defaultValue);
  }

  return def;
}

std::vector<double> Tools::double_vectorD(const json11::Json& j,
                                          const std::string& key,
                                          const std::vector<double>& def,
                                          double defaultValue)
{
  std::vector<double> res(def);
  set_double_vectorD(res, j, key, def, defaultValue);
  return res;
}

//---------

void Tools::set_int_vectorD(std::vector<int>& var,
                            const json11::Json& j,
                            const std::string& key,
                            const std::vector<int>& def,
                            int defaultValue,
                            bool useDefault)
{
  string err;
  if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string() && a[0].is_array())
      var = int_vectorD(a[0], def, defaultValue);
    else
      var = int_vectorD(a, def, defaultValue);
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
  {
    auto o = j[key];
    if(o.has_shape({{"value", json11::Json::ARRAY}}, err))
      var = int_vectorD(o["value"], def, defaultValue);
  }
  else if(useDefault)
    var = def;
}

std::vector<int> Tools::int_vectorD(const json11::Json& j,
                                    const std::vector<int>& def,
                                    int defaultValue)
{
  if(j.is_array())
  {
    if(j.array_items().size() > 1 && j[1].is_string() && j[0].is_array())
      return int_vectorD(j[0], def, defaultValue);
    else
    {
      std::vector<int> is;
      for(json11::Json v : j.array_items())
        is.push_back(int_valueD(v, defaultValue));
      return is;
    }
  }
  else if(j.is_object())
  {
		string err;
    if(j.has_shape({{"value", json11::Json::ARRAY}}, err))
      return int_vectorD(j["value"], def, defaultValue);
  }

  return def;
}

std::vector<int> Tools::int_vectorD(const json11::Json& j,
                                    const std::string& key,
                                    const std::vector<int>& def,
                                    int defaultValue)
{
  std::vector<int> res(def);
  set_int_vectorD(res, j, key, def, defaultValue);
  return res;
}

//----------

void Tools::set_bool_vectorD(std::vector<bool>& var,
                             const json11::Json& j,
                             const std::string& key,
                             const std::vector<bool>& def,
                             bool defaultValue,
                             bool useDefault)
{
  string err;
  if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string() && a[0].is_array())
      var = bool_vectorD(a[0], def, defaultValue);
    else
      var = bool_vectorD(a, def, defaultValue);
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
  {
    auto o = j[key];
    if(o.has_shape({{"value", json11::Json::ARRAY}}, err))
      var = bool_vectorD(o["value"], def, defaultValue);
  }
  else if(useDefault)
    var = def;
}


std::vector<bool> Tools::bool_vectorD(const json11::Json& j,
                                      const std::vector<bool>& def,
                                      bool defaultValue)
{
  if(j.is_array())
  {
    if(j.array_items().size() > 1 && j[1].is_string() && j[0].is_array())
      return bool_vectorD(j[0], def, defaultValue);
    else
    {
      std::vector<bool> is;
      for(json11::Json v : j.array_items())
        is.push_back(bool_valueD(v, defaultValue));
      return is;
    }
  }
  else if(j.is_object())
  {
		string err;
    if(j.has_shape({{"value", json11::Json::ARRAY}}, err))
      return bool_vectorD(j["value"], def, defaultValue);
  }

  return def;
}

std::vector<bool> Tools::bool_vectorD(const json11::Json& j,
                                      const std::string& key,
                                      const std::vector<bool>& def,
                                      bool defaultValue)
{
  std::vector<bool> res(def);
  set_bool_vectorD(res, j, key, def, defaultValue);
  return res;
}

//------------------

void Tools::set_string_vectorD(std::vector<std::string>& var,
                               const json11::Json& j,
                               const string& key,
                               const std::vector<string>& def,
                               const std::string& defaultValue,
                               bool useDefault)
{
  string err;
  if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string() && a[0].is_array())
      var = string_vectorD(a[0], def, defaultValue);
    else
      var = string_vectorD(a, def, defaultValue);
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
  {
    auto o = j[key];
    if(o.has_shape({{"value", json11::Json::ARRAY}}, err))
      var = string_vectorD(o["value"], def, defaultValue);
  }
  else if(useDefault)
    var = def;
}

std::vector<string> Tools::string_vectorD(const json11::Json& j,
                                          const std::vector<string>& def,
                                          const string& defaultValue)
{
  if(j.is_array())
  {
    if(j.array_items().size() > 1 && j[1].is_string() && j[0].is_array())
      return string_vectorD(j[0], def, defaultValue);
    else
    {
      std::vector<string> is;
      for(json11::Json v : j.array_items())
        is.push_back(string_valueD(v, defaultValue));
      return is;
    }
  }
  else if(j.is_object())
  {
    string err;
    if(j.has_shape({{"value", json11::Json::ARRAY}}, err))
      return string_vectorD(j["value"], def, defaultValue);
  }

  return vector<string>();
}

std::vector<string> Tools::string_vectorD(const json11::Json& j,
                                          const std::string& key,
                                          const std::vector<string>& def,
                                          const string& defaultValue)
{
  std::vector<string> res(def);
  set_string_vectorD(res, j, key, def, defaultValue);
  return res;
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

void Tools::set_double_valueD(double& var,
                              const json11::Json& j,
                              const std::string& key,
                              double def,
                              std::function<double(double)> transf,
                              bool useDefault)
{
  string err;
  if(j.has_shape({{key, json11::Json::NUMBER}}, err))
    var = transf(j[key].number_value());
  else if(j.has_shape({{key, json11::Json::ARRAY}}, err))
  {
    auto a = j[key];
    if(a.array_items().size() > 1 && a[1].is_string())
    {
      auto v = a[0];
      if(v.is_number())
        var = transf(v.number_value());
    }
  }
  else if(j.has_shape({{key, json11::Json::OBJECT}}, err))
    set_double_valueD(var, j[key], "value", def, transf);
  else if(useDefault)
    var = transf(def);
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

double Tools::double_valueD(const json11::Json& j, 
														const std::string& key, 
														double def)
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

//---------------------------------------------------------

void Tools::set_iso_date_value(Tools::Date& var,
                               const json11::Json& j,
                               const std::string& key)
{
  string defaultMarker = "__DEFAULT__USED__";
  string dateStr = string_valueD(j, key, defaultMarker);
  if(dateStr != defaultMarker)
    var = Date::fromIsoDateString(dateStr, var.useLeapYears());
}

Tools::Date Tools::iso_date_value(const json11::Json& j, const std::string& key)
{
  Date res;
  set_iso_date_value(res, j, key);
  return res;
}
