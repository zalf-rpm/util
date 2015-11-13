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

#ifndef _TIME_MEASUREMENT_H
#define _TIME_MEASUREMENT_H

#include <ctime> //timeval
#include <utility>

namespace Tools
{
	/**
	 * this function has to be called before any real measurement, as
	 * it takes by default 10 seconds to measure the clockspeed of the cpu
	 */
	//double measureAndSetCpuClockSpeed();
	/**
	 * - will measure the time (in conjuction with the function
	 * "getSecondsSinceLastStartViaCPUCycles"
	 * with direct code for measuring the CPU cycles (instead of done by the OS)
	 * - but works only on x86 with gcc
	 */
	//void startMeasurementViaCPUCycles();
	/**
	 * get the time elapsed since last call to according start function
	 */
	//double getSecondsSinceLastStartViaCPUCycles();
	/**
	 * similar function to before, but measures the execution time via CPU
	 * cycles of the given function repeatedly, so long that at least the
	 * given amount of cycles has been reachead
	 */
	//double getExecutionTimeViaMeanCPUCycles(void (*fun)(),
	//                                        int minimumCycles = 50000);

	/**
	 * - start the measurement via the OS time of day implementation
	 * - on linux measures via cycle counting like the functions above
	 */
//	std::timeval startMeasurementViaTimeOfDay();
	/**
	 * get the difference between the given start time and the measured
	 * (via time of day) current time in seconds
	 */
//	double stopMeasurementViaTimeOfDay(timeval startTime);
	/**
	 * get the execution time of the given function in milliseconds
	 */
//	double getExecutionTimeMs(void (*fun)());
	/**
	 * get the execution tim of the given function in seconds
	 */
//	double getExecutionTime(void (*fun)());

//	template<typename Class, typename T>
//	std::pair<T, double> getExecutionTimeMS(Class* self, T (Class::*method)());

//	template<typename Class, typename T>
//	std::pair<T, double> getExecutionTime(Class* self, T (Class::*method)());
}

//template<typename Class, typename T>
//std::pair<T, double> Tools::
//getExecutionTimeMS(Class* self, T (Class::*method)())
//{
//	std::pair<T, double> res = getExecutionTime(self, method);
//	return std::make_pair(res.first, res.second * 1000);
//}

//template<typename Class, typename T>
//std::pair<T, double> Tools::
//getExecutionTime(Class* self, T (Class::*method)())
//{
//	timeval before = startMeasurementViaTimeOfDay();
//	T result = (self->*method)();
//	return std::make_pair(result, stopMeasurementViaTimeOfDay(before));
//}

#endif
