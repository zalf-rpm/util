#include <sstream>
#include <cmath>
#include <vector>

#include "clock-spec.h"
#include "tools/algorithms.h"

using namespace Models;
using namespace std;
using namespace Tools;

TimePeriodsList Models::allTimePeriodsBetween(int start, int end,
                                              int periodLength, int stepSize)
{
	TimePeriodsList ps;
	for(int i = start, k = start + (periodLength - 1);
  i <= (end - (periodLength - 1)); i += stepSize, k += stepSize)
  {
		ostringstream s; s << (i+int(double(periodLength)/2.0)-1);
		ps.push_back(make_pair(s.str(), TimePeriod(i, k)));
	}
	return ps;
}

//------------------------------------------------------------------------------

std::string TimePeriod::toString() const
{
  std::ostringstream s;
  s << fromYear << "-" << toYear << " (ref: " << refYear() << ")";
  return s.str();
}

//------------------------------------------------------------------------------


string Color::toString() const
{
	ostringstream s; s
	<< "r: " << r << " g: " << g << " b: " << b;
	return s.str();
}

//------------------------------------------------------------------------------

bool SegmentSpec::operator==(const SegmentSpec& other) const
{
	return prototypeId == other.prototypeId
	&& start == other.start && noOfSlots == other.noOfSlots
	&& label == other.label && desc == other.desc && color == other.color
	&& isRemainder == other.isRemainder
	&& abs(sigma) - abs(other.sigma) < 0.0001;
}

string SegmentSpec::toString() const
{
	ostringstream s; s
	<< "SegmentSpec\nstart: " << start << ", "
	<< "noOfSlots: " << noOfSlots << ", "
	<< "label: " << label << ", "
	<< "desc: " << desc << ", "
	<< "color: " << color.toString() << ", "
	<< "isRemainder: " << (isRemainder ? "true" : "false") << ", "
	<< "sigma: " << sigma << "\n";
	return s.str();
}

//------------------------------------------------------------------------------

bool RingSpec::operator==(const RingSpec& other) const
{
	bool t = type == type && noOfSlots == other.noOfSlots
	&& abs(innerRadius) - abs(other.innerRadius) < 0.0001
	&& abs(outerRadius) - abs(other.outerRadius) < 0.0001
	&& label == other.label;
	if(t)
		return segments == other.segments;
	return false;
}

string RingSpec::toString() const
{
	ostringstream s; s
	<< "RingSpec\nslots in ring: " << noOfSlots << "\n"
	<< "innerRadius: " << innerRadius << ", "
	<< "outerRadius: " << outerRadius << "\n";

	for(vector<SegmentSpec>::const_iterator ci = segments.begin();
	ci != segments.end(); ci++)
		s << ci->toString();

	return s.str();
}

RingSpec RingSpec::cloneWithRadi(double inner, double outer) const
{
	RingSpec clone(*this);
	clone.innerRadius = inner;
	clone.outerRadius = outer;
	return clone;
}

RingSpec& RingSpec::withRadi(double inner, double outer)
{
	innerRadius = inner;
	outerRadius = outer;
	return *this;
}

//------------------------------------------------------------------------------

string ClockSpec::toString() const
{
	ostringstream s; s
	<< "ClockSpec:\nrings:\n";

  for(RingSpec rs : rings)
  {
    s << rs.toString();
  }

	s << "compareAltRings:\n";
  for(RingSpec rs : compareAltRings())
  {
    s << rs.toString();
  }

	s << "referenceAltRings:\n";
  for(RingSpec rs : referenceAltRings())
  {
    s << rs.toString();
  }

	return s.str();
}

vector<RingSpec> ClockSpec::compareAltRings() const
{
	map<int, vector<RingSpec> >::const_iterator ci =
		altRings.find(compareRingIndex);
	return ci == altRings.end() ? vector<RingSpec>() : ci->second;
}

vector<RingSpec> ClockSpec::referenceAltRings() const
{
	map<int, vector<RingSpec> >::const_iterator ci =
		altRings.find(referenceRingIndex);
	return ci == altRings.end() ? vector<RingSpec>() : ci->second;
}

//------------------------------------------------------------------------------

vector<Color> Models::colorRange(int n)
{
	vector<Color> cs;

	double step = 1.0 / (n - 1); //creates n points

	double e = 0.0001;
  for(double i = 0; i < 1 + e && step > 0; i+=step)
  {
		cs.push_back
		(i >= 0.5 ? Color(int((2-(2*bound(0.5, i, 1.0)))*255), 1*255, 0*255)
		          : Color(1*255, int((2*bound(0.0, i, 0.5))*255), 0*255));
	}

	return cs;
}

RingSpec Models::standardSeasonRingSpec()
{
	RingSpec ring(seasonRing, 365);

	ring.segments.push_back
	(SegmentSpec(0, -31, 90, "", "Winter", Color::black()));
	ring.segments.push_back
	(SegmentSpec(1, 59, 92, "", "Frühling", Color::white()));
	ring.segments.push_back
	(SegmentSpec(2, 151, 92, "", "Sommer", Color::black()));
	ring.segments.push_back
	(SegmentSpec(3, 243, 91, "", "Herbst", Color::white()));

	return ring;
}

RingSpec Models::standardMonthRingSpec()
{
	RingSpec ring(monthRing, 365);

	vector<string> names(12);
	names[0] = "Jan."; names[1] = "Feb."; names[2] = "März";
	names[3] = "April"; names[4] = "Mai"; names[5] = "Juni";
	names[6] = "Juli"; names[7] = "Aug."; names[8] = "Sep.";
	names[9] = "Okt."; names[10] = "Nov."; names[11] = "Dez.";

	vector<int> days(12);
	days[0] = 31; days[1] = 28; days[2] = 31;
	days[3] = 30; days[4] = 31; days[5] = 30;
	days[6] = 31; days[7] = 31; days[8] = 30;
	days[9] = 31; days[10] = 30; days[11] = 31;

	for(int i = 0, acc = 0; i < 12; i++){
		ring.segments.push_back(SegmentSpec(i, acc, days.at(i), names.at(i),
		                                    names.at(i), Color::white()));
		acc += days.at(i);
	}

	return ring;
}

