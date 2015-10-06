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

  std::vector<double> double_vectorD(const json11::Json& j, double def);

  inline std::vector<double> double_vector(const json11::Json& j){ return double_vectorD(j, 0.0); }

  std::vector<double> double_vectorD(const json11::Json& j, const std::string& key, double def);

  inline std::vector<double> double_vector(const json11::Json& j, const std::string& key){ return double_vectorD(j, key, 0.0); }

  //-------

  std::vector<int> int_vectorD(const json11::Json& j, int def);

  inline std::vector<int> int_vector(const json11::Json& j){ return int_vectorD(j, 0); }

  std::vector<int> int_vectorD(const json11::Json& j, const std::string& key, int def);

  inline std::vector<int> int_vector(const json11::Json& j, const std::string& key){ return int_vectorD(j, key, 0); }

  //-------

  std::vector<bool> bool_vectorD(const json11::Json& j, bool def);

  inline std::vector<bool> bool_vector(const json11::Json& j){ return bool_vectorD(j, false); }

  std::vector<bool> bool_vectorD(const json11::Json& j, const std::string& key, bool def);

  inline std::vector<bool> bool_vector(const json11::Json& j, const std::string& key){ return bool_vectorD(j, key, false); }

  //-------

  std::vector<std::string> string_vectorD(const json11::Json& j, const std::string& def);

  inline std::vector<std::string> string_vector(const json11::Json& j){ return string_vectorD(j, std::string()); }

  std::vector<std::string> string_vectorD(const json11::Json& j, const std::string& key, const std::string& def);

  inline std::vector<std::string> string_vector(const json11::Json& j, const std::string& key){ return string_vectorD(j, key, std::string()); }

  //---------------------------------------------------------------------------------------

  void set_int_value(int& var, const json11::Json& j, const std::string& key, int def = 0);

  int int_valueD(const json11::Json& j, int def);

  inline int int_value(const json11::Json& j){ return int_valueD(j, 0); }

  int int_valueD(const json11::Json& j, const std::string& key, int def);

  inline int int_value(const json11::Json& j, const std::string& key){ return int_valueD(j, key, 0); }

  //-----

  void set_double_value(double& var, const json11::Json& j, const std::string& key, double def = 0.0);

  double double_valueD(const json11::Json& j, double def);

  inline double double_value(const json11::Json& j){ return double_valueD(j, 0.0); }

  double double_valueD(const json11::Json& j, const std::string& key, double def);

  inline double double_value(const json11::Json& j, const std::string& key){ return double_valueD(j, key, 0.0); }

  //------

  void set_bool_value(bool& var, const json11::Json& j, const std::string& key, bool def = false);

  bool bool_valueD(const json11::Json& j, bool def);

  inline bool bool_value(const json11::Json& j){ return bool_valueD(j, false); }

  bool bool_valueD(const json11::Json& j, const std::string& key, bool def = false);

  inline bool bool_value(const json11::Json& j, const std::string& key){ return bool_valueD(j, key, false); }

  //-------

  void set_string_value(std::string& var, const json11::Json& j, const std::string& key, const std::string& def = std::string());

  std::string string_valueD(const json11::Json& j, const std::string& def);

  inline std::string string_value(const json11::Json& j){ return string_valueD(j, ""); }

  std::string string_valueD(const json11::Json& j, const std::string& key, const std::string& def);

  inline std::string string_value(const json11::Json& j, const std::string& key){ return string_valueD(j, key, ""); }

  //---------------------------------------------------------------------------------------

  inline json11::Json::array cljson11Collection(std::string type, std::initializer_list<json11::Json> l = std::initializer_list<json11::Json>())
  {
    json11::Json::array a{ type };
    for(auto j : l) a.push_back(j);
    return a; //a.insert(a.end(), l.begin(), l.end());
  }

  inline json11::Json::array cljson11Vector(std::initializer_list<json11::Json> l = std::initializer_list<json11::Json>())
  {
    return cljson11Collection("v", l);
  }

  inline json11::Json::array cljson11Map(std::initializer_list<json11::Json> l = std::initializer_list<json11::Json>())
  {
    return cljson11Collection("m", l);
  }

  inline json11::Json::array cljson11Set(std::initializer_list<json11::Json> l = std::initializer_list<json11::Json>())
  {
    return cljson11Collection("s", l);
  }

  inline json11::Json::array cljson11List(std::initializer_list<json11::Json> l = std::initializer_list<json11::Json>())
  {
    return cljson11Collection("l", l);
  }

  inline json11::Json cljson11Keyword(const std::string& kw) { return json11::Json::array { "k", kw }; }

  inline json11::Json cljson11Symbol(const std::string& s) { return json11::Json::array { "y", s }; }

  inline json11::Json cljson11Date(const Tools::Date& d) { return json11::Json::array { "inst", d.toIsoDateString() + "T00:00:00.000-00:00" }; }

  //  json11::Json cljsonUuid(const QUuid& uuid);

//  QVariant encodeString(QString s);

//  QVariant encodeCljsonFormat(QVariant v);

//  QJsonArray encodeCljson(QVariantList v);

//  inline QByteArray encodeToCljsonByteArray(QVariantList v)
//  {
//    return QJsonDocument(encodeCljson(v)).toJson();
//  }

//  QVariant decodeCljson(QJsonValue v);

//  QVariant decodeCljsonTagged(QJsonArray a);
}

#endif
