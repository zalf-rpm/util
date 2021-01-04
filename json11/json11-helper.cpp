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

#include <string>

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

Errors Json11Serializable::merge(json11::Json j)
{
	Errors res;

	if(j["DEFAULT"].is_object())
		res = merge(j["DEFAULT"]);
	if(j["="].is_object())
		res = merge(j["="]);

	return res;
}

//-----------------------------------------------------------------------------

void Tools::set_double_vectorD(std::vector<double>& var,
                               const json11::Json& j,
                               const std::string& key,
                               const std::vector<double>& def,
                               double defaultValue,
                               bool useDefault)
{
  auto v = j[key];
  if (v.is_null() && useDefault) var = def;
  if(v.is_array())
  {
    if(v.array_items().size() > 1 && v[1].is_string() && v[0].is_array()) var = double_vectorD(v[0], def, defaultValue);
    else var = double_vectorD(v, def, defaultValue);
  }
  else if(v.is_object() && v["value"].is_array()) var = double_vectorD(v["value"], def, defaultValue);
}

std::vector<double> Tools::double_vectorD(const json11::Json& j,
                                          const std::vector<double>& def,
                                          double defaultValue)
{
  if(j.is_array())
  {
    if(j.array_items().size() > 1 && j[1].is_string() && j[0].is_array()) return double_vectorD(j[0], def, defaultValue);
    else
    {
      std::vector<double> is;
      for(json11::Json v : j.array_items()) is.push_back(double_valueD(v, defaultValue));
      return is;
    }
  }
  else if(j.is_object() && j["value"].is_array()) return double_vectorD(j["value"], def, defaultValue);
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
  auto v = j[key];
  if (v.is_null() && useDefault) var = def;
  if(v.is_array())
  {
    if(v.array_items().size() > 1 && v[1].is_string() && v[0].is_array()) var = int_vectorD(v[0], def, defaultValue);
    else var = int_vectorD(v, def, defaultValue);
  }
  else if(v.is_object() && v["value"].is_array()) var = int_vectorD(v["value"], def, defaultValue);
}

std::vector<int> Tools::int_vectorD(const json11::Json& j,
                                    const std::vector<int>& def,
                                    int defaultValue)
{
  if(j.is_array())
  {
    if(j.array_items().size() > 1 && j[1].is_string() && j[0].is_array()) return int_vectorD(j[0], def, defaultValue);
    else
    {
      std::vector<int> is;
      for(json11::Json v : j.array_items()) is.push_back(int_valueD(v, defaultValue));
      return is;
    }
  }
  else if(j.is_object() && j["value"].is_array()) return int_vectorD(j["value"], def, defaultValue);
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
  auto v = j[key];
  if (v.is_null() && useDefault) var = def;
  else if(v.is_array())
  {
    if(v.array_items().size() > 1 && v[1].is_string() && v[0].is_array()) var = bool_vectorD(v[0], def, defaultValue);
    else var = bool_vectorD(v, def, defaultValue);
  }
  else if(v.is_object() && v["value"].is_array()) var = bool_vectorD(v["value"], def, defaultValue);
}


std::vector<bool> Tools::bool_vectorD(const json11::Json& j,
                                      const std::vector<bool>& def,
                                      bool defaultValue)
{
  if(j.is_array())
  {
    if(j.array_items().size() > 1 && j[1].is_string() && j[0].is_array()) return bool_vectorD(j[0], def, defaultValue);
    else
    {
      std::vector<bool> is;
      for(json11::Json v : j.array_items())
        is.push_back(bool_valueD(v, defaultValue));
      return is;
    }
  }
  else if(j.is_object() && j["value"].is_array()) return bool_vectorD(j["value"], def, defaultValue);
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
  auto v = j[key];
  if (v.is_null() && useDefault) var = def;
  else if(v.is_array())
  {
    if(v.array_items().size() > 1 && v[1].is_string() && v[0].is_array()) var = string_vectorD(v[0], def, defaultValue);
    else var = string_vectorD(v, def, defaultValue);
  }
  else if(v.is_object() && v["value"].is_array()) var = string_vectorD(v["value"], def, defaultValue);
  
}

std::vector<string> Tools::string_vectorD(const json11::Json& j,
                                          const std::vector<string>& def,
                                          const string& defaultValue)
{
  if(j.is_array())
  {
    if(j.array_items().size() > 1 && j[1].is_string() && j[0].is_array()) return string_vectorD(j[0], def, defaultValue);
    else
    {
      std::vector<string> is;
      for(json11::Json v : j.array_items()) is.push_back(string_valueD(v, defaultValue));
      return is;
    }
  }
  else if(j.is_object() && j["value"].is_array()) return string_vectorD(j["value"], def, defaultValue);
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
  auto v = j[key];
  if (v.is_null() && useDefault) var = def;
  else if(v.is_number()) var = v.int_value();
  else if(v.is_string()) var = stoi(v.string_value());
  else if(v.is_array() && v.array_items().size() >= 1 && v[0].is_number()) var = v[0].int_value();
  else if(v.is_object()) var = int_valueD(v, "value", def);
}

int Tools::int_valueD(const json11::Json& j, int def)
{
  if(j.is_number()) return j.int_value();
  else if (j.is_string()) return stoi(j.string_value());
  else if(j.is_array() && j.array_items().size() >= 1 && j[0].is_number()) return j[0].int_value();
  else if(j.is_object()) return int_valueD(j, "value", def);
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
  auto v = j[key];
  if (v.is_null() && useDefault) var = transf(def);
  else if(v.is_number()) var = transf(v.number_value());
  else if (v.is_string()) var = transf(stod(v.string_value()));
  else if(v.is_array() && v.array_items().size() >= 1 && v[0].is_number()) var = transf(v[0].number_value());
  else if(v.is_object()) set_double_valueD(var, v, "value", def, transf);
}

double Tools::double_valueD(const json11::Json& j, double def)
{
  if (j.is_number()) return j.number_value();
  else if (j.is_string()) return stod(j.string_value());
  else if (j.is_array() && j.array_items().size() >= 1 && j[0].is_number()) return j[0].number_value();
  else if(j.is_object()) return double_valueD(j, "value", def);
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
  auto v = j[key];
  if (v.is_null() && useDefault) var = def;
  else if (v.is_bool()) var = v.bool_value();
  else if (v.is_string()) {
    auto bs = toUpper(v.string_value());
    if (bs == "TRUE" || bs == "FALSE") var = bs == "TRUE";
  }
  else if (v.is_array() && v.array_items().size() >= 1 && v[0].is_bool()) var = v[0].bool_value();
  else if(v.is_object()) var = bool_valueD(v, "value", def);
}

bool Tools::bool_valueD(const json11::Json& j, bool def)
{
  if(j.is_bool()) return j.bool_value();
  else if (j.is_string()) {
    auto bs = toUpper(j.string_value());
    if (bs == "TRUE" || bs == "FALSE") return bs == "TRUE";
  }
  else if(j.is_array() && j.array_items().size() >= 1 && j[0].is_bool()) return j[0].bool_value();
  else if(j.is_object()) return bool_valueD(j, "value", def);
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
  auto v = j[key];
  if (v.is_null() && useDefault) var = def;
  else if(v.is_string()) var = v.string_value();
  else if(v.is_array() && v.array_items().size() >= 1 && v[0].is_string()) var = v[0].string_value();
  else if(v.is_object()) var = string_valueD(v, "value", def);
}

string Tools::string_valueD(const json11::Json& j, const string& def)
{
  if (j.is_string()) return j.string_value();
  else if (j.is_array() && j.array_items().size() >= 1 && j[0].is_string()) return j[0].string_value();
  else if(j.is_object()) return string_valueD(j, "value", def);
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
  if(dateStr != defaultMarker) var = Date::fromIsoDateString(dateStr);
}

Tools::Date Tools::iso_date_value(const json11::Json& j, const std::string& key)
{
  Date res;
  set_iso_date_value(res, j, key);
  return res;
}
