#ifndef DATASTRUCTURES_H_
#define DATASTRUCTURES_H_

#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <set>

#include "boost/foreach.hpp"

#include "util/helper.h"

namespace Util
{
	//! just to impose some structure on 4 values
	template<typename T>
	struct Quadruple 
  {
		Quadruple() {}
		Quadruple(const T& tl, const T& tr, const T& br, const T& bl)
		: tl(tl), tr(tr), br(br), bl(bl) {}
		Quadruple(const std::vector<T>& v)
		: tl(v.at(0)), tr(v.at(1)), br(v.at(2)), bl(v.at(3)) {}

		//! this one works with QVector
    //template<template<class> class Collection>
    //Collection<T> asTlTrBrBl() const {
    //	Collection<T> v;
    //	v.push_back(tl); v.push_back(tr); v.push_back(br); v.push_back(bl);
    //	return v;
    //}

		//! this is really stupid as std::vectors default template argument must be matched
    //template<template<class, class> class Collection>
    //Collection<T, std::allocator<T> > asTlTrBrBl() const {
    //	Collection<T, std::allocator<T> > v;
    //	v.push_back(tl); v.push_back(tr); v.push_back(br); v.push_back(bl);
    //	return v;
    //}

		T tl, tr, br, bl;
	};

  template<typename T>
  std::vector<T> asTlTrBrBlVector(const Quadruple<T>& q)
  {
    std::vector<T> v;
    v.push_back(q.tl); v.push_back(q.tr); v.push_back(q.br); v.push_back(q.bl);
    return v;
  }

  template<class Collection>
  Collection asTlTrBrBl(const Quadruple<typename Collection::value_type>& q)
  {
    Collection v;
    v.push_back(q.tl); v.push_back(q.tr); v.push_back(q.br); v.push_back(q.bl);
    return v;
  }

	//----------------------------------------------------------------------------

	template<typename T>
  class StdMatrix
  {
	public:
		StdMatrix() {}
    StdMatrix(unsigned int noOfRows, unsigned int noOfCols, T init = T())
    : _m(noOfRows, std::vector<T>(noOfCols, init)) {}

		std::vector<T>& operator[](unsigned int row){ return _m[row]; }

    const std::vector<T>& operator[](unsigned int row) const
    {
			return _m.at(row);
		}

		T& valueAt(unsigned int row, unsigned int col){ return _m[row][col]; }

		const T& valueAt(unsigned int row, unsigned int col) const {
			return _m.at(row).at(col);
		}

		unsigned rows() const { return _m.size(); }
		unsigned cols() const { return _m.size() > 0 ? _m.front().size() : 0; }

    void resize(unsigned int rows, unsigned int cols, T init = T()){
      _m.resize(rows, std::vector<T>(cols, init));
		}

		bool isEmpty() const { return _m.empty(); }

	private:
		std::vector<std::vector<T> > _m;
	};

  //----------------------------------------------------------------------------

  template<typename T>
  class SparseMatrix
  {
  public:
    typedef std::map<int, T> ColType;
    typedef std::map<int, std::map<int, T> > MatType;

		SparseMatrix(T initValue = T()) : _m(), _initValue(initValue) {}

    T valueAt(unsigned int row, unsigned int col, T def = T()) const
    {
      return valueD(_m, row, col, def);
    }

    void setValueAt(unsigned int row, unsigned int col, T value)
    {
      _m[row][col] = value;
    }

    unsigned rows() const { return _m.size(); }

    //! max number of cols
    unsigned cols() const
    {
      std::set<int> s;
      s.insert(0);
      BOOST_FOREACH(typename MatType::value_type p, _m)
      {
        s.insert(p.second.size());
      }
      return *s.rbegin();
    }

    bool isEmpty() const { return _m.empty(); }

  private:
    MatType _m;
    T _initValue;
  };

	//----------------------------------------------------------------------------

	template<typename T>
	class Point 
  {
	public:
		Point() : _x(0), _y(0) {}
		Point(T x, T y) : _x(x), _y(y) {}

		//! for usage in maps
		bool operator<(const Point<T>& other) const {
			return _x < other._x ? true : _x == other._x ? _y < other._y : false;
		}

		bool operator==(const Point<T>& other) const {
			return _x == other._x && _y == other._y;
		}

		std::string toString() const;

		inline T x() const { return _x; }
		inline void setX(T x){ _x = x; }

		inline T y() const { return _y; }
		inline void setY(T y){ _y = y; }

	private: //state
		T _x, _y;
	};

	/*
		template<typename T> struct H {};

		template<> struct H<int> { typedef long ReturnType; };

		template<> struct H<double> { typedef std::string ReturnType; };

		//! a point class for integer or double points which can be used in maps
		template<typename T>
		class Point {
			template<typename R> struct Hash {};

			//typedef T PointComponentType;

		public:
			Point() : _x(0), _y(0) {}
			Point(T x, T y) : _x(x), _y(y) {}

			bool operator<(const Point<T>& other) const;

			bool operator==(const Point<T>& other) const;

			std::string toString() const;

			inline T x() const { return _x; }
			inline void setX(T x){ _x = x; }

			inline T y() const { return _y; }
			inline void setY(T y){ _y = y; }

		private:
			template<int intSize, int longSize>
			typename H<T>::ReturnType cmpHash() const;

		private: //state
			T _x, _y;
		};

		template<> template<>
		H<int>::ReturnType Point<int>::cmpHash<4,8>() const;

		template<> template<>
		H<int>::ReturnType Point<int>::cmpHash<4,4>() const;

		template<> template<int, int>
		H<double>::ReturnType Point<double>::cmpHash() const;
	 */

	//----------------------------------------------------------------------------

	template<class Point>
	struct TableCellRole 
  {
		Point* self() { return static_cast<Point*>(this); }

		const Point* self() const { return static_cast<const Point*>(this); }

		int row() const { return self()->y(); }
		//typename Point::PointComponentType row() const;
		void setRow(int r){ self()->setY(r); }

		int col() const { return self()->x(); }
		//typename Point::PointComponentType col() const;
		void setCol(int c){ self()->setX(c); }
	};

	//----------------------------------------------------------------------------

	//! a point with accessors expected in a grid or table
	struct GridPoint : public Point<int>, public TableCellRole<GridPoint> 
  {
		//! default constructor for value type use
		GridPoint(){}
		//! grid/table like constructor
		GridPoint(int row, int col){
			setRow(row); setCol(col);
		}
	};

	//------------------------------------------------------------------------------
	//implementations---------------------------------------------------------------
	//------------------------------------------------------------------------------

	/*
	template<typename T> template<int, int>
	typename Tools::H<T>::ReturnType Tools::Point<T>::cmpHash() const {
		typedef typename H<T>::ReturnType RT;
		return RT(_x & 0xFFFFFFFF) << 32 | RT(_y & 0xFFFFFFFF);
	}

	template<> template<>
	inline Tools::H<int>::ReturnType Tools::Point<int>::cmpHash<4,8>() const {
		return long(_x) << 32 | long(_y);
	}

	template<> template<>
	inline Tools::H<int>::ReturnType Tools::Point<int>::cmpHash<4,4>() const {
		return long(_x & 0xFFFF) << 16 | long(_y & 0xFFFF);
	}

	template<> template<int, int>
	inline Tools::H<double>::ReturnType Tools::Point<double>::cmpHash() const {
		return toString();
	}
	 */

	template<typename T>
	std::string Util::Point<T>::toString() const {
		std::ostringstream s;
		s << _x << "x" << _y;
		return s.str();
	}

	/*
	template<typename T>
	bool Tools::Point<T>::operator<(const Tools::Point<T>& other) const {
		return cmpHash<sizeof(int), sizeof(long)>() <
		other.cmpHash<sizeof(int), sizeof(long)>();
	}

	template<typename T>
	bool Tools::Point<T>::operator==(const Point<T>& other) const {
		return _x == other._x && _y == other._y;
	}
	 */

}

#endif
