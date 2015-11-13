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

#ifndef HELPER_H_
#define HELPER_H_

#include <cmath>
#include <sstream>
#include <string>
#include <cstring>
#include <locale>
#include <vector>
#include <algorithm>

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

  inline double stod_comma(std::string s)
  {
    size_t pos = s.find_first_of(',');
    if(pos != std::string::npos)
      return std::stod(s.replace(pos, 1, "."));

    return std::stod(s);
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

//  template<typename T, bool isStdFundamental>
//  struct ToString
//  {
//    std::string operator()(const T& object){ return object.toString(); }
//  };

//  template<typename T>
//  struct ToString<T, true>
//  {
//    std::string operator()(const T& object)
//    {
//      std::ostringstream s;
//      s << object;
//      return s.str();
//    }
//  };

//  template<typename T>
//  struct ToString<T, false>
//  {
//    std::string operator()(const T& object)
//    {
//      std::ostringstream s;
//      s << "[";
//      typename T::const_iterator end = object.end();
//      for(typename T::const_iterator ci = object.begin(); ci != end; ci++)
//        s << *ci << (ci+1 == end ? "]" : ",");
//      return s.str();
//    }
//  };

//  template<typename T>
//  std::string toString(T t)
//  {
//    const bool isStdF = Loki::TypeTraits<T>::isStdFundamental;
//    return ToString<T, isStdF>()(t);
//  }

  template<typename T>
  std::string toString(T t, std::string indent = std::string(), bool detailed = false) { return t.toString(indent, detailed); }

  template<typename T>
  std::string toString(T* t, std::string indent = std::string(), bool detailed = false){ return t->toString(indent, detailed); }

//  template<typename Container>
//  std::string toString(const Container& c)
//  {
//      std::ostringstream s;
//      s << "[";
//      typename Container::const_iterator end = c.end();
//      for(typename Container::const_iterator ci = c.begin(); ci != end; ci++)
//        s << *ci << (ci+1 == end ? "]" : ",");
//      return s.str();
//  }

  inline std::string toString(int i){ std::ostringstream s; s << i; return s.str(); }

  inline std::string toString(double d){ std::ostringstream s; s << d; return s.str(); }

  inline std::string toString(float f){ std::ostringstream s; s << f; return s.str(); }

  inline std::string toString(bool b){ std::ostringstream s; s << b; return s.str(); }

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

  inline std::string toLower(const std::string& str)
	{
    std::locale loc;
    std::string lowerStr;
    for(auto elem : str)
      lowerStr.append(1, std::tolower(elem,loc));
    return lowerStr;
	}

  inline std::string toUpper(const std::string& str)
	{
    std::locale loc;
    std::string upperStr;
    for(auto elem : str)
      upperStr.append(1, std::toupper(elem,loc));
    return upperStr;
	}

	//-------------------------------------------------------------------------

	inline int satoi(const std::string& s, int def = 0)
	{
    return s.empty() ? def : std::stoi(s);
	}

	inline double satof(const std::string& s, double def = 0.0)
	{
    return s.empty() ? def : std::stof(s);
	}

  bool satob(const std::string& s, bool def = false);

	inline bool stob(const std::string& s, bool def = false) { return satob(s, def); }

	inline std::string surround(std::string with, std::string str){ return with + str + with;  };

	std::string fixSystemSeparator(std::string path);

  void ensureDirExists(std::string& path);
}


#endif /* HELPER_H_ */
