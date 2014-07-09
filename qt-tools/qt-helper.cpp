#include <QtCore/QtCore>
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
