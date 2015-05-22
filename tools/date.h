/**
Authors: 
Michael Berg <michael.berg@zalf.de>

Maintainers: 
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of 
Landscape Systems Analysis at the ZALF.
Copyright (C) 2007-2013, Leibniz Centre for Agricultural Landscape Research (ZALF)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DATE_H_
#define DATE_H_

/*!
 * @file date.h
 */

#include <string>

namespace Tools
{
  enum Month
  {
    jan = 1, feb, mar, apr, may, jun, jul, aug, sep, oct, nov, dec
  };

  /*!
   * A simple date class, created when there shouldn't be any boost dependencies.
   * Should maybe be replaced by something more standard.
   * Supports leap years and simple date arithmetic, even though the
   * implementation might not stand up to professional needs.
   * @author Michael Berg
   */
  class  Date
  {
  public:
    //! to be able to use named months

    static const unsigned int defaultRelativeBaseYear;

  public:
    Date(bool useLeapYears = false);

    explicit Date(unsigned int day, Month month, unsigned int year,
                  bool useLeapYears = false);

    Date(unsigned int day, unsigned int month, unsigned int year,
         bool useLeapYears = false, bool isRelativeDate = false,
         unsigned int relativeBaseYear = defaultRelativeBaseYear);

    static Date relativeDate(unsigned int day, unsigned int month,
                             int deltaYears = 0, bool useLeapYears = false,
                             unsigned int relativeYear = defaultRelativeBaseYear);

    static inline Date julianDate(unsigned int julianDay, unsigned int year,
                                  bool useLeapYears = false, bool isRelativeDate = false,
                                  unsigned int relativeBaseYear = defaultRelativeBaseYear)
    {
      return Date(1,1,year, useLeapYears, isRelativeDate, relativeBaseYear) + (julianDay - 1);
    }

    Date(const Date& other);

    /*!
     * @return true if this is a valid date (not default constructed)
     */
    bool isValid() const { return _d > 0 && _y > 0; }

    Date & operator=(const Date& other);

    bool operator<(const Date& other) const;

    /*!
     * compare two dates for equality
     * @param other the other date
     * @return true if both dates match exactly
     */
    bool operator==(const Date& other) const
    {
      return year() == other.year() && month() == other.month()
        && day() == other.day() && isRelativeDate() == other.isRelativeDate();
    }

    bool operator!=(const Date& other) const
    {
			return !(*this == other);
		}

    /*!
     * compare if 'this' date is equal to the other date or lies before the
     * argument
     * @param other
     * @return
     */
    bool operator<=(const Date& other) const
    {
      return (*this) < other || (*this) == other;
    }

    /*!
     * @param other
     * @return true if 'this' date lies after the argument date
     */
    bool operator>(const Date& other) const
    {
      return !((*this) <= other);
    }

    /*!
     * @param other
     * @return true if 'this' date is equal to or lies after the argument date
     */
    bool operator>=(const Date& other) const
    {
      return !((*this) < other);
    }

    Date operator-(int days) const;

    /*!
     * @param days
     * @return change 'this' date to 'days' before
     */
    Date & operator-=(int days) { return (*this) = (*this) - days; }

    /*!
     * prefix --
     * @see Date& operator-=(int days)
     */
    Date & operator--() {  return operator-=(1); }

    Date operator--(int);

    Date operator+(int days) const;

    /*!
     * @param days
     * @return 'this' date changed to 'days' ahead
     */
    Date & operator+=(int days) { return (*this) = (*this) + days; }

    /*!
     *
     * @return 'this' date changed to one day ahead
     */
    Date & operator++() { return operator+=(1); }

    Date operator++(int);

    /*!
     * subtract two dates
     * @param other date
     * @return difference in days between the two days
     */
    int operator-(const Date& other) const {

      return other.numberOfDaysTo(*this);
    }

    /*!
     * @return if 'this' dates year is a leap year
     */
    bool isLeapYear() const { return (_y - _aLeapYear) % 4 == 0; }

    /*!
     * @param month
     * @return how many days has the argument month
     */
    unsigned int daysInMonth(unsigned int month) const
    {
      return _daysInMonth[month];
    }

    /*!
     * @param year
     * @param month
     * @param useLeapYears
     * @return how many days has the month in the argument year, month under
     * possible leap year conditions
     */
    static unsigned int daysInMonth(unsigned int year, unsigned int month,
                                    bool useLeapYears = false)
    {
      return Date(1, 1, year, useLeapYears).daysInMonth(month);
    }


    int numberOfDaysTo(const Date& toDate) const;

    /*!
     * @return 'this' dates day
     */
    unsigned int day() const { return _d; }

    /*!
     * set 'this' dates day
     * @param day
     */
    void setDay(unsigned int day) { _d = day; }

    Date withDay(unsigned int day);

    /*!
     * @return 'this' dates month
     */
    unsigned int month() const { return _m; }

    Date withMonth(unsigned int month);

    /*!
     * set 'this' dates month
     * @param month
     */
    void setMonth(unsigned int month) { _m = month; }

    /*!
     * @return 'this' dates year
     */
    unsigned int year() const { return _y; }

    /*!
     * set 'this' dates year
     * @param year
     */
    void setYear(unsigned int year) { _y = year; }

    Date withYear(unsigned int year);

    void addYears(int years){  setYear(year() + years); }

    std::string toMysqlString(const std::string& wrapInto = "'") const;

    std::string toString(const std::string& separator = ".",
                         bool skipYear = false) const;

    /*!
     * @return some leap year for use in calculations
     */
    static unsigned int aLeapYear() { return _aLeapYear; }

		static unsigned int notALeapYear() { return aLeapYear() - 1; }

    /*!
     * @ see dayInYear(int year, int day, int month, bool useLeapYears)
     * @return julian day of day in 'this' date
     */
    unsigned int julianDay() const
    {
      return dayInYear(year(), day(), month(), _useLeapYears);
    }

    /*!
     * @see julianDay()
     * @return day in year of day in 'this' date
     */
    unsigned int dayOfYear() const { return julianDay(); }

    /*!
     * @param day
     * @param month
     * @return day in year of argument day in argument month in 'this' dates year
     */
    unsigned int dayInYear(unsigned int day, unsigned int month) const
    {
      return dayInYear(year(), day, month, _useLeapYears);
    }

    /*!
     * @param year
     * @param day
     * @param month
     * @param useLeapYears
     * @return day in year of argument day in argument month in argument year
     * under possible leap year conditions
     */
    static unsigned int dayInYear(unsigned int year, unsigned int day,
                                  unsigned int month,
                                  bool useLeapYears = false)
    {
      return Date(day, month, year, useLeapYears)
        - Date(1, jan, year, useLeapYears) + 1;
    }

    /*!
     * @return if 'this' date uses leap years
     */
    bool useLeapYears() const { return _useLeapYears; }

    /*!
     * @return date representing the start of 'this' dates year
     */
    Date startOfYear() const { return Date(1, 1, year()); }

    /*!
     * @return date representing the end of 'this' dates year
     */
    Date endOfYear() const { return Date(31, 12, year()); }

    /*!
     * @return date representing the start of 'this' dates month
     */
    Date startOfMonth() const { return Date(1, month(), year()); }

    /*!
     * @return date representing the end of 'this' dates month
     */
    Date endOfMonth() const
    {
      return Date(daysInMonth(month()), month(), year());
    }

    /*!
     * @return true if this date is relative, thus the year
     * doesn't actually mean much and is just used to distinguish
     * between this and next year (this and the other year)
     */
    bool isRelativeDate() const { return _isRelativeDate; }

    //! convert this year to an absolute year
    Date toAbsoluteDate(unsigned int absYear = 0,
                        bool ignoreDeltaYears = false) const;

    Date toRelativeDate(int deltaYears = 0, bool useLeapYears = false,
                        unsigned int relativeYear = defaultRelativeBaseYear)
    {
      return relativeDate(day(), month(), deltaYears, useLeapYears,
                          relativeYear);
    }

    //! is the current year different from base year
    bool isRelativeBaseYear() const {  return _relativeBaseYear == year(); }

    //! get the base year if this is a relative date
    int relativeBaseYear() const { return _relativeBaseYear; }

  private:
    //! days in month (1-indexed)
    static const unsigned int _dim[];
    //! days in month in a leap year
    static const unsigned int _ldim[];
    //! pointer to correct leap year array, depending of activated leap years
    unsigned int const* _daysInMonth;

    //! members variables for day, month, year
    unsigned int _d, _m, _y;
    //! the choosen arbitrary leap year
    static const unsigned int _aLeapYear = 2008;

    //! member var holding whether leap years are being used or not
    bool _useLeapYears;

    //! choosen base relative year
    unsigned int _relativeBaseYear;

    //! is this a relative date = what's the meaning of the year
    bool _isRelativeDate;
  };

	Date fromMysqlString(const char* mysqlDateString);

	inline Date fromMysqlString(const std::string& mysqlDateString)
	{
		return fromMysqlString(mysqlDateString.c_str());
	}

	void testDate();
}

#endif /*DATE_H_*/
