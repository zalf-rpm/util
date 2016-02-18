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

#ifndef JSON11_HELPER_H_
#define JSON11_HELPER_H_

#include <string>
#include <functional>
#include <initializer_list>

#include "json11/json11.hpp"

#include "tools/date.h"

//#include "tools/helper.h"

namespace Tools
{
  template<typename T>
  T identity(T t){ return t; }

  struct Json11Serializable
  {
    virtual void merge(json11::Json j) = 0;

    virtual json11::Json to_json() const = 0;

    virtual std::string toString() const { return to_json().dump(); }
  };

  //-------------------------------------------

  typedef json11::Json::array J11Array;
  typedef json11::Json::object J11Object;

  template<class Collection>
  std::vector<json11::Json> toPrimJsonArray(const Collection& col)
  {
    std::vector<json11::Json> js;
    for(auto v : col)
      js.push_back(v);
    return js;
  }

  template<class Collection>
  std::vector<json11::Json> toJsonArray(const Collection& col)
  {
    std::vector<json11::Json> js;
    for(typename Collection::value_type t : col)
      js.push_back(t.to_json());
    return js;
  }

  template<typename T>
  std::vector<T> toVector(const json11::Json& arr)
  {
    std::vector<T> ts;
    for(json11::Json j : arr.array_items())
      ts.push_back(j);
    return ts;
  }

//  inline std::vector<double> toDoubleVector(const json11::Json& arr)
//  {
//    std::vector<double> ds;
//    for(json11::Json j : arr.array_items())
//      ds.push_back(j.number_value());
//    return ds;
//  }

//  inline std::vector<int> toIntVector(const json11::Json& arr)
//  {
//    std::vector<int> is;
//    for(json11::Json j : arr.array_items())
//      is.push_back(j.int_value());
//    return is;
//  }

//  inline std::vector<std::string> toStringVector(const json11::Json& arr)
//  {
//    std::vector<std::string> ss;
//    for(json11::Json j : arr.array_items())
//      ss.push_back(j.string_value());
//    return ss;
//  }

  //---------------------------------------------------------------------------------------

  void set_double_vectorD(std::vector<double>& var,
                          const json11::Json& j,
                          const std::string& key,
                          const std::vector<double>& def,
                          double defaultValue = 0.0,
                          bool useDefault = true);

  inline void set_double_vector(std::vector<double>& var,
                                const json11::Json& j,
                                const std::string& key)
  {
    set_double_vectorD(var, j, key, std::vector<double>(), 0.0, false);
  }

  std::vector<double> double_vectorD(const json11::Json& j,
                                     const std::vector<double>& def,
                                     double defaultValue = 0.0);

  inline std::vector<double> double_vector(const json11::Json& j)
  {
    return double_vectorD(j, std::vector<double>());
  }

  std::vector<double> double_vectorD(const json11::Json& j,
                                     const std::string& key,
                                     const std::vector<double>& def,
                                     double defaultValue = 0.0);

  inline std::vector<double> double_vector(const json11::Json& j,
                                           const std::string& key)
  {
    return double_vectorD(j, key, std::vector<double>());
  }

  //-------

  void set_int_vectorD(std::vector<int>& var,
                       const json11::Json& j,
                       const std::string& key,
                       const std::vector<int>& def,
                       int defaultValue = 0,
                       bool useDefault = true);

  inline void set_int_vector(std::vector<int>& var,
                             const json11::Json& j,
                             const std::string& key)
  {
    set_int_vectorD(var, j, key, std::vector<int>(), 0, false);
  }

  std::vector<int> int_vectorD(const json11::Json& j,
                               const std::vector<int>& def,
                               int defaultValue = 0);

  inline std::vector<int> int_vector(const json11::Json& j)
  {
    return int_vectorD(j, std::vector<int>());
  }

  std::vector<int> int_vectorD(const json11::Json& j,
                               const std::string& key,
                               const std::vector<int>& def,
                               int defaultValue = 0);

  inline std::vector<int> int_vector(const json11::Json& j,
                                     const std::string& key)
  {
    return int_vectorD(j, key, std::vector<int>());
  }

  //-------

  void set_bool_vectorD(std::vector<bool>& var,
                        const json11::Json& j,
                        const std::string& key,
                        const std::vector<bool>& def,
                        bool defaultValue = false,
                        bool useDefault = true);

  inline void set_bool_vector(std::vector<bool>& var,
                              const json11::Json& j,
                              const std::string& key)
  {
    set_bool_vectorD(var, j, key, std::vector<bool>(), false, false);
  }

  std::vector<bool> bool_vectorD(const json11::Json& j,
                                 const std::vector<bool>& def,
                                 bool defaultValue = false);

  inline std::vector<bool> bool_vector(const json11::Json& j)
  {
    return bool_vectorD(j, std::vector<bool>());
  }

  std::vector<bool> bool_vectorD(const json11::Json& j,
                                 const std::string& key,
                                 const std::vector<bool>& def,
                                 bool defaultValue = false);

  inline std::vector<bool> bool_vector(const json11::Json& j,
                                       const std::string& key)
  {
    return bool_vectorD(j, key, std::vector<bool>());
  }

  //-------

  void set_string_vectorD(std::vector<std::string>& var,
                          const json11::Json& j,
                          const std::string& key,
                          const std::vector<std::string>& def,
                          const std::string& defaultValue = std::string(),
                          bool useDefault = true);

  inline void set_string_vector(std::vector<std::string>& var,
                                const json11::Json& j,
                                const std::string& key)
  {
    set_string_vectorD(var, j, key, std::vector<std::string>(), "", false);
  }

  std::vector<std::string>
  string_vectorD(const json11::Json& j,
                 const std::vector<std::string>& def,
                 const std::string& defaultValue = std::string());

  inline std::vector<std::string> string_vector(const json11::Json& j)
  {
    return string_vectorD(j, std::vector<std::string>());
  }

  std::vector<std::string>
  string_vectorD(const json11::Json& j,
                 const std::string& key,
                 const std::vector<std::string>& def,
                 const std::string& defaultValue = std::string());

  inline std::vector<std::string> string_vector(const json11::Json& j,
                                                const std::string& key)
  {
    return string_vectorD(j, key, std::vector<std::string>());
  }

  //---------------------------------------------------------------------------------------


  void set_int_valueD(int& var, const json11::Json& j,
                      const std::string& key,
                      int def = 0,
                      bool useDefault = true);

  inline void set_int_value(int& var,
                            const json11::Json& j,
                            const std::string& key)
  {
    set_int_valueD(var, j, key, 0, false);
  }

  int int_valueD(const json11::Json& j, int def);

  inline int int_value(const json11::Json& j){ return int_valueD(j, 0); }

  int int_valueD(const json11::Json& j, const std::string& key, int def);

  inline int int_value(const json11::Json& j, const std::string& key)
  {
    return int_valueD(j, key, 0);
  }

  //-----

  void set_double_valueD(double& var,
                         const json11::Json& j,
                         const std::string& key,
                         double def = 0.0,
                         std::function<double(double)> transf = identity<double>,
                         bool useDefault = true);

  inline void set_double_value(double& var,
                               const json11::Json& j,
                               const std::string& key,
                               std::function<double(double)> transf = identity<double>)
  {
    set_double_valueD(var, j, key, 0.0, transf, false);
  }

  double double_valueD(const json11::Json& j, double def);

  inline double double_value(const json11::Json& j)
  {
    return double_valueD(j, 0.0);
  }

  double double_valueD(const json11::Json& j,
                       const std::string& key,
                       double def);

  inline double double_value(const json11::Json& j, const std::string& key)
  {
    return double_valueD(j, key, 0.0);
  }

  //------

  void set_bool_valueD(bool& var,
                       const json11::Json& j,
                       const std::string& key,
                       bool def = false,
                       bool useDefault = true);

  inline void set_bool_value(bool& var,
                             const json11::Json& j,
                             const std::string& key){
    set_bool_valueD(var, j, key, false, false);
  }

  bool bool_valueD(const json11::Json& j, bool def);

  inline bool bool_value(const json11::Json& j){ return bool_valueD(j, false); }

  bool bool_valueD(const json11::Json& j,
                   const std::string& key,
                   bool def = false);

  inline bool bool_value(const json11::Json& j, const std::string& key)
  {
    return bool_valueD(j, key, false);
  }

  //-------

  void set_string_valueD(std::string& var, 
												 const json11::Json& j,
                         const std::string& key,
                         const std::string& def = std::string(),
                         bool useDefault = true);

  inline void set_string_value(std::string& var,
                               const json11::Json& j,
                               const std::string& key)
  {
    set_string_valueD(var, j, key, "", false);
  }

  std::string string_valueD(const json11::Json& j, const std::string& def);

  inline std::string string_value(const json11::Json& j)
  {
    return string_valueD(j, "");
  }

  std::string string_valueD(const json11::Json& j,
                            const std::string& key,
                            const std::string& def);

  inline std::string string_value(const json11::Json& j, const std::string& key)
  {
    return string_valueD(j, key, "");
  }

  //-------

  Tools::Date iso_date_value(const json11::Json& j, 
														 const std::string& key);

  void set_iso_date_value(Tools::Date& var,
                          const json11::Json& j,
                          const std::string& key);

  template<class C>
  void set_shared_ptr_value(std::shared_ptr<C>& var,
                            const json11::Json& j,
                            const std::string& key)
  {
    std::string err;
    if(j.has_shape({{key, json11::Json::OBJECT}}, err))
      var = std::make_shared<C>(j[key]);
		if(!err.empty())
			std::cerr << "Error @ Tools::set_shared_ptr_value: " << err << std::endl;
  }

  template<class C>
  void set_ptr_value(C& var,
                     const json11::Json& j,
                     const std::string& key)
  {
    std::string err;
    if(j.has_shape({{key, json11::Json::OBJECT}}, err))
      var = new C(j[key]);
		if(!err.empty())
			std::cerr << "Error @ Tools::set_ptr_value: " << err << std::endl;
  }

  template<class C>
  void set_value_obj_value(C& var,
                           const json11::Json& j,
                           const std::string& key)
  {
    std::string err;
    if(j.has_shape({{key, json11::Json::OBJECT}}, err))
      var = C(j[key]);
		if(!err.empty())
			std::cerr << "Error @ Tools::set_value_obj_value: " << err << std::endl;
  }

  //---------------------------------------------------------------------------------------

  inline json11::Json::array
  cljson11Collection(std::string type,
                     std::initializer_list<json11::Json> l
                     = std::initializer_list<json11::Json>())
  {
    json11::Json::array a{ type };
    for(auto j : l) a.push_back(j);
    return a; //a.insert(a.end(), l.begin(), l.end());
  }

  inline json11::Json::array
  cljson11Vector(std::initializer_list<json11::Json> l
                 = std::initializer_list<json11::Json>())
  {
    return cljson11Collection("v", l);
  }

  inline json11::Json::array
  cljson11Map(std::initializer_list<json11::Json> l
              = std::initializer_list<json11::Json>())
  {
    return cljson11Collection("m", l);
  }

  inline json11::Json::array
  cljson11Set(std::initializer_list<json11::Json> l
              = std::initializer_list<json11::Json>())
  {
    return cljson11Collection("s", l);
  }

  inline json11::Json::array
  cljson11List(std::initializer_list<json11::Json> l
               = std::initializer_list<json11::Json>())
  {
    return cljson11Collection("l", l);
  }

  inline json11::Json cljson11Keyword(const std::string& kw)
  {
    return json11::Json::array { "k", kw };
  }

  inline json11::Json cljson11Symbol(const std::string& s)
  {
    return json11::Json::array { "y", s };
  }

  inline json11::Json cljson11Date(const Tools::Date& d)
  {
    return json11::Json::array {
      "inst",
      d.toIsoDateString() + "T00:00:00.000-00:00" };
  }

}

#endif
