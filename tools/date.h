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

#ifndef DATE_H_
#define DATE_H_

/*!
 * @file date.h
 */

#include <string>
#include <vector>

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
		static const size_t defaultRelativeBaseYear = 2000;

	public:
		Date(bool useLeapYears = false);

		Date(const std::string& isoDateString, bool useLeapYears = false);

		explicit Date(size_t day, Month month, size_t year,
									bool useLeapYears = false);

		Date(size_t day, size_t month, size_t year,
				 bool useLeapYears = false, bool isRelativeDate = false,
				 size_t relativeBaseYear = defaultRelativeBaseYear);

		static Date relativeDate(size_t day, size_t month,
														 int deltaYears = 0, bool useLeapYears = false,
														 size_t relativeYear = defaultRelativeBaseYear);

		static inline Date julianDate(size_t julianDay, size_t year,
																	bool useLeapYears = false, bool isRelativeDate = false,
																	size_t relativeBaseYear = defaultRelativeBaseYear)
		{
			return Date(1, 1, year, useLeapYears, isRelativeDate, relativeBaseYear) + (julianDay - 1);
		}

		static Date fromIsoDateString(const std::string& isoDateString, bool useLeapYears = true);

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

		Date operator-(size_t days) const;

		/*!
		 * @param days
		 * @return change 'this' date to 'days' before
		 */
		Date & operator-=(size_t days) { return (*this) = (*this) - days; }

		/*!
		 * prefix --
		 * @see Date& operator-=(int days)
		 */
		Date & operator--() { return operator-=(1); }

		Date operator--(int);

		Date operator+(size_t days) const;

		/*!
		 * @param days
		 * @return 'this' date changed to 'days' ahead
		 */
		Date & operator+=(size_t days) { return (*this) = (*this) + days; }

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
		int operator-(const Date& other) const
		{

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
		size_t daysInMonth(size_t month = 0) const
		{
			return _daysInMonth->at(month == 0 ? this->month() : month);
		}

		/*!
		 * @param year
		 * @param month
		 * @param useLeapYears
		 * @return how many days has the month in the argument year, month under
		 * possible leap year conditions
		 */
		static size_t daysInMonth(size_t year, size_t month,
															bool useLeapYears = false)
		{
			return Date(1, 1, year, useLeapYears).daysInMonth(month);
		}


		int numberOfDaysTo(const Date& toDate) const;

		/*!
		 * @return 'this' dates day
		 */
		size_t day() const { return _d; }

		/*!
		 * set 'this' dates day
		 * @param day
		 */
		void setDay(size_t day) { _d = day; }

		Date withDay(size_t day);

		/*!
		 * @return 'this' dates month
		 */
		size_t month() const { return _m; }

		Date withMonth(size_t month);

		/*!
		 * set 'this' dates month
		 * @param month
		 */
		void setMonth(size_t month) { _m = month; }

		/*!
		 * @return 'this' dates year
		 */
		size_t year() const { return _y; }

		/*!
		 * set 'this' dates year
		 * @param year
		 */
		void setYear(size_t year) { _y = year; }

		Date withYear(size_t year);

		void addYears(int years) { setYear(year() + years); }

		//return mysql compatible string representation of 'this' date
		std::string toMysqlString(const std::string& wrapInto = "'") const
		{
			return toIsoDateString(wrapInto);
		}

		std::string toIsoDateString(const std::string& wrapInto = "") const;

		std::string toString(const std::string& separator = ".",
												 bool skipYear = false) const;

		/*!
		 * @return some leap year for use in calculations
		 */
		static size_t aLeapYear() { return _aLeapYear; }

		static size_t notALeapYear() { return aLeapYear() - 1; }

		/*!
		 * @ see dayInYear(int year, int day, int month, bool useLeapYears)
		 * @return julian day of day in 'this' date
		 */
		size_t julianDay() const
		{
			return dayInYear(year(), day(), month(), _useLeapYears);
		}

		/*!
		 * @see julianDay()
		 * @return day in year of day in 'this' date
		 */
		size_t dayOfYear() const { return julianDay(); }

		/*!
		 * @param day
		 * @param month
		 * @return day in year of argument day in argument month in 'this' dates year
		 */
		size_t dayInYear(size_t day, size_t month) const
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
		static size_t dayInYear(size_t year, size_t day,
														size_t month,
														bool useLeapYears = false)
		{
			return size_t(Date(day, month, year, useLeapYears)
				- Date(1, jan, year, useLeapYears) + 1);
		}

		/*!
		 * @return if 'this' date uses leap years
		 */
		bool useLeapYears() const { return _useLeapYears; }

		void setUseLeapYears(bool useLeapYears)
		{
			_useLeapYears = useLeapYears;
      _daysInMonth = _useLeapYears ? _ldim() : _dim();
		}

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
		Date toAbsoluteDate(size_t absYear = 0,
												bool ignoreDeltaYears = false) const;

		Date toRelativeDate(int deltaYears = 0, bool useLeapYears = false,
												size_t relativeYear = defaultRelativeBaseYear)
		{
			return relativeDate(day(), month(), deltaYears, useLeapYears,
													relativeYear);
		}

		//! is the current year different from base year
		bool isRelativeBaseYear() const { return _relativeBaseYear == year(); }

		//! get the base year if this is a relative date
		size_t relativeBaseYear() const { return _relativeBaseYear; }

	private:
		//! days in month (1-indexed)
    static const std::vector<size_t>* _dim();
		//! days in month in a leap year
    static const std::vector<size_t>* _ldim();
		//! pointer to correct leap year array, depending of activated leap years
		const std::vector<size_t>* _daysInMonth{nullptr};

		//! members variables for day, month, year
		size_t _d{0}, _m{0}, _y{0};
		//! the choosen arbitrary leap year
		static const size_t _aLeapYear = 2008;

		//! member var holding whether leap years are being used or not
		bool _useLeapYears{false};

		//! choosen base relative year
		size_t _relativeBaseYear{0};

		//! is this a relative date = what's the meaning of the year
		bool _isRelativeDate{false};
	};

	inline Date fromMysqlString(const char* mysqlDateString, bool useLeapYears = true)
	{
		return Date::fromIsoDateString(mysqlDateString, useLeapYears);
	}

	inline Date fromMysqlString(const std::string& mysqlDateString, bool useLeapYears = true)
	{
		return Date::fromIsoDateString(mysqlDateString, useLeapYears);
	}

	void testDate();
}

#endif /*DATE_H_*/
