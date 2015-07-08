#ifndef ZMQ_HELPER_H_
#define ZMQ_HELPER_H_

#include <string>
#include <functional>
#include <initializer_list>

#include "json11/json11.hpp"

#include "tools/date.h"

//#include "tools/helper.h"

namespace Tools
{
  inline json11::Json::array cljsonCollection(std::string type, std::initializer_list<json11::Json> l = std::initializer_list<json11::Json>())
  {
    json11::Json::array a{ type };
    return a.insert(a.end(), l.begin(), l.end());
  }

  inline json11::Json::array cljsonVector(std::initializer_list<json11::Json> l = std::initializer_list<json11::Json>())
  {
    return cljsonCollection("v", l);
  }

  inline json11::Json::array cljsonMap(std::initializer_list<json11::Json> l = std::initializer_list<json11::Json>())
  {
    return cljsonCollection("m", l);
  }

  inline json11::Json::array cljsonSet(std::initializer_list<json11::Json> l = std::initializer_list<json11::Json>())
  {
    return cljsonCollection("s", l);
  }

  inline json11::Json::array cljsonList() { return cljsonCollection("l", l); }

  inline json11::Json cljsonKeyword(const std::string& kw) { return json11::Json::array { "k", kw }; }

  inline json11::Json cljsonSymbol(const std::string& s) { return json11::Json::array { "y", s }; }

  inline json11::Json cljsonDate(const Tools::Date& d) { return json11::Json::array { "inst", d.toIsoDateString() + "T00:00:00.000-00:00" }; }

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
