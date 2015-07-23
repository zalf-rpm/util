#include <sstream>

#include "zmq-helper.h"

using namespace Tools;
using namespace std;
using namespace json11;

Msg Tools::receiveMsg(zmq::socket_t& pullSocket, bool nonBlockingMode)
{
  zmq::message_t message;
  if(pullSocket.recv(&message, nonBlockingMode ? ZMQ_NOBLOCK : 0))
  {
    std::string strMsg(static_cast<char*>(message.data()), message.size());

//    cout << "receiveMsg: " << strMsg << endl;

    //    string strMsg = s_recv(pullSocket);
    string err;
    const Json& jsonMsg = Json::parse(strMsg, err);
    return Msg{jsonMsg, err, true};
  }
  return Msg{Json(), "", false};
}


//QJsonValue Tools::cljsonUuid(const QUuid& uuid)
//{
//  return json11::Json::array { "uuid" << uuid.toString().remove(QRegularExpression("[\{\}]"));
//}


//QVariant Tools::encodeString(QString s)
//{
//  if(s.startsWith(":"))
//    return QVariantList() << "k" << s.mid(1);
//  else if(s.startsWith("'"))
//    return QVariantList() << "y" << s.mid(1);

//  return QVariant(s);
//}

//QJsonArray Tools::encodeCljson(QVariantList vs)
//{
//  QVariantList l;
//  l.append("v");
//  foreach(QVariant v, vs)
//  {
//    QVariant var = encodeCljsonFormat(v);
//    qDebug() << "var: " << var;
//    l.append(var);
//  }

//  return QJsonArray::fromVariantList(l);
//}

//QVariant Tools::encodeCljsonFormat(QVariant v)
//{
//  switch(int(v.type()))
//  {
//  case QMetaType::QVariantMap:
//  {
//    QVariantList l;
//    l.append("m");
//    QVariantMap m = v.toMap();
//    foreach(QString key, m.keys())
//    {
//      l.append(encodeCljsonFormat(encodeString(key)));
//      l.append(encodeCljsonFormat(m[key]));
//    }
//    return l;
//    break;
//  }
//  case QMetaType::QVariantList:
//  {
//    QVariantList l;
//    QVariantList vs = v.toList();
//    l.append("v");
//    foreach(QVariant v, vs)
//    {
//      l.append(encodeCljsonFormat(v));
//    }
//    return l;
//    break;
//  }
//  case QMetaType::Bool:
//  case QMetaType::Double:
//    return v;
//    break;
//  case QMetaType::QDate:
//    return QVariantList() << "inst" << (v.toDate().toString("yyyy-MM-dd") + "T00:00:00.000-00:00");
//    break;
//  case QMetaType::QUuid:
//    return QVariantList() << "uuid" << v.toUuid().toString().remove(QRegularExpression("[\{\}]"));
//    break;
//  case QMetaType::QString:
//    return encodeString(v.toString());
//    break;
//  default: ;
//  }

//  return v;
//}

//QVariant Tools::decodeCljson(QJsonValue v)
//{
//  return v.isArray() ? decodeCljsonTagged(v.toArray()) : v.toVariant();
//}

//QVariant Tools::decodeCljsonTagged(QJsonArray a)
//{
//  QString tag = a.at(0).toString("");
//  if(tag == "v")
//  {
//    QVariantList l;
//    for(int i = 1, size = a.size(); i < size; i++)
//      l.append(decodeCljson(a.at(i)));
//    return l;
//  }
//  else if(tag == "l")
//  {
//    QVariantList l;
//    for(int i = 1, size = a.size(); i < size; i++)
//      l.append(decodeCljson(a.at(i)));
//    return l;
//  }
//  else if(tag == "m")
//  {
//    QVariantMap m;
//    for(int i = 1, size = a.size(); i < size; i += 2)
//    {
//      QVariant key = decodeCljson(a.at(i));
//      QString skey;
//      switch(key.type())
//      {
//      case QVariant::Map:
//      case QVariant::List:
//        skey = QString(key.toJsonDocument().toJson()); break;
//      case QVariant::Bool:
//        skey = QString("%1").arg(key.toBool()); break;
//      case QVariant::Double:
//        skey = QString("%1").arg(key.toDouble()); break;
//      case QVariant::Date:
//        skey = key.toDate().toString(); break;
//      case QVariant::String:
//        skey = key.toString(); break;
//      default: ;
//      }

//      m[skey] = decodeCljson(a.at(i+1));
//    }
//    return m;
//  }
//  else if(tag == "s")
//  {
//    QVariantList l;
//    for(int i = 1, size = a.size(); i < size; i++)
//      l.append(decodeCljson(a.at(i)));
//    return l;
//  }
//  else if(tag == "k")
//    return QString(":") + a.at(1).toString();
//  else if(tag == "y")
//    return QString("'") + a.at(1).toString();
//  else if(tag == "z")
//    return QVariant("z tag is currently unimplemented");
//  else if(tag == "uuid")
//    return QUuid(a.at(1).toString());
//  else if(tag == "inst")
//    return QVariant(QDate::fromString(a.at(1).toString().mid(0, 10), "yyyy-MM-dd"));

//  return QVariant(tag + " tag is currently unimplemented");
//}
