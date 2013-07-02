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

#ifndef HELPER_H_
#define HELPER_H_

#include <cmath>
#include <sstream>
#include <string>
#include <cstring>

#include "boost/algorithm/string/case_conv.hpp"
#include "loki/TypeTraits.h"

namespace Tools
{
	template<typename T, typename S>
	struct Cast { S operator()(const T& in) const { return S(in); } };

	//! Identity functor
	template<class T>
	struct Id { T operator()(const T& in) const { return in; } };

	//! functor returning the first value of a given pair
	template<class Pair>
	struct ExtractFirst
	{
		typename Pair::first_type
		operator()(const Pair& p) const { return p.first; }
	};

	//! functor returning the second value of a given pair
	template<class Pair>
	struct ExtractSecond
	{
		typename Pair::second_type
		operator()(const Pair& p) const { return p.second; }
	};

	inline double atof_comma(const char* c)
	{
		std::string s(c);
		int pos = s.find_first_of(',');
		if(pos != std::string::npos)
			return std::atof(s.replace(pos, 1, ".").c_str());

		return 0.0;
	}

	inline double atof_comma(std::string s)
  {
		int pos = s.find_first_of(',');
    if(pos != std::string::npos)
      return std::atof(s.replace(pos, 1, ".").c_str());

    return 0.0;
  }

  inline bool fuzzyIsNull(double d)
  {
    return std::abs(d) <= 0.000000000001;
  }

  inline bool fuzzyIsNull(float f)
  {
		return std::abs(f) <= 0.00001f;
  }

  inline bool fuzzyCompare(double p1, double p2)
  {
		using std::abs;
		using std::min;
		return abs(p1 - p2) <= 0.000000000001 * min(abs(p1) , abs(p2));
  }

  inline bool fuzzyCompare(float p1, float p2)
  {
		using std::abs;
		using std::min;
		return abs(p1 - p2) <= 0.00001f * min(abs(p1), abs(p2));
  }

  //! get value from map with default value if not found
  template<class Map, typename KT, typename VT>
  VT valueD(const Map& map, KT key, VT def)
  {
		typename Map::const_iterator ci = map.find(key);
		return ci == map.end() ? def : ci->second;
	}

  //! get value from map with default-constructed value if not found
  template<class Map, typename KT>
  typename Map::mapped_type value(const Map& map, KT key)
  {
    return valueD(map, key, typename Map::mapped_type());
  }

  //! short cut to value(map, key)
  template<class Map, typename KT>
  typename Map::mapped_type operator&(const Map& map, KT key)
  {
    return value(map, key);
	}

  //! get value from two nested maps with given default if nothing found
  template<class Map, typename KT, typename KT2, typename VT2>
  VT2 valueD(const Map& map, KT key, KT2 key2, VT2 def)
  {
    typename Map::const_iterator ci = map.find(key);
    if(ci != map.end())
    {
      typedef typename Map::mapped_type VT;
      typename VT::const_iterator ci2 = ci->second.find(key2);
      if(ci2 != ci->second.end())
        return ci2->second;
    }
    return def;
  }

  //! get value from two nested maps with default constructed default if not found
  template<class Map, typename KT, typename KT2>
  typename Map::mapped_type::mapped_type value(const Map& map, KT key, KT2 key2)
  {
    return valueD(map, key, key2, typename Map::mapped_type::mapped_type());
  }

  //! get value from three nested maps with given default of nothing found
  template<class Map, typename KT, typename KT2, typename KT3, typename VT3>
  VT3 valueD(const Map& map, KT key, KT2 key2, KT3 key3, VT3 def)
  {
    typename Map::const_iterator ci = map.find(key);
    if(ci != map.end())
    {
      typedef typename Map::mapped_type VT;
      typename VT::const_iterator ci2 = ci->second.find(key2);
      if(ci2 != ci->second.end())
      {
        typedef typename VT::mapped_type VT2;
        typename VT2::const_iterator ci3 = ci2->second.find(key3);
        if(ci3 != ci2->end())
          return ci3->second;
      }
    }
    return def;
  }

  //! get value from three nested maps with default constructed default if nothing found
  template<class Map, typename KT, typename KT2, typename KT3>
  typename Map::mapped_type::mapped_type::mapped_type
      value(const Map& map, KT key, KT2 key2, KT3 key3)
  {
    return valueD(map, key, key2, key3,
                  typename Map::mapped_type::mapped_type::mapped_type());
  }

	//----------------------------------------------------------------------------

	//! square the argument
	template<typename T> inline T sq(T x) { return x*x; }

	//----------------------------------------------------------------------------

	template<typename Container>
  Container range(int from, int to = 0, int step = 1)
  {
		Container c;
		for(int i = from; i <= to; i += step)
			c.insert(c.end(), typename Container::value_type(i));
		return c;
	}

	//----------------------------------------------------------------------------

	/*!
	 * create a vector out of the template argument enum and it's size
	 * enum values have to be continous and start at 0
	 */
	template<typename E, int size>
  const std::vector<E>& vectorOfEs()
  {
		static std::vector<E> v(size);
		static bool initialized = false;
    if(!initialized)
    {
      for(int i = 0; i < size; i++)
        v[i] = E(i);
			initialized = true;
		}
		return v;
	}

  //----------------------------------------------------------------------------

  template<typename T, bool isStdFundamental>
  struct ToString
  {
    std::string operator()(const T& object){ return object.toString(); }
  };

  template<typename T>
  struct ToString<T, true>
  {
    std::string operator()(const T& object)
    {
      std::ostringstream s;
      s << object;
      return s.str();
    }
  };

  template<typename T>
  struct ToString<T, false>
  {
    std::string operator()(const T& object)
    {
      std::ostringstream s;
      s << "[";
      typename T::const_iterator end = object.end();
      for(typename T::const_iterator ci = object.begin(); ci != end; ci++)
        s << *ci << (ci+1 == end ? "]" : ",");
      return s.str();
    }
  };

  template<typename T>
  std::string toString(T t)
  {
    const bool isStdF = Loki::TypeTraits<T>::isStdFundamental;
    return ToString<T, isStdF>()(t);
  }

  //----------------------------------------------------------------------------

  //makes year a multiple of second parameter
  inline int multipleOfDown(int value, int mult = 5)
  {
    return value - value % mult;
  }

  inline int multipleOfUp(int value, int mult = 5)
  {
    return value + (value % mult == 0 ? 0 : mult - value % mult);
  }

	//-----------------------------------------------------------------------

	inline std::string toLower(const std::string& arg)
	{
		return boost::to_lower_copy(arg);
	}

	inline std::string toUpper(const std::string& arg)
	{
		return boost::to_upper_copy(arg);
	}

	//-------------------------------------------------------------------------

	inline int satoi(const std::string& s){ return std::atoi(s.c_str()); }

	inline double satof(const std::string& s){ return std::atof(s.c_str()); }

}


#endif /* HELPER_H_ */
