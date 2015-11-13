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

#ifndef CLOCKSPEC_H_
#define CLOCKSPEC_H_

#include <string>
#include <list>
#include <map>
#include <vector>

namespace Models
{
	//! region, either uecker g8 or weisseritz g18 (saxony)
	enum Region {g8 = 8, g18 = 18};

  struct TimePeriod
  {
		TimePeriod() : fromYear(0), toYear(0) {}
		TimePeriod(int fy, int ty) : fromYear(fy), toYear(ty) {}
    TimePeriod(const std::pair<int, int>& other)
    {
			fromYear = other.first;
			toYear = other.second;
		}
    int refYear() const
    {
      return fromYear + int(double(toYear - fromYear)/2.0);
    }

    bool operator<(const TimePeriod& other) const
    {
			return fromYear < other.fromYear && toYear < other.toYear;
		}

    bool operator==(const TimePeriod& other) const
    {
			return fromYear == other.fromYear && toYear == other.toYear;
		}

    std::pair<int, int> toPair(){ return std::make_pair(fromYear, toYear); }

    std::string toString() const;

		int fromYear;
		int toYear;
	};

	typedef std::pair<std::string, TimePeriod> DescTimePeriodPair;
	typedef std::vector<DescTimePeriodPair> TimePeriodsList;
	TimePeriodsList allTimePeriodsBetween(int startYear = 1961,
	                                      int endYear = 2050,
	                                      int periodLength = 30,
	                                      int stepSize = 5);

	//----------------------------------------------------------------------------

	enum RingType { monthRing, seasonRing, standardRing	};

  struct Color
  {
		Color() : r(0), g(0), b(0) {}
		Color(int r, int g, int b) : r(r), g(g), b(b) {}
		int r,g,b;

    bool operator==(const Color& other) const
    {
			return r == other.r && g == other.g && b == other.b;
		}

		std::string toString() const;

		static Color black(){ return Color(0,0,0); }
		static Color white(){ return Color(255,255,255); }
	};

	//----------------------------------------------------------------------------

  struct SegmentSpec
  {
    SegmentSpec()
    : prototypeId(-1), start(0), noOfSlots(0), sigma(-1), isRemainder(false) { }

    SegmentSpec(int prototypeId, int start, int noOfSlots,
                const std::string& label, const std::string& desc,
                Color color, bool isRemainder = false)
    : prototypeId(prototypeId), start(start), noOfSlots(noOfSlots), sigma(-1),
    label(label), desc(desc),
		color(color), isRemainder(isRemainder) {}

    SegmentSpec(int prototypeId, int start, double sigma,
                const std::string& label, const std::string& desc, Color color)
		: prototypeId(prototypeId), start(start), noOfSlots(0), sigma(sigma), label(label),
    desc(desc), color(color), isRemainder(false) { }

		bool operator==(const SegmentSpec& other) const;

    bool operator!=(const SegmentSpec& other) const
    {
			return !(*this == other);
    }

    //! is the segment empty
    bool isEmpty() const { return noOfSlots == 0; }

    //! has this segment a standard deviation
    bool hasSigma() const { return sigma >= 0; }

		std::string toString() const;

		/*!
     * this should be unique for segments with the same conceptual prototype
     * this is not enforced right now
		*/
		int prototypeId;
    //! start at that slot
		int start;
    //! no of slots segment consists of
		int noOfSlots;
    //! standard deviation
		double sigma;
    //! name of segment
		std::string label;
    //! description of segment
		std::string desc;
    //! color of segment
		Color color;
    //! is the segment a remainder segment to make a full circle
		bool isRemainder;
	};

	//----------------------------------------------------------------------------

  struct RingSpec
  {
		RingSpec() : type(standardRing), noOfSlots(0), innerRadius(0),
		outerRadius(0) {}

    RingSpec(RingType t, int nos, const std::string& l = std::string())
		: type(t), noOfSlots(nos), innerRadius(0), outerRadius(0),
		label(l) {}

		bool operator==(const RingSpec& other) const;

		bool operator!=(const RingSpec& other) const { return !(*this == other); }

		std::string toString() const;

		RingSpec cloneWithRadi(double inner, double outer) const;

		RingSpec& withRadi(double inner, double outer);

		std::vector<SegmentSpec> segments;
		RingType type;
		int noOfSlots; //no of slots per ring
		double innerRadius;
		double outerRadius;
		std::string label;
	};

	//----------------------------------------------------------------------------

	typedef double OuterRadius;

  struct ClockSpec
  {
		ClockSpec() : compareRingIndex(2), referenceRingIndex(3) {}
		std::vector<RingSpec> rings;
		std::map<int, std::vector<RingSpec> > altRings;

    std::vector<RingSpec>& compareAltRings()
    {
      return altRings[compareRingIndex];
		}
		std::vector<RingSpec> compareAltRings() const;

    std::vector<RingSpec>& referenceAltRings()
    {
			return altRings[referenceRingIndex];
		}
		std::vector<RingSpec> referenceAltRings() const;

		RingSpec compareRing(){ return rings.at(compareRingIndex); }

		RingSpec referenceRing(){ return rings.at(referenceRingIndex); }

		std::string toString() const;

		int compareRingIndex;
		int referenceRingIndex;
	};

	std::vector<Color> colorRange(int n);

	RingSpec standardSeasonRingSpec();

	RingSpec standardMonthRingSpec();
}

#endif
