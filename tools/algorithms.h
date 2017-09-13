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

#ifndef ALGORITHMS_H_
#define ALGORITHMS_H_

#include <vector>
#include <utility>
#include <map>
#include <set>
#include <string>
#include <cmath>
#include <algorithm>
#include <utility>
#include <numeric>
#include <sstream>
#include <iterator>
#include <cassert>
#include <functional>
#include <limits>

namespace Tools
{
	std::vector<std::string> splitString(std::string s, 
																			 std::string splitElements, 
																			 std::pair<std::string, std::string> tokenDelimiters = std::make_pair("", ""),
																			 bool removeDelimiters = false);

	int createRandomNumber();
	int createRandomNumber(int max);

  inline double interpolate(double x1, double x2, double f1, double f2, double x)
  {
    return ((f2-f1)/(x2-x1)*(x-x1))+f1;
  }

  //----------------------------------------------------------------------------

  template<class Collection>
  Collection linearRegression(const Collection& xs, const Collection& ys);

  template<class Collection>
  Collection cumulativeSum(const Collection& ys, int n);

  template<class Collection>
  Collection expGlidingAverage(const Collection& ys,
    typename Collection::value_type alpha = 0.05);//= 0.3);

  template<class Collection>
  Collection simpleGlidingAverage(const Collection& ys, int n = 9);

  //----------------------------------------------------------------------------

  /*!
  * remove leading and trailing whitespace from copy of string
  * @param s the input string
  * @param whitespaces list of whitespace to be removed
  * @return a trimmed version of s
  */
  std::string trim(const std::string& s,
									 const std::string& whitespaces = " \t\f\v\n\r");

  /*!
  * keep a value in the given border [lower, upper]
  * @param lower the lower border
  * @param value
  * @param upper the upper border
  * @return value or lower if value < lower or upper if value > upper
  */
  template<typename T>
  T bound(T lower, T value, T upper);

  //! returns by default MJ/m²/d (or if set to false J/cm²)
  double sunshine2globalRadiation(int julianDay, double sunHours,
    double latitude, bool asMJpm2pd = true);

	double cloudAmount2globalRadiation(int dayOfYear,
		double cloudAmount, //[1/8]
		double latitude, //[deg°]
		double heightNN, //[m]
		bool asMJpm2pd = true);

	double hourlyVaporPressureDeficit(double hourlyTemperature, double dailyTmin, double dailyTavg, double dailyTmax);

	double solarDeclination(int dayOfTheYear);

	double solarElevation(int hour, double latitude, int dayOfTheYear);

	//returns hourly temperature value from daily variables
	double hourlyT(double tmin, double tmax, int h, int sunrise_h);

	//returns hourly global radiation
	double hourlyRad(double globrad, double lat, int doy, int h);

  /*!
  * capitalize a copy of the input
  * @param s the input string
  * @return the capitalized input string
  */
  std::string capitalize(const std::string& s);

  /*!
  * capitalize the input string itself
  * @param s the input string
  */
  void capitalizeInPlace(std::string& s);

  /*!
  * the reverse of capitalize
  * @param s the input string
  * @return decapitalized version of s
  */
  std::string decapitalize(const std::string& s);

  /*!
  * the reverse if capitalizeInPlace
  * @param s the input string
  */
  void decapitalizeInPlace(std::string& s);

  /*!
  * @param number
  * @return whether number is even
  */
  template<typename T>
  inline bool isEven(T number){ return int(number) % 2 == 0; }

  /*!
  * structure holding information necessary to build a histogram
  */
  struct HistogramData 
	{
    //! borders separating histogram slots
    std::vector<double> borders;

    //! x positions of columns
    std::vector<double> xs;

    //! the actual values of the histogram classes
    std::vector<double> classes;
  };

  /*!
  * calculate histogram data for the given step size
  * @param ys ... list of continuous data
  * @param step ... step size (jumps from class to class)
  * @param normalizeCount ... if data contains multiple parallel data sets,
  * which shouldn't appear in the class-counts, then the counts will be
  * divided by normalizeCount (e.g. WettReg offers 3 realizations which
  * should not appear directly in the counts, to be able to compare the
  * class-counts to say CLM data with just one realization
  * @return [borders, xs, class-counts]
  */
  HistogramData histogramDataByStepSize(const std::vector<double>& ys,
                                        double step,
                                        int normalizeCount = 1);

  /*!
  * similar to histogramDataByStepSize, but with a given amount of classes
  * @param ys ... list of data
  * @param n ... the amount of classes
  * @param normalizeCount
  * @return [borders, xs, class-counts]
  */
  HistogramData histogramDataByNoOfClasses(const std::vector<double>& ys,
    int n,
    int normalizeCount = 1);

  /*!
  * @param xs a collection
  * @return (minimum, maximum) of the collection
  */
  template<class Collection>
  std::pair<typename Collection::value_type, typename Collection::value_type>
    minMax(const Collection& xs);

  /*!
  * @param xs the collection
  * @return average of the values in the given collection
  */
  template<class Collection>
  double average(const Collection& xs);

  /*!
  * calculate the standard deviation of the values in the given collection
  * @param xis the collection
  * @param calcAvg function calculates the average itself
  * @param xavg if calcAvg == false, the average value to be used
  * @return
  */
  template<class Collection>
  double standardDeviation(const Collection& xs, bool calcAvg = true,
                           double xavg = 0);

  /*!
  * calculate the standard deviation and average of the collection of values
  * @param xis the collection of values
  * @return (standardDeviation, average)
  */
  template<class Collection>
  std::pair<double, double> standardDeviationAndAvg(const Collection& xis);

  //----------------------------------------------------------------------------

  /*!
  * a structure holding all necessary information to create a BoxPlot
  */
  struct BoxPlotInfo {
    //! empty boxplot, usually for use of BoxPlotInfo as value object
    BoxPlotInfo();

    /*!
    * create BoxPlotInfo object
    * this constructor won't set all possible data
    * @param m median
    * @param q25 25% quartile
    * @param q75 75% quartile
    * @param min absolute minimum
    * @param max absolute maximum
    */
    BoxPlotInfo(double m, double q25, double q75, double min, double max);

    //! median
    double median;

    //! 25% quartile
    double Q25;

    //! 75% quartile
    double Q75;

    //! absolute minimum
    double min;

    //! absolute maximum
    double max;

    //! minimal inner fence of the boxplot
    double minInnerFence;

    //! maximal inner fence of the boxplot
    double maxInnerFence;

    //! interquartile range (the box size itself)
    double IQ() const { return Q75 - Q25; }

    //! lower inner fence value
    double lowerInnerFence(int roundToDigits = 1) const;

    //! upper inner fence value
    double upperInnerFence(int roundToDigits = 1) const;

    //! lower outer fence value
    double lowerOuterFence(int roundToDigits = 1) const;

    //! upper outer fence value
    double upperOuterFence(int roundToDigits = 1) const;

    //! set of mild lower outliers
    std::set<double> mildLowerOutliers;

    //! set of mild upper outliers
    std::set<double> mildUpperOutliers;

    //! set of extreme lower outliers
    std::set<double> extremeLowerOutliers;

    //! set of extreme upper outliers
    std::set<double> extremeUpperOutliers;

    //! string representation (usually for debugging purposes)
    std::string toString() const;
  };

  /*!
  * analyse the input vector data for creation of a BoxPlot
  * @param data the input vector
  * @param orderedData whether the data are already ordered
  * @return BoxPlotInfo object containing all information
  * necessary to create a box plot
  */
  BoxPlotInfo boxPlotAnalysis(const std::vector<double>& data,
                              bool orderedData = false,
                              int roundToDigits = 1);

	double round(double, int);
															
  /*!
  * calculate median of given vector of values
  * @param orderedData an ordered vector of input values
  * @return median of all the values in orderedData
  */
  template<class Collection>
  double median(const Collection& orderedData, int roundToDigits = 1)
  {
    int size = orderedData.size();
    if(size == 0)
      return 0;
    return isEven(size)
        ? Tools::round((orderedData.at((size/2) - 1) + orderedData.at((size/2) + 1 - 1))/2.0, roundToDigits)
        : orderedData.at(int(size/2.0) + 1 - 1);
  }


  /*!
  * calculate the x*100 % quartile of the input data
  * @param xth quartile in the range of 0 - 1
  * @param orderedData ordered input data
  * @return the x*100 % quarile of the values in orderedData
  */
  double quartile(double xth, const std::vector<double>& orderedData, int roundToDigits = 1);

  //----------------------------------------------------------------------------

  std::pair<double, int> decomposeIntoSci(double value);

	int integerRound1stDigit(int value);

	template<typename ReturnType>
	inline ReturnType shiftDecimalPointRight(double value, int digits)
	{
		for(int d = 0; d < digits; d++)
			value *= 10;
		return ReturnType(value);
	}
	
	template<typename ReturnType>
	inline ReturnType shiftDecimalPointLeft(double value, int digits)
	{
		for(int d = 0; d < digits; d++)
			value /= 10.0;
		return ReturnType(value);
	}
	
	//round to int but keep shifted to roundToDigits
	int roundShiftedInt(double value, int roundToDigits);

	//round to given amount of digits (-digits = round to integer digits = left side of decimal point)
	//return casted to given ReturnType
  template<typename ReturnType>
  ReturnType roundRT(double value, int roundToDigits)
	{
		int rsi = roundShiftedInt(value, roundToDigits);
		
		//round to full integers or digits after the decimal point
		if(roundToDigits >= 0) 
			return shiftDecimalPointLeft<ReturnType>(rsi, roundToDigits);
		
		//round to full 100s
		if(roundToDigits < -1) 
			return shiftDecimalPointRight<ReturnType>(rsi, -roundToDigits);
		
		//round to full 10s
		return ReturnType(rsi);
	}
  
	inline double round(double value, int roundToDigits) { return roundRT<double>(value, roundToDigits); }

	inline double round(double value){ return roundRT<double>(value, 0); }
	
  double floor(double value, int digits = 0, bool trailingDigits = true);
  double ceil(double value, int digits = 0, bool trailingDigits = true);

  void testRoundFloorCeil();

  //----------------------------------------------------------------------------

  /*!
  * elementwise vector operation (eg. + on elements)
  * @param left vector
  * @param right vector
  * @param op binary operation
  * @param autoFit if autofit, result vector will be autofit to shorter
  * of the two input vectors
  * @return vector as result of elementwise operation
  * (l1 op r1, l2 op r2, ...., lx op rx)
  */
  template<class Vector1, class Vector2, class OP>
  Vector1 elemVecOp(const Vector1& left, const Vector2& right, OP op,
    bool autoFit);

  template<typename T>
  std::vector<T> operator+(const std::vector<T>& left, const std::vector<T>& right)
  {
    return elemVecOp(left, right, std::plus<T > (), false);
  };

  template<typename T>
  std::vector<T> operator-(const std::vector<T>& left, const std::vector<T>& right)
  {
    return elemVecOp(left, right, std::minus<T > (), false);
  };

  template<typename T>
  std::vector<T> operator*(const std::vector<T>& left, const std::vector<T>& right)
  {
    return elemVecOp(left, right, std::multiplies<T > (), false);
  };

  template<typename T>
  std::vector<T> operator/(const std::vector<T>& left, const std::vector<T>& right)
  {
    return elemVecOp(left, right, std::divides<T > (), false);
  };

  /*!
  * inplace elementwise vector operation (eg. + on elements) on left
  * @param left vector
  * @param right vector
  * @param op binary operation
  * @param autoFit if autofit, result vector will be autofit to shorter
  * of the two input vectors
  * @return vector as result of elementwise operation
  * (l1 op r1, l2 op r2, ...., lx op rx)
  */
  template<class Vector1, class Vector2, class OP>
  Vector1& inElemVecOp(Vector1& left, const Vector2& right, OP op, bool autoFit);

  template<typename T>
  std::vector<T> operator+=(std::vector<T>& left, const std::vector<T>& right)
  {
    return inElemVecOp(left, right, std::plus<T>(), false);
  };

  template<typename T>
  std::vector<T>& operator-=(std::vector<T>& left, const std::vector<T>& right)
  {
    return inElemVecOp(left, right, std::minus<T>(), false);
  };

  template<typename T>
  std::vector<T> & operator*=(std::vector<T>& left, const std::vector<T>& right)
  {
    return inElemVecOp(left, right, std::multiplies<T > (), false);
  };

  template<typename T>
  std::vector<T>& operator/=(std::vector<T>& left, const std::vector<T>& right)
  {
    return inElemVecOp(left, right, std::divides<T>(), false);
  };

  /*!
  * scalar operation on vector
  * @param left input vector
  * @param right scalar
  * @param op operation to apply to every every vector element with scalar
  * @return vector as result of scalar operation
  * (l1 op r, l2 op r, ..., lx op r)
  */
  template<class Vector, typename T, class OP>
  Vector scalVecOp(const Vector& left, T right, OP op);

  template<typename T>
  std::vector<T> operator+(const std::vector<T>& left, T right)
  {
    return scalVecOp(left, right, std::plus<T>());
  }

  template<typename T>
  std::vector<T> operator-(const std::vector<T>& left, T right)
  {
    return scalVecOp(left, right, std::minus<T > ());
  }

  template<typename T>
  std::vector<T> operator*(const std::vector<T>& left, T right)
  {
    return scalVecOp(left, right, std::multiplies<T>());
  }

  template<typename T>
  std::vector<T> operator/(const std::vector<T>& left, T right)
  {
    return scalVecOp(left, right, std::divides<T>());
  }

  /*!
  * inplace scalar operation on vector
  * @param left input vector
  * @param right scalar
  * @param op operation to apply to every every vector element with scalar
  * @return vector as result of scalar operation
  * (l1 op r, l2 op r, ..., lx op r)
  */
  template<class Vector, typename T, class OP>
  Vector& inScalVecOp(Vector& left, T right, OP op);

  template<typename T>
  std::vector<T>& operator+=(std::vector<T>& left, T right)
  {
    return inScalVecOp(left, right, std::plus<T>());
  };

  template<typename T>
  std::vector<T>& operator-=(std::vector<T>& left, T right)
  {
    return inScalVecOp(left, right, std::minus<T>());
  };

  template<typename T>
  std::vector<T>& operator*=(std::vector<T>& left, T right)
  {
    return inScalVecOp(left, right, std::multiplies<T>());
  };

  template<typename T>
  std::vector<T>& operator/=(std::vector<T>& left, T right)
  {
    return inScalVecOp(left, right, std::divides<T>());
  };

  //----------------------------------------------------------------------------

  /*!
  * season start is, if the average daily temperature raises above 5°C
  * and the sum of (the average daily temperature - 5°C) of the following 30 days
  * is above 0°C
  */
  template<class DoubleVector>
  int findThermalVegetationalSeasonStart(const DoubleVector& ts);

  /*!
  * - season end is, if the average daily temperature falls below 5°C and
  * on average stays that way
  * - that means that the sum of (the average daily temperatures - 5°C) of the
  * rest of the year has to be below 0°C
  */
  template<class DoubleVector>
  int findThermalVegetationalSeasonEnd(const DoubleVector& ts, int startDay = 0);

  // - is being defined only for an average year of a choosen climate period
  // (thus a lot of years), to prevent virtual short periods in winter time
  // - is the longest season in the year where the average air temperature
  // is above 10°C (thus tavg > 10°C)
  template<class Collection>
  std::pair<int, int> forestalVegetationalSeasonStartEnd(const Collection& ts)
  {
    return forestalVegetationalSeasonStartEnd(ts.begin(), ts.end());
  }

  template<class Iterator>
  std::pair<int, int> forestalVegetationalSeasonStartEnd(Iterator start, Iterator end);

  //template<class Collection>
  //Collection classifyClimateZonesForPNV(const Collection& qyrs,
  //																			const Collection& qgss);

  enum PNVQ { Qyr = 0, Qgs };
  template<class Collection>
  Collection classifyClimateZonesForPNV(const Collection& qs, PNVQ pnvq);

}

//------------------------------------------------------------------------------
//implementations---------------------------------------------------------------
//------------------------------------------------------------------------------

template<class Collection>
Collection Tools::linearRegression(const Collection& xs, const Collection& ys)
{
  typedef typename Collection::value_type T;

  Q_ASSERT(xs.size() <= ys.size());

  int n = xs.size();
  T b_r, a_r;
  T sumXY, sumX, sumY, sumXSq;
  b_r = a_r = sumXY = sumX = sumY = sumXSq = 0;

  for(int i = 0; i < n; i++)
  {
    T x_i = xs.at(i);
    T y_i = ys.at(i);
    sumXY += x_i * y_i;
    sumX += x_i;
    sumY += y_i;
    sumXSq += x_i * x_i;
  }

  b_r = (sumXY - ((sumX * sumY) / T(n))) / (sumXSq - ((sumX * sumX) / T(n)));
  a_r = (sumY - (b_r * sumX)) / (T(n));

  Collection ys_r(n);
  for(int i = 0; i < n; i++)
    ys_r[i] = a_r + (b_r * xs.at(i));

  return ys_r;
}

template<class Collection>
Collection Tools::cumulativeSum(const Collection& ys, int n)
{
  typedef typename Collection::value_type T;

  T acc = 0;
  Collection yis(n);
  for(int i = 0; i < n; i++)
    yis[i] = (acc += ys[i]);

  return yis;
}

template<class Collection>
Collection Tools::expGlidingAverage(const Collection& ys,
                                    typename Collection::value_type alpha)
{
  typedef typename Collection::value_type T;

  if(ys.empty())
    return Collection();

  Collection yas(ys.size());
  T ya = yas[0] = ys.first();
  for(int i = 1; i < ys.size(); i++)
    ya = yas[i] = (alpha * ys.at(i)) + ((1-alpha) * ya);

  return yas;
}

template<class Collection>
Collection Tools::simpleGlidingAverage(const Collection& ys, int n)
{
  typedef typename Collection::value_type T;

  int lr = int(T(isEven(n) ? n : n-1) / 2.0);  //left right
  n = (2*lr) + 1;

  if(ys.size() < n)
    return Collection();

  Collection yas(ys.size() - (2*lr));
  T sum = 0;
  for(int i = 0; i < n; i++)
    sum += ys.at(i);

  yas[0] = sum / T(n);
  for(int i = 0; i < ys.size() - n; i++)
  {
    sum = sum - ys.at(i) + ys.at(i+n);
    yas[i+1] = sum / T(n);
  }

  return yas;
}

//------------------------------------------------------------------------------

template<class Collection>
std::pair<typename Collection::value_type, typename Collection::value_type>
	Tools::minMax(const Collection& vs)
{
  typedef typename Collection::value_type T;
  if(vs.empty())
    return std::make_pair(T(0), T(0));

	T minv = std::numeric_limits<T>::max(); //*(ys.begin());
	T maxv = std::numeric_limits<T>::min(); // *(ys.begin());
  for(auto v : vs)
		minv = std::min(minv, v), maxv = std::max(maxv, v);
  return std::make_pair(minv, maxv);
}

template<typename T>
T Tools::bound(T lower, T value, T upper)
{
  if(value < lower) return lower;
  if(value > upper) return upper;
  return value;
}

template<class Collection>
double Tools::average(const Collection& xs)
{
  return xs.size() > 0
      ? std::accumulate(xs.begin(), xs.end(), double(0)) / double(xs.size())
      : 0.0;
}

template<class Collection>
double Tools::standardDeviation(const Collection& xs, bool calcAvg,
                                double xavg)
{
  int n = xs.size();

  if(n < 2)
    return 0.0;

  if(calcAvg)
    xavg = average(xs);

  double sum = 0;
//  for(typename Collection::const_iterator ci = xs.begin(); ci != xs.end(); ci++)
//    sum += (double(*ci) - xavg) * (double(*ci) - xavg);
  for(auto x : xs)
    sum += (double(x) - xavg)*(double(x) - xavg);

  return std::sqrt(sum / double(n-1));
}

template<class Collection>
std::pair<double, double> Tools::standardDeviationAndAvg(const Collection& xs)
{
  double avg = average(xs);
  return std::make_pair(standardDeviation(xs, false, avg), avg);
}

template<class Vector1, class Vector2, class OP>
Vector1 Tools::elemVecOp(const Vector1& left, const Vector2& right, OP op,
                         bool autoFit)
{
  int msize = std::min(int(left.size()), int(right.size()));

  if(!autoFit && int(left.size()) != int(right.size()))
    return Vector1();

  Vector1 res(msize);
  for(int i = 0; i < msize; i++)
    res[i] = op(left.at(i), right.at(i));

  return res;
}

template<class Vector1, class Vector2, class OP>
Vector1& Tools::inElemVecOp(Vector1& left, const Vector2& right, OP op,
                            bool autoFit)
{
  int msize = std::min(int(left.size()), int(right.size()));

  if(!autoFit && int(left.size()) != int(right.size()))
    return left;

  for(int i = 0; i < msize; i++)
    left[i] = op(left.at(i), right.at(i));

  return left;
}

template<class Vector, typename T, class OP>
Vector Tools::scalVecOp(const Vector& left, T right, OP op){
  Vector res(left.size());
  for(int i = 0, size = left.size(); i < size; i++)
    res[i] = op(left.at(i), right);
  return res;
}

template<class Vector, typename T, class OP>
Vector& Tools::inScalVecOp(Vector& left, T right, OP op){
  for(int i = 0, size = left.size(); i < size; i++)
    left[i] = op(left.at(i), right);
  return left;
}

//------------------------------------------------------------------------------

template<class DoubleVector>
int Tools::findThermalVegetationalSeasonStart(const DoubleVector& ts)
{
  // cout << "start ts: " << ts << endl;
  double tresh = 5;
  int length = 30;
  for(int i = 0, size = ts.size(); i < size; i++)
  {
    if(ts.at(i) > tresh)
    {
      double sum = 0;
      for(int k = i+1; k < size && k < i+length; k++)
        sum += ts.at(k) - tresh;
      if(sum > 0)
        return i;
    }
  }
  return 365;
}

template<class DoubleVector>
int Tools::findThermalVegetationalSeasonEnd(const DoubleVector& ts, int startDay)
{
  //  cout << "end ts: " << ts << endl;
  double tresh = 5;
  for(int i = startDay, size = ts.size(); i < size; i++)
  {
    if(ts.at(i) < tresh)
    {
      double sum = 0;
      for(int k = i+1; k < size; k++)
        sum += ts.at(k) - tresh;
      if(sum < 0)
        return i;
    }
  }
  return 365;
}

template<class Iterator>
std::pair<int, int> Tools::forestalVegetationalSeasonStartEnd(Iterator begin,
                                                              Iterator end)
{
  static const double threshold = 10.0;
  std::map<int, int> start2end;
  int start = 0;
  int i = 0;
  for(Iterator it = begin; it != end; it++, i++)
  {
    typename std::iterator_traits<Iterator>::value_type tavg = *it;
    if(start == 0 && tavg > threshold)
      start = i+1;
    else if(start > 0 && tavg <= threshold)
    {
      start2end[start] = i+1;
      //try to find another period
      start = 0;
    }
  }

  int maxPeriodLength = 0;
  auto periodIt = start2end.end();
  for(auto cit = start2end.begin(); cit != start2end.end(); cit++)
  {
    int currentPeriodLength = cit->second - cit->first;
    if(currentPeriodLength > maxPeriodLength)
    {
      maxPeriodLength = currentPeriodLength;
      periodIt = cit;
    }
  }

  return periodIt == start2end.end() ? std::make_pair(0, 0)
    : std::make_pair(periodIt->first,
    periodIt->second);
}

//template<class Collection>
//Collection Tools::classifyClimateZonesForPNV(const Collection& qyrs,
//																						 const Collection& qgss)
//{
//	assert(qyrs.size() == qgss.size());
//	Collection res;
//	for(int i=0, size=qyrs.size(); i<size; i++)
//	{
//		Collection::value_type qyr = qyrs.at(i);
//		Collection::value_type qgs = qgss.at(i);
//		if(qyr <= 10 && qgs <= 20)
//			res.push_back(1);
//		else if(10 < qyr && qyr <= 20 &&
//						20 < qgs && qgs <= 40)
//			res.push_back(2);
//		else if(20 < qyr && qyr <= 30 &&
//						40 < qgs && qgs <= 53)
//			res.push_back(3);
//		else if(30 < qyr && 53 < qgs)
//			res.push_back(4);
//		else
//			res.push_back(0);
//	}
//	return res;
//}

template<class Collection>
Collection Tools::classifyClimateZonesForPNV(const Collection& qs, PNVQ pnvq)
{
  Collection res;
  for(int i=0, size=qs.size(); i<size; i++)
  {
    typename Collection::value_type q = qs.at(i);
    switch(pnvq)
    {
    case Qyr:
      if(q <= 10)
        res.push_back(1);
      else if(10 < q && q <= 20)
        res.push_back(2);
      else if(20 < q && q <= 30)
        res.push_back(3);
      else if(30 < q)
        res.push_back(4);
      break;
    case Qgs:
      if(q <= 20)
        res.push_back(1);
      else if(20 < q && q <= 40)
        res.push_back(2);
      else if(40 < q && q <= 53)
        res.push_back(3);
      else if(53 < q)
        res.push_back(4);
      break;
    }
  }
  return res;
}

#endif
