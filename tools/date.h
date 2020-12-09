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

#include <string>
#include <vector>
#include "common/common-typedefs.h"

#ifdef CAPNPROTO_SERIALIZATION_SUPPORT
#include "date.capnp.h"
#endif

#ifdef DSS_NO_LEAP_YEAR_BY_DEFAULT
#define DEFAULT_USE_LEAP_YEARS false
#else
#define DEFAULT_USE_LEAP_YEARS true
#endif

namespace Tools
{
	//enum Month
	//{
	//	jan = 1, feb, mar, apr, may, jun, jul, aug, sep, oct, nov, dec
	//};

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
		//static const uint defaultRelativeBaseYear = 2000;

	public:
		Date(bool useLeapYears = DEFAULT_USE_LEAP_YEARS);

		Date(const std::string& isoDateString,
				 bool useLeapYears = DEFAULT_USE_LEAP_YEARS);

		//explicit Date(uint day,
		//							Month month,
		//							int year,
		//							bool createValidDate = false,
		//							bool useLeapYears = DEFAULT_USE_LEAP_YEARS);

		Date(uint day,
				 uint month,
				 int year,
				 bool isRelativeDate = false,
				 bool createValidDate = false,
				 bool useLeapYears = DEFAULT_USE_LEAP_YEARS);

#ifdef CAPNPROTO_SERIALIZATION_SUPPORT
		Date(mas::common::Date::Reader reader) { deserialize(reader); }

		void serialize(mas::common::Date::Builder builder) const;

		void deserialize(mas::common::Date::Reader reader);
#endif

		static Date relativeDate(uint day,
														 uint month,
														 int deltaYears = 0,
														 bool useLeapYears = DEFAULT_USE_LEAP_YEARS);

		static inline Date julianDate(uint julianDay,
																	uint year,
																	bool isRelativeDate = false,
																	bool useLeapYears = DEFAULT_USE_LEAP_YEARS)
		{
			return Date(1, 1, year, isRelativeDate, false, useLeapYears)
					+ (julianDay - 1);
		}

		static Date fromIsoDateString(const std::string& isoDateString,
																	bool useLeapYears = DEFAULT_USE_LEAP_YEARS);

		Date(const Date& other);

		/*!
		 * @return true if this is a valid date (not default constructed)
		 */
		bool isValid() const { return _d > 0; }

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

		Date operator-(uint days) const;

		/*!
		 * @param days
		 * @return change 'this' date to 'days' before
		 */
		Date & operator-=(uint days) { return (*this) = (*this) - days; }

		/*!
		 * prefix --
		 * @see Date& operator-=(int days)
		 */
		Date & operator--() { return operator-=(1); }

		Date operator--(int);

		Date operator+(uint days) const;

		/*!
		 * @param days
		 * @return 'this' date changed to 'days' ahead
		 */
		Date & operator+=(uint days) { return (*this) = (*this) + days; }

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
		bool isLeapYear() const;

		/*!
		 * @param month
		 * @return how many days has the argument month
		 */
		uint daysInMonth(uint month = 0) const
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
		static uint daysInMonth(uint year,
															uint month,
															bool useLeapYears = DEFAULT_USE_LEAP_YEARS)
		{
			return Date(1, 1, year, false, false, useLeapYears).daysInMonth(month);
		}


		int numberOfDaysTo(const Date& toDate) const;

		/*!
		 * @return 'this' dates day
		 */
		uint day() const { return _d; }

		/*!
		 * set 'this' dates day
		 * @param day
		 */
		void setDay(uint day,
								bool createValidDate = false);

		Date withDay(uint day, bool createValidDate = false);

		/*!
		 * @return 'this' dates month
		 */
		uint month() const { return _m; }

		Date withMonth(uint month, bool createValidDate = false);

		/*!
		 * set 'this' dates month
		 * @param month
		 */
		void setMonth(uint month, 
									bool createValidDate = false);

		/*!
		 * @return 'this' dates year
		 */
		int year() const { return _y; }

		/*!
		 * set 'this' dates year
		 * @param year
		 */
		void setYear(uint year) { _y = year; }

		Date withYear(uint year);

		void addYears(int years) { setYear(year() + years); }

		Date withAddedYears(int years) const;

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
		static int aLeapYear() { return _aLeapYear; }

		static int notALeapYear() { return aLeapYear() - 1; }

		/*!
		 * @ see dayInYear(int year, int day, int month, bool useLeapYears)
		 * @return julian day of day in 'this' date
		 */
		uint julianDay() const
		{
			return dayInYear(year(), day(), month(), _useLeapYears);
		}

		/*!
		 * @see julianDay()
		 * @return day in year of day in 'this' date
		 */
		uint dayOfYear() const { return julianDay(); }

		/*!
		 * @param day
		 * @param month
		 * @return day in year of argument day in argument month in 'this' dates year
		 */
		uint dayInYear(uint day, uint month) const
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
		static uint dayInYear(uint year,
														uint day,
														uint month,
														bool useLeapYears = DEFAULT_USE_LEAP_YEARS)
		{
			return uint(Date(day, month, year, false, false, useLeapYears)
				- Date(1, 1, year, false, false, useLeapYears) + 1);
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

		bool isAbsoluteDate() const { return !_isRelativeDate; }

		//! convert this year to an absolute year
		Date toAbsoluteDate(uint absYear,
												bool ignoreDeltaYears = false) const;

		Date toRelativeDate(int deltaYears = 0,
												bool useLeapYears = DEFAULT_USE_LEAP_YEARS)
		{
			return relativeDate(day(), month(), deltaYears, useLeapYears);
		}

		bool useLeapYears() const { return _useLeapYears; }

		void setUseLeapYears(bool useLeapYears)
		{
			_useLeapYears = useLeapYears;
			_daysInMonth = _useLeapYears ? _ldim() : _dim();
		}

	private:
		//! days in month (1-indexed)
		static const std::vector<uint>* _dim();
		//! days in month in a leap year
    static const std::vector<uint>* _ldim();
		//! pointer to correct leap year array, depending of activated leap years
		const std::vector<uint>* _daysInMonth{nullptr};

		//! members variables for day, month, year
		uint _d{0}, _m{0};
		int _y{0};
		//! the choosen arbitrary leap year
		static const int _aLeapYear = 2008;

		//! member var holding whether leap years are being used or not
		bool _useLeapYears{DEFAULT_USE_LEAP_YEARS};

		//! is this a relative date = what's the meaning of the year
		bool _isRelativeDate{false};
	};

	inline Date fromMysqlString(const char* mysqlDateString,
															bool useLeapYears = true)
	{
		return Date::fromIsoDateString(mysqlDateString, useLeapYears);
	}

	inline Date fromMysqlString(const std::string& mysqlDateString,
															bool useLeapYears = true)
	{
		return Date::fromIsoDateString(mysqlDateString, useLeapYears);
	}

	void testDate();
}

#endif /*DATE_H_*/
