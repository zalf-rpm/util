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
    for(Collection::value_type t : col)
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

  inline std::vector<double> toDoubleVector(const json11::Json& arr)
  {
    std::vector<double> ds;
    for(json11::Json j : arr.array_items())
      ds.push_back(j.number_value());
    return ds;
  }

  inline std::vector<int> toIntVector(const json11::Json& arr)
  {
    std::vector<int> is;
    for(json11::Json j : arr.array_items())
      is.push_back(j.int_value());
    return is;
  }

  inline std::vector<std::string> toStringVector(const json11::Json& arr)
  {
    std::vector<std::string> ss;
    for(json11::Json j : arr.array_items())
      ss.push_back(j.string_value());
    return ss;
  }

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
