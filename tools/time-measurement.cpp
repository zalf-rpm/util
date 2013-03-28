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

#include <iostream> //cout
#ifdef WIN32
#include <windows.h>
#include <time.h>
#define sleep(x) Sleep((x)*1000)
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "time-measurement.h"

using namespace std;

#ifdef WIN32
//taken from http://www.cpp-programming.net/c-tidbits/gettimeofday-function-for-windows/
#include <time.h>
 
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if (NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		/*converting file time to unix epoch*/
		tmpres /= 10;  /*convert into microseconds*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}
#endif

/*
namespace {
  //Keep track of most recent reading of cycle counter
  static unsigned cyc_hi = 0;
  static unsigned cyc_lo = 0;

  double MHZ = 3192.5;
  //int CMIN = 5000000; //minimal cycles for cycleCount2

  
  //works only on gcc and x86
  void access_counter(unsigned* hi, unsigned* lo){
    //Get cycle counter
    asm("rdtsc; movl %%edx,%0; movl %%eax,%1"
		: "=r" (*hi), "=r" (*lo)
		: //No input
		: "%edx", "%eax");
  }

  double get_counter(){
    unsigned ncyc_hi, ncyc_lo;
    unsigned hi, lo, borrow;
    //Get cycle counter
    access_counter(&ncyc_hi, &ncyc_lo);
    //Do double precision subtraction
    lo = ncyc_lo - cyc_lo;
    borrow = lo > ncyc_lo;
    hi = ncyc_hi - cyc_hi - borrow;
    return (double) hi * (1 << 30) * 4 + lo;
  }

  void start_counter(){
    //Get current value of cycle counter
    access_counter(&cyc_hi, &cyc_lo);
  }

  double getCpuClockSpeed(){
    int sleep_time = 10;
    start_counter();
    sleep(sleep_time);
    return get_counter()/(sleep_time * 1e6);
  }
}


void Tools::startMeasurementViaCPUCycles(){
	start_counter();
}

double Tools::getSecondsSinceLastStartViaCPUCycles(){
	return get_counter() / (MHZ * 1e6);
}

double Tools::getExecutionTimeViaMeanCPUCycles(void (*fun)(), int minimumCycles){
	int cnt = 1;
  double cmeas = 0;
  double cycles;
  do {
    int c = cnt;
    fun(); //Warm up cache
    start_counter();
    while (c-- > 0){
			fun();
    }
    cmeas = get_counter();
    cycles = cmeas / cnt;
    cnt += cnt;
  } while (cmeas < minimumCycles);  // Make sure have enough
  cout << "cycles: " << cycles << endl;
  cout << "cmeas: " << cmeas << endl;
  cout << "cnt: " << cnt << endl;
  return cycles / (1e6 * MHZ);
}
*/

timeval Tools::startMeasurementViaTimeOfDay()
{
	timeval tstart;
	gettimeofday(&tstart, NULL);
	return tstart;
}

double Tools::stopMeasurementViaTimeOfDay(timeval startTime)
{
	timeval tfinish;
	gettimeofday(&tfinish, NULL);
	return (tfinish.tv_sec - startTime.tv_sec) +
	((tfinish.tv_usec - startTime.tv_usec) / 1e6);
}

double Tools::getExecutionTimeMs(void (*fun)())
{
	return getExecutionTime(fun) * 1000;
}

double Tools::getExecutionTime(void (*fun)())
{
	timeval before = startMeasurementViaTimeOfDay();
	fun();
	return stopMeasurementViaTimeOfDay(before);
}


