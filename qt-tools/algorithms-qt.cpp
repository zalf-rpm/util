//#include <gsl/gsl_fit.h>
#include <cstdio>
#include <QtCore/QDebug>
#include <cmath>
#include <cstdlib>
#include <numeric>
#include <algorithm>

#include "algorithms-qt.h"
#include "tools/algorithms.h"

using namespace std;
using namespace Tools;

/*
QPair<double, double> Tools::qtMinMax(const QVector<double>& ys){
	const std::pair<double, double>& mm = minMax(ys.toStdVector());
	return qMakePair(mm.first, mm.second);
}
*/

/*
double Tools::standardDeviation(const QVector<double>& xis, double xavg, int n){
	double sum = 0;
	foreach(double xi, xis){
		sum += (xi - xavg) * (xi - xavg);
	}
	return sqrt(sum / (n-1));
}
*/

/*
QPair<double, double> Tools::standardDeviation(const QVector<double>& xis){
	int n = xis.size();
	double avg = accumulate(xis.begin(), xis.end(), double(0)) / double(n);
	return QPair<double, double>(standardDeviation(xis, avg, n), avg);
}
*/

/*
QVector<double> Tools::linearRegressionGSL(const QVector<double>& xs, const QVector<double>& ys,
	int n){
	double c0, c1, cov00, cov01, cov11, chisq;

  gsl_fit_linear(xs.data(), 1, ys.data(), 1, n, &c0, &c1, &cov00, &cov01, &cov11, &chisq);

  printf ("# best fit: Y = %g + %g X\n", c0, c1);
 	printf ("# covariance matrix:\n");
 	printf ("# [ %g, %g\n#   %g, %g]\n", cov00, cov01, cov01, cov11);
 	printf ("# chisq = %g\n", chisq);

  //for (int i = 0; i < n; i++)
  //	printf ("data: %g %g %g\n", x[i], y[i], 1/sqrt(w[i]));

 	//printf ("\n");

 	QVector<double> yis(n);
  for(int i = 0; i < n; i++){
		double yf_err;

    gsl_fit_linear_est (xs.at(i), c0, c1, cov00, cov01, cov11, &(yis[i]), &yf_err);

    //printf ("fit: %g %g\n", xs[i], yis[i]);
 		//printf ("hi : %g %g\n", xf, yf + yf_err);
   	//printf ("lo : %g %g\n", xf, yf - yf_err);
	}

	return yis;
}
//*/
/*
QVector<double> Tools::linearRegression(const QVector<double>& xs,
                                        const QVector<double>& ys){
	Q_ASSERT(xs.size() <= ys.size());

	int n = xs.size();
	double b_r, a_r;
	double sumXY, sumX, sumY, sumXSq;
	b_r = a_r = sumXY = sumX = sumY = sumXSq = 0;

	for(int i = 0; i < n; i++){
		double x_i = xs.at(i);
		double y_i = ys.at(i);
		sumXY += x_i * y_i;
		sumX += x_i;
		sumY += y_i;
		sumXSq += x_i * x_i;
	}

	b_r = (sumXY - ((sumX * sumY) / double(n))) / (sumXSq - ((sumX * sumX) / double(n)));
	a_r = (sumY - (b_r * sumX)) / (double(n));

	QVector<double> ys_r(n);
	for(int i = 0; i < n; i++){
		ys_r[i] = a_r + (b_r * xs.at(i));
	}

	return ys_r;
}

QVector<double> Tools::cumulativeSum(const QVector<double>& ys, int n){
	double acc = 0;
	QVector<double> yis(n);
	for(int i = 0; i < n; i++)
		yis[i] = (acc += ys[i]);

	return yis;
}

QVector<double> Tools::expGlidingAverage(const QVector<double>& ys, double alpha){
	QVector<double> yas(ys.size());
	double ya = yas[0] = ys.first();
	for(int i = 1; i < ys.size(); i++){
		ya = yas[i] = (alpha * ys.at(i)) + ((1-alpha) * ya);
	}

	return yas;
}



QVector<double> Tools::simpleGlidingAverage(const QVector<double>& ys, int n){
	int lr = int(double(isEven(n) ? n : n-1) / 2.0);  //left right
	n = (2*lr) + 1;

	if(ys.size() < n)
			return QVector<double>();

	QVector<double> yas(ys.size() - (2*lr));
	double sum = 0;
	for(int i = 0; i < n; i++){
		sum += ys.at(i);
	}
	yas[0] = sum / double(n);
	for(int i = 0; i < ys.size() - n; i++){
		sum = sum - ys.at(i) + ys.at(i+n);
		yas[i+1] = sum / double(n);
	}

	return yas;
}
*/

/*
 * create histogram data with the amount of given classes n
 */
QList<QVector<double> > Tools::hist(const QVector<double>& ys, int n){
	QPair<double, double> mima = qtMinMax(ys);
	double minY = mima.first;
	double shift = -1*minY;
	double maxY = mima.second;
	double step = (maxY - minY) / n;
	//qDebug() << "minY: " << minY << " shift: " << shift << " maxY: " << maxY << " step: " << step;

	QVector<double> xs(n);
	for(int i = 0; i < n; i++)
		xs[i] = minY + (i * step + (step / 2));
	//qDebug() << "xs: " << xs;

	QVector<double> borders(n+1);
	for(int i = 0; i < n+1; i++)
		borders[i] = minY + (i * step);
	//qDebug() << "borders: " << borders;

	QVector<double> classes(n+1);
	foreach(double y, ys){
		//qDebug() << "(int)ceil((" << shift << " + " << y << ") /" << step << ")=" <<
		//	(int(ceil((shift + y)/step)));
		classes[int(ceil((shift + y)/step))]++;
	}
	//qDebug() << "classes: " << classes;
	classes[1] += classes.at(0);
	classes.remove(0); //lowest border actually belongs to class above (0 value)
	//qDebug() << "classes: " << classes;

	return QList<QVector<double> >() << borders << xs << classes;
}

QVector<int> Tools::periods(const QVector<double>& ys, bool (*f)(double), int nPlus1){
	QVector<int> cs(nPlus1);
	int c = 0;
	bool wasF = false;
	foreach(double y, ys){
		if(f(y)){
			c++;
			wasF = true;
		}	else if(wasF){
			cs[(c >= nPlus1 ? nPlus1 : c) - 1]++;
			c = 0;
			wasF = false;
		}
	}
	//count possibly last elements
	if(wasF)
		cs[(c >= nPlus1 ? nPlus1 : c) - 1]++;

	return cs;
}

QVector<int> Tools::periodsByNeighbour(const QVector<double>& xs, int nPlus1){
	QVector<int> cs(nPlus1);
	int c = 1;
	for(int k = 0; k < xs.size() - 1; k++){
		if(xs.at(k+1) - xs.at(k) == 1)
			c++;
		else {
//			qDebug() << "c@xs: " << c << "@" << xs.at(k == 0 ? k : k-1);
			cs[(c >= nPlus1 ? nPlus1 : c) - 1]++;
			c = 1;
		}
	}
	//take care of last element
	if(!xs.isEmpty())
		cs[(c >= nPlus1 ? nPlus1 : c) - 1]++;

	return cs;
}

QVector<QVector<double> > Tools::distributeColumns(int n, int upTo, int widthInPercent){
	Q_ASSERT(n > 0);
	QVector<QVector<double> > columns(n);

	div_t t = div(n, 2);
	bool even = t.rem == 0;
	double delta = floor(((widthInPercent / 100.0) / (2 * n)) * 1000.0) / 1000.0; //round down to three digits after ,
	int sn = even ? n : n - 1; //start n
	int evenStart = (2 * ((sn / 2) - 1)) + 1;
	double start = even ? evenStart * delta : (evenStart + 1) * delta; //start distance for delta from main tick
	//qDebug() << "delta: " << delta << " sn: " << sn << " evenStart: " << evenStart << " start: " << start;
	for(int i = 1; i <= upTo; i++){
		double value = i - start;
		//qDebug() << " v: " << value;
		int col = 0;
		(columns[col++]).append(value); //at least one value is always there
		for(int k = 1; k < n; k++){
			value += 2 * delta;
			//qDebug() << " lv: " << value;
			(columns[col++]).append(value);
		}
	}

	return columns;
}

QVector<double> Tools::distributeColumnsJoined(int n, int upTo, int widthInPercent){
	QVector<double> xs;

	QVector<QVector<double> > columns = distributeColumns(n, upTo, widthInPercent);
	for(int i = 0; i < upTo; i++)
		foreach(QVector<double> c, columns)
			xs.append(c.at(i));

	return xs;
}

/*
int Tools::findThermalVegetationalSeasonStart(const QVector<double>& ts)
{
	double tresh = 5;
	int length = 30;
  for(int i = 0; i < ts.size(); i++)
  {
    if(ts.at(i) > tresh)
    {
			double sum = 0;
			for(int k = i+1; k < ts.size() && k < i+length+1; k++)
				sum += ts.at(k) - tresh;
			if(sum > 0)
				return i;
		}
	}
  return 1;//-1;
}

int Tools::findThermalVegetationalSeasonEnd(const QVector<double>& ts)
{
	double tresh = 5;
  for(int i = 0; i < ts.size(); i++)
  {
    if(ts.at(i) < tresh)
    {
			double sum = 0;
			for(int k = i+1; k < ts.size(); k++)
				sum += ts.at(k) - tresh;
			if(sum < 0)
				return i;
		}
	}
  return 365;//-1;
}
*/

/**
 * calculate the average of the values regarding the unique values in the groupCol vector
 */
QPair<QVector<double>, QVector<double> >
Tools::average(const QVector<double>& groupCol, const QVector<double>& values){
	QMap<double, QPair<int, double> > group2value;
	for(int i = 0; i < groupCol.size(); i++){
		double gc = groupCol.at(i);
		const QPair<int, double>& v = group2value.value(gc, QPair<int, double>(0, 0));
		group2value.insert(gc, QPair<int, double>(v.first + 1, v.second + values.at(i)));
	}

	QVector<double> resG;
	QVector<double> resV;
	foreach(double d, group2value.keys()){
		resG.append(d);
		QPair<int, double> p = group2value.value(d);
		resV.append(p.second / double(p.first));
	}
	return QPair<QVector<double>, QVector<double> >(resG, resV);
}

/**
 * calculate the sum of the values regarding the unique values in the
 * groupCol vector
 */
QPair<QVector<double>, QVector<double> >
Tools::sum(const QVector<double>& groupCol, const QVector<double>& values){
	QMap<double, double> group2value;
	for(int i = 0; i < groupCol.size(); i++){
		double gc = groupCol.at(i);
		double v = group2value.value(gc, 0);
		group2value.insert(gc, v + values.at(i));
	}

	QVector<double> resG;
	QVector<double> resV;
	foreach(double d, group2value.keys()){
		resG.append(d);
		resV.append(group2value.value(d));
	}
	return QPair<QVector<double>, QVector<double> >(resG, resV);
}

/**
 * get the min of the values regarding the unique values in the groupCol vector
 */
QPair<QVector<double>, QVector<double> >
Tools::min(const QVector<double>& groupCol, const QVector<double>& values){
	QMap<double, double> group2value;
	for(int i = 0; i < groupCol.size(); i++)
		group2value.insertMulti(groupCol.at(i), values.at(i));

	QVector<double> resG;
	QVector<double> resV;
	foreach(double d, group2value.uniqueKeys()){
		resG.append(d);
		const QList<double>& vs = group2value.values(d);
		resV.append(*(std::min_element(vs.begin(), vs.end())));
	}
	return QPair<QVector<double>, QVector<double> >(resG, resV);
}

/**
 * get the max of the values regarding the unique values in the groupCol vector
 */
QPair<QVector<double>, QVector<double> >
Tools::max(const QVector<double>& groupCol, const QVector<double>& values){
	QMap<double, double> group2value;
	for(int i = 0; i < groupCol.size(); i++)
		group2value.insertMulti(groupCol.at(i), values.at(i));

	QVector<double> resG;
	QVector<double> resV;
	foreach(double d, group2value.uniqueKeys()){
		resG.append(d);
		const QList<double>& vs = group2value.values(d);
		resV.append(*(std::max_element(vs.begin(), vs.end())));
	}
	return QPair<QVector<double>, QVector<double> >(resG, resV);
}

/**
 * - calculate the average for the given month range for the given values
 */
QPair<QVector<double>, QVector<double> >
Tools::monthlyAverage(int from, int to, const QVector<double>& monthCol,
                      const QVector<double>& yearCol,
                      const QVector<double>& values){
	QVector<double> resY;
	QVector<double> resV;
	double avgCount = 0;
	double sum = 0;
	int currentYear = int(yearCol.first());
	bool inArea = from > to || from == 1; //tells whether we are in the area
	bool onNextMonthLeaveArea = from == to && inArea; //tells when to leave the area
	int lastMonth = 1;

	for(int i = 0; i < monthCol.size(); i++){
		int month = int(monthCol.at(i));
		int year = int(yearCol.at(i));

		if(onNextMonthLeaveArea && month != lastMonth){
			resY.append(currentYear);
			if(avgCount > 0){
				//if(from > to && int(yearCol.first()) == currentYear)
				//	resV.append(0);
				//else
					resV.append(double(sum)/double(avgCount));
			}
			else
				resV.append(0);
			sum = avgCount = 0;

			onNextMonthLeaveArea = false;
			inArea = false;
		}

		if(!inArea && month == from)
			inArea = true;

		if(inArea){
			avgCount++;
			sum += values.at(i);

			lastMonth = month;
			currentYear = year;
			onNextMonthLeaveArea = to == month;
		}
	}

	//add last element
	if(onNextMonthLeaveArea){// || from > to){ //discards the last year if from > to
		resY.append(currentYear);
		if(avgCount > 0)
			resV.append(double(sum)/double(avgCount));
		else
			resV.append(0);
	}

	return QPair<QVector<double>, QVector<double> >(resY, resV);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

QLVVDouble Tools::groupBy(const QVVDouble& cols, int byColumn){
	QList<QVector<QVector<double> > > r;
	double oldValue = cols.at(byColumn).first();
	int oldIndex = 0;
	for(int i = 0; i < cols.at(byColumn).size(); i++){
		double value = cols.at(byColumn).at(i);
		if(value != oldValue){
			QVVDouble ncols; //new columns
			foreach(QVector<double> v, cols){
				ncols.append(v.mid(oldIndex, i - oldIndex));
			}
			r.append(ncols);
			oldValue = value;
			oldIndex = i;
		}

	}
	QVVDouble ncols; //new columns
	foreach(QVector<double> v, cols){
		ncols.append(v.mid(oldIndex));
	}
	r.append(ncols);
	return r;
}
