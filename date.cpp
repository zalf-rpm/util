#include <sstream>
#include <cmath>
#include <cassert>
#include <iostream>
#include <cstdlib>

/*!
 * @file date.cpp
 */

#include "date.h"

using namespace Util;
using namespace std;

const unsigned int Date::_dim[] =
{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const unsigned int Date::_ldim[] =
{0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

const unsigned int Date::defaultRelativeBaseYear = 2000;

//! default constructor
Date::Date(bool useLeapYears)
:
_daysInMonth(NULL),
_d(0),
_m(0),
_y(0),
_useLeapYears(useLeapYears),
_relativeBaseYear(0),
_isRelativeDate(false)
{}

/*!
 * construct a date with a named month
 * @param day
 * @param month
 * @param year
 * @param useLeapYears date object will support leap yesrs
 */
Date::Date(unsigned int day, Month month, unsigned int year,
           bool useLeapYears) :
_daysInMonth(NULL),
_d(day),
_m(month),
_y(year),
_useLeapYears(useLeapYears),
_relativeBaseYear(0),
_isRelativeDate(false)
{
	_daysInMonth = isLeapYear() && _useLeapYears ? _ldim : _dim;
  if(day > daysInMonth(month) || day == 0)
  {
		_daysInMonth = NULL;
		_d = _m = _y = 0;
	}
}

/*!
 * construct a date from number values
 * @param day
 * @param month
 * @param year
 * @param useLeapYears date object will support leap years
 */
Date::Date(unsigned int day, unsigned int month, unsigned int year,
           bool useLeapYears, bool isRelativeDate,
           unsigned int relativeBaseYear) :
_daysInMonth(NULL),
_d(day),
_m(month),
_y(year),
_useLeapYears(useLeapYears),
_relativeBaseYear(relativeBaseYear),
_isRelativeDate(isRelativeDate)
{
	_daysInMonth = isLeapYear() && _useLeapYears ? _ldim : _dim;
  if(month > 12 || month == 0 || day > daysInMonth(month) || day == 0)
  {
		_daysInMonth = NULL;
		_d = _m = _y = 0;
	}
}

Date Date::relativeDate(unsigned int day, unsigned int month,
												int deltaYears, bool useLeapYears,
                        unsigned int relativeYear)
{
	return Date(day, month, relativeYear + deltaYears, useLeapYears,
							true, relativeYear);
}

//! copy constructor
Date::Date(const Date& other)
:
_daysInMonth(other._daysInMonth),
_d(other._d),
_m(other._m),
_y(other._y),
_useLeapYears(other._useLeapYears),
_relativeBaseYear(other._relativeBaseYear),
_isRelativeDate(other._isRelativeDate)
{}

/*!
 * assignment operator, copies the argument
 * @param other the other date
 * @return this, with the copied argument
 */
Date& Date::operator=(const Date& other)
                     {
	_daysInMonth = other._daysInMonth;
  _d = other.day();
  _m = other.month();
  _y = other.year();
	_useLeapYears = other._useLeapYears;
	_relativeBaseYear = other._relativeBaseYear;
	_isRelativeDate = other._isRelativeDate;
	return *this;
}

/*!
 * @param toDate
 * @return number of days to argument date, excluding the argument date
 * (thus 01.01.2000 to 01.01.2000 = 0 days)
 */
int Date::numberOfDaysTo(const Date& toDate) const
{
	assert(useLeapYears() == toDate.useLeapYears());
	assert((isRelativeDate() && toDate.isRelativeDate())
			|| (!isRelativeDate() && !toDate.isRelativeDate()));

	Date from(*this), to(toDate);
	bool reverse = from > to;
	if(reverse)
		from = toDate, to = *this;

	int nods = 0; //number of days
	if(from.year() == to.year() && from.month() == to.month())
		nods += to.day() - from.day();
  else
  {
    for(unsigned int y = from.year(); y <= to.year(); y++)
    {
			int startMonth = 1, endMonth = 12;
      if(y == from.year())
      {
				startMonth = from.month() + 1;
				nods += from.daysInMonth(from.month()) - from.day();
			}
      if(y == to.year())
      {
				endMonth = to.month() - 1;
				nods += to.day();
			}
			Date cy(1, 1, y, useLeapYears()); //current year, needed to count months in leap years correctly
			for(int m = startMonth; m >= 1 && m <= 12 && m <= endMonth; m++)
				nods += cy.daysInMonth(m);
		}
	}

	return nods * (reverse ? -1 : 1);
}

Date Date::withDay(unsigned int d)
{
  Date t(*this);
  t.setDay(d);
  return t;
}

Date Date::withMonth(unsigned int m)
{
  Date t(*this);
  t.setMonth(m);
  return t;
}

Date Date::withYear(unsigned int y)
{
  Date t(*this);
  t.setYear(y);
  return t;
}

/*!
 * compare two dates
 * @param other the other date
 * @return true if 'this' date lies before the argument date
 */
bool Date::operator<(const Date& other) const
{
	if(!isValid())
		return true;
	else if(!other.isValid())
		return false;
	if((isRelativeDate() && !other.isRelativeDate())
			|| (!isRelativeDate() && other.isRelativeDate()))
		return false;

	bool t = day() < other.day();
	//month might be <=
  if(t)
  {
		t = month() <= other.month();
		if(t) //year might be <=
			return year() <= other.year();
		else //year has to be <
			return year() < other.year();
  }
  else
  { //month has to be <
		t = month() < other.month();
		if(t) //year might be <=
			return year() <= other.year();
		else //year has to be <
			return year() < other.year();
	}

	return false;
}

/*!
 * @return mysql compatible string representation of 'this' date
 */
std::string Date::toMysqlString(const std::string& wrapInto) const
{
	ostringstream s;
	s << wrapInto << year()
	<< "-" << (month() < 10 ? "0" : "") << month()
	<< "-" << (day() < 10 ? "0" : "") << day() << wrapInto;
	return s.str();
}

/*!
 * @param separator to use for delimit days, month, year
 * @return general string representation
 */
std::string Date::toString(const std::string& separator,
                           bool skipYear) const
{
	ostringstream s;
	s << (day() < 10 ? "0" : "") << day()
	<< separator << (month() < 10 ? "0" : "") << month();
  if(!skipYear)
  {
    if(isRelativeDate())
    {
			int deltaYears = year() - _relativeBaseYear;
			s << separator << "year"
      << (deltaYears > 0 ? "+" : "");
			if(deltaYears != 0)
				s << deltaYears;
    }
    else
			s << separator << year();
	}
	return s.str();
}

/*!
 * @param days
 * @return a copy of 'this' date 'days' before
 */
Date Date::operator-(int days) const
{
	if(days < 0)
		return (*this) + (-days);

	Date cd(*this); //current date
	unsigned int ds = days; //days

  bool isRelativeDate = cd.isRelativeDate();
  int relativeBaseYear = cd.relativeBaseYear();

  while(true)
  {
    if(cd.day() <= ds)
    {
			ds -= cd.day();

			if(cd.month() == 1)
        cd = Date(31, dec, cd.year()-1, useLeapYears(),
                  isRelativeDate, relativeBaseYear);
			else
				cd = Date(cd.daysInMonth(cd.month()-1), cd.month()-1, cd.year(),
                  useLeapYears(), isRelativeDate, relativeBaseYear);
    }
    else
    {
			cd.setDay(cd.day() - ds);
			break;
		}
	}

	return cd;
}

/*!
 * postfix --
 * decrement 'this' date by one day
 * @param dummy to distiguish prefix from postfix
 * @return the old 'this' before decrementation
 */
Date Date::operator--(int)
{
	Date d = *this;
	operator-=(1);
	return d;
}

/*!
 * @param days
 * @return date 'days' ahead of 'this' date
 */
Date Date::operator+(int days) const
{
	if(days < 0)
		return (*this) - (-days);

	Date cd(*this); //current date
	int ds = days; //days

  bool isRelativeDate = cd.isRelativeDate();
  int relativeBaseYear = cd.relativeBaseYear();

  while(true)
  {
		int delta = cd.daysInMonth(cd.month()) - cd.day() + 1;
    if(delta <= ds)
    {
			ds -= delta;

			if(cd.month() == 12)
        cd = Date(1, 1, cd.year()+1, useLeapYears(),
                  isRelativeDate, relativeBaseYear);
			else
        cd = Date(1, cd.month()+1, cd.year(), useLeapYears(),
                  isRelativeDate, relativeBaseYear);
    }
    else
    {
			cd.setDay(cd.day() + ds);
			break;
		}
	}

	return cd;
}

/*!
 * postfix ++
 * increment 'this' date by one day
 * @param dummy to distinguish prefix from postfix
 * @return old 'this' before incrementation
 */
Date Date::operator++(int)
{
	Date d = *this;
	operator+=(1);
	return d;
}

Date Date::toAbsoluteDate(unsigned int absYear, bool ignoreDeltaYears) const
{
  int deltaYears = ignoreDeltaYears ? 0 : year() - _relativeBaseYear;
  return Date(day(), month(), absYear == 0 ? year() : absYear + deltaYears,
              useLeapYears());
}

//------------------------------------------------------------------------------

/*!
 * @param mysqlDateString
 * @return date parsed from a mysql date string
 */
Date Util::fromMysqlString(const char* mysqlDateString)
{
	string d(mysqlDateString ? mysqlDateString : "1951-01-01");
	int year = atoi(d.substr(0, 4).c_str());
	int month = atoi(d.substr(5, 2).c_str());
	int day = atoi(d.substr(8,2).c_str());
	//cout << day << "." << month << "." << year << endl;
	return Date(day, month, year);
}

//------------------------------------------------------------------------------

/*!
 * function testing the date class
 */
void Util::testDate()
{
	assert(Date(1,1,2001).numberOfDaysTo(Date(2,1,2001))== 1);
	assert(Date(1,1,2001).numberOfDaysTo(Date(1,1,2001))== 0);
	assert(Date(1,1,2001).numberOfDaysTo(Date(1,2,2001))== 31);

	assert(Date(1,1,2001) == Date(1,1,2001));
	assert(Date(1,1,2001) > Date(31,12,2000));
	assert(Date(1,1,2001) < Date(2,1,2001));
	assert(Date(1,1,2001) >= Date(1,1,2001));
	assert(Date(1,1,2001) >= Date(31,12,2000));
	assert(Date(1,1,2001) <= Date(1,1,2001));
	assert(Date(1,1,2001) <= Date(2,1,2001));

	//with leap years
	Date t(5,3,2008, true);
	assert(t-5 == Date(29,2,2008, true));
	assert(t-10 == Date(24,2,2008, true));
	assert(t-20 == Date(14,2,2008, true));
	assert(t-30 == Date(4,2,2008, true));
	assert(t-100 == Date(26,11,2007, true));
	assert(t-300 == Date(10,5,2007, true));
	assert(t-400 == Date(30,1,2007, true));
	assert(t-1000 == Date(9,6,2005, true));

	t = Date(25,2,2008, true);
	assert(t+5 == Date(1,3,2008, true));
	assert(t+10 == Date(6,3,2008, true));
	assert(t+20 == Date(16,3,2008, true));
	assert(t+30 == Date(26,3,2008, true));
	assert(t+100 == Date(4,6,2008, true));
	assert(t+300 == Date(21,12,2008, true));
	assert(t+400 == Date(31,3,2009, true));
	assert(t+1000 == Date(21,11,2010, true));

	//without leap years
	t = Date(5,3,2008);
	assert(t-5 == Date(28,2,2008));
	assert(t-10 == Date(23,2,2008));
	assert(t-20 == Date(13,2,2008));
	assert(t-30 == Date(3,2,2008));
	assert(t-100 == Date(25,11,2007));
	assert(t-300 == Date(9,5,2007));
	assert(t-400 == Date(29,1,2007));
	assert(t-1000 == Date(8,6,2005));

	t = Date(25,2,2008);
	assert(t+5 == Date(2,3,2008));
	assert(t+10 == Date(7,3,2008));
	assert(t+20 == Date(17,3,2008));
	assert(t+30 == Date(27,3,2008));
	assert(t+100 == Date(5,6,2008));
	assert(t+300 == Date(22,12,2008));
	assert(t+400 == Date(1,4,2009));
	assert(t+1000 == Date(22,11,2010));

}
