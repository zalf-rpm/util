#include <QtCore>
#include <sstream>

#include "qt-helper.h"
#include "tools/agricultural-helper.h"

using namespace Tools;
using namespace std;
using namespace boost;

namespace
{
	string sttCode2stt(double value){ return sttFromCode(int(value)); }
	
	string dgm(double value)
	{
		ostringstream s;
		s << value << " m ü.NN";
		return s.str();
	}
}

std::function<string (double)>
Tools::gridValueTransformFunction4datasetName(const string& datasetName){

	if(datasetName == "stt")
		return std::function<string(double)>(sttCode2stt);
	if(datasetName == "dgm")
		return std::function<string(double)>(dgm);

	return std::function<string(double)>();
}


string JaNein::toString() const {
	return value ? QObject::tr("ja").toStdString().c_str()
							 : QObject::tr("nein").toStdString().c_str();
}

const bool JaNein::JA = true;
const bool JaNein::NEIN = false;

//------------------------------------------------------------------------------

GermanBoolEditor::GermanBoolEditor(QWidget* parent) : QComboBox(parent){
	addItem(JaNein::ja().toString().c_str(), JaNein::JA);
	addItem(JaNein::nein().toString().c_str(), JaNein::NEIN);
}

JaNein GermanBoolEditor::jaNein() const {
	return JaNein(itemData(currentIndex()).toBool());
}

void GermanBoolEditor::setJaNein(JaNein jn){
	setCurrentIndex(findData(jn.value));
}

//------------------------------------------------------------------------------

QPair<QVector<double>, QVector<double> >
Tools::createMinMaxPolygon(const QVector<double>& xs,
													 const QVector<double>& mins,
													 const QVector<double>& maxs)
{
	//create a curve comprised of the min and max curves which form
	//an almost closed polygon
	QVector<double> revMinYValues(maxs.size());
	QVector<double> revMinXValues(xs.size());
	std::reverse_copy(mins.begin(), mins.end(), revMinYValues.begin());
	std::reverse_copy(xs.begin(), xs.end(), revMinXValues.begin());
	QVector<double> yValues(maxs);
	yValues += revMinYValues;
	QVector<double> xValues(xs);
	xValues += revMinXValues;

	return qMakePair(xValues, yValues);
}

QPair<QVector<double>, QVector<double> >
Tools::createStandardDeviationPolygon(const QVector<double>& xs,
																			const QVector<double>& ys,
																			const QVector<double>& sigmas)
{
	//subtract sigmas from values for minimum curve
	QVector<double> revMinYValues1(ys.size());
	std::transform(ys.begin(), ys.end(), sigmas.begin(),
								 revMinYValues1.begin(), std::minus<double>());

	//reverse the minimum curve and the according x-values
	QVector<double> revMinXValues(xs.size());
	QVector<double> revMinYValues(ys.size());
	std::reverse_copy(revMinYValues1.begin(), revMinYValues1.end(),
										revMinYValues.begin());
	std::reverse_copy(xs.begin(), xs.end(), revMinXValues.begin());

	//add sigmas to values for maximum curve and add the reversed min to end
	QVector<double> yValues(ys.size());
	std::transform(ys.begin(), ys.end(), sigmas.begin(),
								 yValues.begin(), std::plus<double>());
	yValues += revMinYValues;

	//create x-values
	QVector<double> xValues(xs);
	xValues += revMinXValues;

	return qMakePair(xValues, yValues);
}

//------------------------------------------------------------------------------

QVariant Tools::executeJs(QWebFrame* webFrame, QString jsc, QStringList vns,
                          QVariantList ivs, QString jsBridgeObjectName)
{
  QObject* jsBridge = new QObject;

  for(int i = 0; i < vns.size(); i++)
  {
    QString vn = vns.at(i);
    if(ivs.size() > i)
    {
      QVariant v = ivs.at(i);
      //qDebug() << "v: " << v;
      switch(v.type())
      {
      case QVariant::Map:
        jsBridge->setProperty(vn.toStdString().c_str(), v.toMap());
        break;
      case QVariant::List:
        jsBridge->setProperty(vn.toStdString().c_str(), v.toList());
        break;
      default:
        jsBridge->setProperty(vn.toStdString().c_str(), v);
      }
    }
    else
    {
      jsBridge->setProperty(vn.toStdString().c_str(), QVariant(-1));
    }
  }

  //qDebug() << "jsc: " << jsc;

  webFrame->addToJavaScriptWindowObject(jsBridgeObjectName, jsBridge);
  QVariant res = webFrame->evaluateJavaScript(jsc);

  return res;
}

QVariant Tools::executeJs(QWebFrame* webFrame, const QString& jsc, const QString& varName,
                          const QVariant& value,
                          const QString& jsBridgeObjectName)
{
  QObject* jsBridge = new QObject;

  switch(value.type())
  {
    case QVariant::Map:
      jsBridge->setProperty(varName.toStdString().c_str(), value.toMap());
      break;
    case QVariant::List:
      jsBridge->setProperty(varName.toStdString().c_str(), value.toList());
      break;
    default:
      jsBridge->setProperty(varName.toStdString().c_str(), value);
  }

  webFrame->addToJavaScriptWindowObject(jsBridgeObjectName, jsBridge);
  return webFrame->evaluateJavaScript(jsc);
}

//---------------------------------------------------------------------------------------------

QJsonValue Tools::cljsonKeyword(const QString& kw)
{
  return QJsonArray() << "k" << kw;
}

QJsonValue Tools::cljsonSymbol(const QString& s)
{
  return QJsonArray() << "y" << s;
}

QJsonValue Tools::cljsonDate(const QDate& d)
{
  return QJsonArray() << "inst" << (d.toString("yyyy-MM-dd") + "T00:00:00.000-00:00");
}

QJsonValue Tools::cljsonUuid(const QUuid& uuid)
{
  return QJsonArray() << "uuid" << uuid.toString().remove(QRegularExpression("[\{\}]"));
}


QVariant Tools::encodeString(QString s)
{
  if(s.startsWith(":"))
    return QVariantList() << "k" << s.mid(1);
  else if(s.startsWith("'"))
    return QVariantList() << "y" << s.mid(1);

  return QVariant(s);
}

QJsonArray Tools::encodeCljson(QVariantList vs)
{
  QVariantList l;
  l.append("v");
  foreach(QVariant v, vs)
  {
    QVariant var = encodeCljsonFormat(v);
    qDebug() << "var: " << var;
    l.append(var);
  }

  return QJsonArray::fromVariantList(l);
}

QVariant Tools::encodeCljsonFormat(QVariant v)
{
  switch(int(v.type()))
  {
  case QMetaType::QVariantMap:
  {
    QVariantList l;
    l.append("m");
    QVariantMap m = v.toMap();
    foreach(QString key, m.keys())
    {
      l.append(encodeCljsonFormat(encodeString(key)));
      l.append(encodeCljsonFormat(m[key]));
    }
    return l;
    break;
  }
  case QMetaType::QVariantList:
  {
    QVariantList l;
    QVariantList vs = v.toList();
    l.append("v");
    foreach(QVariant v, vs)
    {
      l.append(encodeCljsonFormat(v));
    }
    return l;
    break;
  }
  case QMetaType::Bool:
  case QMetaType::Double:
    return v;
    break;
  case QMetaType::QDate:
    return QVariantList() << "inst" << (v.toDate().toString("yyyy-MM-dd") + "T00:00:00.000-00:00");
    break;
  case QMetaType::QUuid:
    return QVariantList() << "uuid" << v.toUuid().toString().remove(QRegularExpression("[\{\}]"));
    break;
  case QMetaType::QString:
    return encodeString(v.toString());
    break;
  default: ;
  }

  return v;
}

QVariant Tools::decodeCljson(QJsonValue v)
{
  return v.isArray() ? decodeCljsonTagged(v.toArray()) : v.toVariant();
}

QVariant Tools::decodeCljsonTagged(QJsonArray a)
{
  QString tag = a.at(0).toString("");
  if(tag == "v")
  {
    QVariantList l;
    for(int i = 1, size = a.size(); i < size; i++)
      l.append(decodeCljson(a.at(i)));
    return l;
  }
  else if(tag == "l")
  {
    QVariantList l;
    for(int i = 1, size = a.size(); i < size; i++)
      l.append(decodeCljson(a.at(i)));
    return l;
  }
  else if(tag == "m")
  {
    QVariantMap m;
    for(int i = 1, size = a.size(); i < size; i += 2)
    {
      QVariant key = decodeCljson(a.at(i));
      QString skey;
      switch(key.type())
      {
      case QVariant::Map:
      case QVariant::List:
        skey = QString(key.toJsonDocument().toJson()); break;
      case QVariant::Bool:
        skey = QString("%1").arg(key.toBool()); break;
      case QVariant::Double:
        skey = QString("%1").arg(key.toDouble()); break;
      case QVariant::Date:
        skey = key.toDate().toString(); break;
      case QVariant::String:
        skey = key.toString(); break;
      default: ;
      }

      m[skey] = decodeCljson(a.at(i+1));
    }
    return m;
  }
  else if(tag == "s")
  {
    QVariantList l;
    for(int i = 1, size = a.size(); i < size; i++)
      l.append(decodeCljson(a.at(i)));
    return l;
  }
  else if(tag == "k")
    return QString(":") + a.at(1).toString();
  else if(tag == "y")
    return QString("'") + a.at(1).toString();
  else if(tag == "z")
    return QVariant("z tag is currently unimplemented");
  else if(tag == "uuid")
    return QUuid(a.at(1).toString());
  else if(tag == "inst")
    return QVariant(QDate::fromString(a.at(1).toString().mid(0, 10), "yyyy-MM-dd"));

  return QVariant(tag + " tag is currently unimplemented");
}
