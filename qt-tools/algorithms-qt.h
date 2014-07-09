#ifndef ALGORITHMS_QT_H_
#define ALGORITHMS_QT_H_

#include <QtCore/QVector>
#include <QtCore/QPair>
#include <QtCore/QList>
#include <utility>
#include <numeric>
#include <algorithm>

#include "tools/algorithms.h"

namespace Tools
{
#undef min
#undef max

	template<class Collection>
	QPair<double, double> qtMinMax(const Collection& ys)
	{
		const std::pair<double, double>& mm = minMax(ys);
		return qMakePair(mm.first, mm.second);
	}

	/**
	 * only useful if first consists of unique values
	 */
	template<typename T1, typename T2>
	QMap<T1, T2> joinToMap(const QList<T1>& first, const QList<T2>& second,
												 bool multi = false)
	{
		QMap<T1, T2> map;
		for(int i = 0; i < first.size(); i++)
		{
			if(multi)
				map.insertMulti(first.at(i), second.at(i));
			else
				map.insert(first.at(i), second.at(i));
		}
		return map;
	}

	//----------------------------------------------------------------------------

	template<typename T, template<class> class Collection = QVector>
	class Range
	{
	public:
		Range(T from) : _from(from), _to(0), _step(1) {}
		Range(T from, T to) : _from(from), _to(to), _step(1) {}
		Range(T from, T to, T step) : _from(from), _to(to), _step(step) {}

		Collection<T> operator()() const { return createRange(); }

		Collection<T> operator>>(T to)
		{
			_to = to;
			return createRange();
		}

	private: //methods
		Collection<T> createRange() const
		{
			Collection<T> res;
			for(T i = _from; i <= _to; i += _step)
				res.append(i);
			return res;
		}

	private: //state
		T _from;
		T _to;
		T _step;
	};

	//----------------------------------------------------------------------------

	/*
	QVector<double> linearRegressionGSL(const QVector<double>& xs,
	                                    const QVector<double>& ys, int n);
	//*/
	/*
	QVector<double> linearRegression(const QVector<double>& xs,
	                                 const QVector<double>& ys);

	QVector<double> cumulativeSum(const QVector<double>& ys, int n);

	QVector<double> expGlidingAverage(const QVector<double>& ys,
	                                  double alpha = 0.3);

	QVector<double> simpleGlidingAverage(const QVector<double>& ys, int n = 9);
	 */

	QList<QVector<double> > hist(const QVector<double>& ys, int noOfClasses);

	QVector<int> periods(const QVector<double>& ys, bool (*f)(double), int nPlus1);
	//QVector<int> periods(const QVector<double>& ys, bool (*f)(double), int nPlus1,
	//	bool shorterPeriodsInclusive = false);
	QVector<int> periodsByNeighbour(const QVector<double>& xs, int nPlus1);
	//QVector<int> periodsFilteredByNeighbour(const QVector<double>& xs, int nPlus1,
	//bool shorterPeriodsInclusive);

	QVector<double> distributeColumnsJoined(int n, int upTo, int widthInPercent = 100);
	QVector<QVector<double> > distributeColumns(int n, int upTo, int widthInPercent = 100);

  //int findThermalVegetationalSeasonStart(const QVector<double>& ts);
  //int findThermalVegetationalSeasonEnd(const QVector<double>& ts);

	QPair<QVector<double>, QVector<double> >
	average(const QVector<double>& groupCol, const QVector<double>& values);
	QPair<QVector<double>, QVector<double> >
	sum(const QVector<double>& groupCol, const QVector<double>& values);
	QPair<QVector<double>, QVector<double> >
	min(const QVector<double>& groupCol, const QVector<double>& values);
	QPair<QVector<double>, QVector<double> >
	max(const QVector<double>& groupCol, const QVector<double>& values);
	QPair<QVector<double>, QVector<double> >
	monthlyAverage(int from, int to, const QVector<double>& monthCol,
	               const QVector<double>& yearCol,
	               const QVector<double>& values);

	/**
	 * split/group a list of columns by the values in the given columns
	 * thus, return a list of separate lists of columns
	 */
	typedef QVector<QVector<double> > QVVDouble;
	typedef QList<QVVDouble> QLVVDouble;
	QLVVDouble groupBy(const QVVDouble& cols, int byColumn);
}

#endif
