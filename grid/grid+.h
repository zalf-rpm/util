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

#ifndef GRIDPLUS_H_
#define GRIDPLUS_H_

#include <vector>
#include <functional>
#include <iostream>

#define LOKI_OBJECT_LEVEL_THREADING

#include "loki/Threads.h"

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/lambda/if.hpp>

#include "tools/stl-algo-boost-lambda.h"

#include "grid.h"
#include "tools/coord-trans.h"
#include "tools/algorithms.h"
#include "tools/datastructures.h"

namespace Grids
{
grid* loadGrid(const std::string& gridName,
							 const std::string& pathToHdf = std::string());

void printGridFields(const grid& g, bool onlyDataFields = true);

void setAllGridFieldsTo(grid* g, double newValue, bool keepNodata = true);

//----------------------------------------------------------------------------

struct GK5Rect
{
	GK5Rect() {}
	GK5Rect(const Tools::GK5Coord& tl, const Tools::GK5Coord& br)
		: tl(tl), br(br) {}

	bool contains(const Tools::GK5Coord& p,
								bool exclusiveBottomRightBorder = false) const
	{
		return exclusiveBottomRightBorder ?
          (tl.r <= p.r && p.r < br.r) && (br.h < p.h && p.h <= tl.h) :
          (tl.r <= p.r && p.r <= br.r) && (br.h <= p.h && p.h <= tl.h);
	}

	bool contains(const GK5Rect& other) const;

	bool intersects(const GK5Rect& other) const;

	GK5Rect intersected(const GK5Rect& other) const;

	bool isEmpty() const
	{
		return ((br.r - tl.r) <= 0. || (tl.h - br.h) <= 0.);
	}

	bool isNull() const
	{
		return ((br.r - tl.r) <= 0. && (tl.h - br.h) <= 0.);
	}

	std::string toString() const;

	std::vector<Tools::GK5Coord> toTlTrBrBlVector() const;

	Tools::GK5Coord tl, br;
};

//----------------------------------------------------------------------------

class GridP;

//! metadata common to all grids
struct GridMetaData
{
	GridMetaData();

	GridMetaData(const grid* g);

	GridMetaData(const GridP* g);

	bool isValid() const { return ncols > -1 && nrows > -1; }

	bool operator==(const GridMetaData& other) const;

	bool operator!=(const GridMetaData & other) const
	{
		return !(*this == other);
	}

	bool operator<(const GridMetaData& other) const
	{
		return toCanonicalString() < other.toCanonicalString();
	}

	Tools::GK5Coord topLeftCorner() const
	{
		return Tools::GK5Coord(xllcorner, yllcorner+(nrows*cellsize));
	}

	Tools::GK5Coord topRightCorner() const
	{
		return Tools::GK5Coord(xllcorner+(ncols*cellsize),
													 yllcorner+(nrows*cellsize));
	}

	Tools::GK5Coord bottomRightCorner() const
	{
		return Tools::GK5Coord(xllcorner+(ncols*cellsize), yllcorner);
	}

	Tools::GK5Coord bottomLeftCorner() const
	{
		return Tools::GK5Coord(xllcorner, yllcorner);
	}

	GK5Rect gk5Rect() const
	{
		return GK5Rect(topLeftCorner(), bottomRightCorner());
	}

	std::string toString() const;
	std::string toShortDescription() const;
	std::string toCanonicalString(const std::string& separator = " ") const;

	int ncols, nrows;
	int nodata;
	int xllcorner, yllcorner;
	int cellsize;
	std::string regionName;
};

//----------------------------------------------------------------------------

typedef unsigned int Row;
typedef unsigned int Col;
typedef unsigned int Rows;
typedef unsigned int Cols;

//! holds data needed to know where and how large the subgrid is
struct SubData
{

	SubData() : row(0), col(0), rows(0), cols(0) { }
	SubData(Row r, Col c, Rows rs, Cols cs)
		: row(r), col(c), rows(rs), cols(cs) { }

	Row row;
	Col col;
	Rows rows;
	Cols cols;

	std::string toString() const;

	bool isValid() const { return rows > 0 && cols > 0; }
};

GK5Rect
extendedBoundingRect(const GridMetaData& gmd,
										 const Tools::Quadruple<Tools::GK5Coord>& gk5poly,
										 double cellSize);

std::pair<Row, Col> rowColInGrid(const GridMetaData& gmd,
																 const Tools::GK5Coord& c);

//----------------------------------------------------------------------------

typedef boost::shared_ptr<grid> GridPtr;

class GridP;

typedef boost::shared_ptr<GridP> GridPPtr;

//!grid+ class
class GridP
{
public:
	enum FileType { HDF, ASCII };

	GridP();

	//! new grid with given size and initialized to no data
	GridP(const std::string& datasetName,
				int nrows, int ncols, float cellSize,
				double llx, double lly, float noDataValue);

	GridP(GridMetaData gmd, const std::string& datasetName = std::string());

	GridP(const std::string& datasetName,
				FileType ft, const std::string& pathToFile);

	GridP(grid* wrapThisGrid);

	//! copy constructor
	GridP(const GridP& other);

	virtual ~GridP();

	/**
	 * assignment
	 */
	GridP& operator=(const GridP& other);

	/**
	 * conversion copy constructor
	 */
	GridP(const grid& other);

	GridP(grid const *const other);

	//! copies other
	GridP& operator=(const grid& other);

	//! copies other
	GridP & operator=(grid const *const other) { return (*this) = *other; }

	bool operator==(const GridP& other) const;

	bool isValid() const
	{
		return _grid && _grid->feld && cols() > 0 && rows() > 0;
	}

	bool writeHdf(const std::string& pathToHdfFile,
								const std::string& datasetName,
								const std::string& regionName, time_t t);

	void writeAscii(const std::string& pathToAsciiFile);

	//! create clone of part of the grid
	GridP* subGridClone(int top, int left, int rows, int cols) const;

	//! create exact copy of the grid
	GridP* clone() const { return new GridP(*this); }

	//! create a structural copy, but set all fields to given emptyValue
	GridP* emptyClone(bool keepNoData = true) const
	{
		return fillClone(noDataValue(), keepNoData);
	}

	GridP* fillClone(double fillValue, bool keepNoData = true) const;

	void setDescriptiveLabel(const std::string& label)
	{
		_descriptiveLabel = label;
	}

	std::string descriptiveLabel() const;

	//! number of rows
	int rows() const { return _grid->nrows; }

	//! number of columns
	int cols() const { return _grid->ncols; }

	//! size of gridcells
	double cellSize() const { return _grid->csize; }

	int noDataValue() const { return int(_grid->nodata); }

	std::vector<double> allDataAsLinearVector() const;

	//! create histogram data
	Tools::HistogramData histogram(int noOfClasses);

	//! value frequency = map of percent of pixels -> pixel value
	std::multimap<double, double, std::greater<double> > frequency();

	GridP* setAllFieldsTo(double newValue, bool keepNoData = true);

	template<typename Collection>
	GridP* setAllFieldsWithinTo(Collection matchValues, float toNewValue, bool includeNoData = false)
	{
		for(auto cit = matchValues.begin(); cit != matchValues.end(); cit++)
			for(int r = 0, rs = rows(); r < rs; r++)
				for(int c = 0, cs = cols(); c < cs; c++)
					if((includeNoData || isDataField(r, c)) && Tools::fuzzyCompare(dataAt(r, c), *cit))
						setDataAt(r, c, toNewValue);
		return this;
	}

	GridP* setAllFieldsWithTo(float withValue, float toNewValue, bool includeNoData = false)
	{
		return setAllFieldsWithinTo(std::vector<float>(1, withValue), toNewValue, includeNoData);
	}

	GridP* setAllFieldsWithoutTo(float withoutValue, float toNewValue, bool includeNoData = false);

	GridP* setFieldsTo(const GridP* other, bool keepNoData = true);

	float dataAt(int row, int col) const { return _grid->feld[row][col]; }

	float dataAt(Tools::GK5Coord gk5c) const;

	GridP* setDataAt(int row, int col, float value)
	{
		_grid->feld[row][col] = value;
		return this;
	}

	float* operator[](int row){ return _grid->feld[row]; }

	GridP* setDataAt(Tools::GK5Coord gk5c, float value);

	GridP* setNoDataValueAt(int row, int col)
	{
		return setDataAt(row, col, float(noDataValue()));
	}

	GridP* setNoDataValueAt(Tools::GK5Coord gk5c)
	{
		return setDataAt(gk5c, float(noDataValue()));
	}

	bool isNoDataField(int row, int col) const
	{
		return int(dataAt(row, col)) == noDataValue();
	}

	bool isNoDataField(Tools::GK5Coord gk5c) const
	{
		return int(dataAt(gk5c)) == noDataValue();
	}

	bool isDataField(int row, int col) const
	{
		return !isNoDataField(row, col);
	}

	bool isDataField(Tools::GK5Coord gk5c) const
	{
		return !isNoDataField(gk5c);
	}

	std::string toString() const;

	grid& gridRef() const { return *gridPtr(); }

	grid* gridPtr() const { return _grid.get(); }

	std::string datasetName() const { return _datasetName; }

	GridP* setDatasetName(const std::string& newName)
	{
		_datasetName = newName;
		return this;
	}

	std::string unit() const { return _unit; }

	GridP* setUnit(const std::string& newUnit)
	{
		_unit = newUnit;
		return this;
	}

	GK5Rect gk5Rect() const;

	Tools::GK5Coord gk5CoordAt(int row, int col) const;

	Tools::GK5Coord gk5CoordAtCenter() const
	{
		return gk5CoordAt(int(double(rows()) / 2.0), int(double(cols()) / 2.0));
	}

	bool isCompatible(const GridP* other) const;

	std::pair<double, double> minMax() const;

	double average() const;

	GridP* transformInPlace(boost::function<float(float)> transformFunction);

	GridP* replace(float searchValue, float replaceValue)
	{
//		return transformInPlace(boost::lambda::if_then_else_return
//														(boost::lambda::_1 == searchValue,
//														 replaceValue, boost::lambda::_1));
		return transformInPlace([=](float v){ return v == searchValue
															? replaceValue : v; });
	}

	GridPPtr transform(boost::function<float(float)> transformFunction) const
	{
		return GridPPtr(transformP(transformFunction));
	}

	GridP* transformP(boost::function<float(float)> transformFunction) const;

	std::pair<int, int> gk52rowCol(Tools::GK5Coord gk5c) const;

	GridP* invert(float value);

	GridP* maskOut(const GridP* maskGrid, float byValueInMaskGrid,
								 bool keepDataAtMatchPoint = true);

	GridP* maskTo(const GridP* maskGrid, float matchMaskValueTo, float newValue,
								bool keepNoData = true);

	//! adjust this grid to match the model model, by croping or adding noData
	GridP* adjustToP(GridMetaData gmd) const;

	GridPPtr adjustTo(GridMetaData gmd) const
	{
		return GridPPtr(adjustToP(gmd));
	}

	template<typename T>
	std::set<T> uniqueValues(boost::function<T(float)> transform =
													 boost::function<T(float)>(boost::lambda::_1),
													 bool ignoreNoDataValues = true) const
	{
		return mapF<std::set<T> >(transform, ignoreNoDataValues);
	}

	template<class Container, typename T>
	Container mapF(boost::function<T(float)> transformFunc,
								 bool ignoreNoDataValues = true) const
	{
		Container cont;
		for(int r = 0, rs = rows(); r < rs; r++)
		{
			for(int c = 0, cs = cols(); c < cs; c++)
			{
				if(isNoDataField(r, c) && ignoreNoDataValues)
					continue;
				cont.insert(cont.end(), transformFunc(dataAt(r, c)));
			}
		}
		return cont;
	}

	template<typename T>
	T foldF(T init, boost::function<T(T, float)> foldFunc) const
	{
		T res = init;
		for(int r = 0, rs = rows(); r < rs; r++)
			for(int c = 0, cs = cols(); c < cs; c++)
				res = foldFunc(res, dataAt(r, c));
		return res;
	}

	void setDisplayValueTransformFunction(boost::function<std::string(double)> f)
	{
		_displayValueTransformFunction = f;
	}

	boost::function<std::string(double)> displayValueTransformFunction() const
	{
		return _displayValueTransformFunction;
	}

private:
	GridPtr _grid;
	std::string _datasetName;
	std::string _descriptiveLabel;
	std::string _unit;
	boost::function<std::string(double)> _displayValueTransformFunction;
};

template<class CollectionOfGrids>
GridP* averageP(const CollectionOfGrids& gridps)
{
	if (gridps.empty())
		return new GridP();

	double size = gridps.size();
	GridP* res = (*(gridps.begin()))->fillClone(0.0);
	for(int r = 0, rs = res->rows(); r < rs; r++)
	{
		for(int c = 0, cs = res->cols(); c < cs; c++)
		{
			if(res->isDataField(r, c))
			{
				BOOST_FOREACH(typename CollectionOfGrids::value_type g, gridps)
				{
					if(g->isNoDataField(r, c))
					{
						res->setNoDataValueAt(r, c);
						break;
					}
					res->setDataAt(r, c, float(res->dataAt(r, c) + (g->dataAt(r, c) / size)));
				}
			}
		}
	}

	return res;
}

template<class CollectionOfGrids>
GridPPtr average(const CollectionOfGrids& gridps)
{
	return GridPPtr(averageP(gridps));
}

template<class OP>
GridP& inPlaceScalarMatrixOp(GridP& left, float value, OP op)
{
	for (int r = 0, rs = left.rows(); r < rs; r++)
		for (int c = 0, cs = left.cols(); c < cs; c++)
			if (left.isDataField(r, c))
				left.setDataAt(r, c, op(left.dataAt(r, c), value));
	return left;
};

inline GridP& operator*=(GridP& left, float value)
{
	return inPlaceScalarMatrixOp(left, value, std::multiplies<float>());
}

template<class OP>
GridP& inPlaceScalarMatrixOp(GridP& left, const GridP& right, OP op)
{
	assert(left.isCompatible(&right));
	for (int r = 0, rs = left.rows(); r < rs; r++)
	{
		for (int c = 0, cs = left.cols(); c < cs; c++)
		{
			if (left.isDataField(r, c) && right.isDataField(r, c))
				left.setDataAt(r, c, op(left.dataAt(r, c), right.dataAt(r, c)));
			else
				left.setNoDataValueAt(r,c);
		}
	}
	return left;
};

inline GridP& operator*=(GridP& left, const GridP& right)
{
	return inPlaceScalarMatrixOp(left, right, std::multiplies<float>());
}

inline GridP& operator+=(GridP& left, const GridP& right)
{
	return inPlaceScalarMatrixOp(left, right, std::plus<float>());
}

template<class OP>
GridP& merge(const GridP& left, const GridP& right, OP op)
{
	assert(left.isCompatible(&right));
	GridP* res = left.clone();
	for (int r = 0, rs = res->rows(); r < rs; r++)
	{
		for (int c = 0, cs = res->cols(); c < cs; c++)
		{
			if(left.isNoDataField(r, c))
			{
				if(right.isDataField(r, c))
					res->setDataAt(r, c, right.dataAt(r, c));
				else
					res->setNoDataValueAt(r, c);
			}
			else
			{
				if(right.isNoDataField(r, c))
					res->setDataAt(r, c, left.dataAt(r, c));
				else
					res->setDataAt(r, c, op(left.dataAt(r, c), right.dataAt(r, c)));
			}
		}
	}
	return *res;
};

template<class OP>
GridP& scalarMatrixOp(const GridP& left, const GridP& right, OP op)
{
	assert(left.isCompatible(&right));
	GridP* res = left.clone();
	for (int r = 0, rs = res->rows(); r < rs; r++)
	{
		for (int c = 0, cs = res->cols(); c < cs; c++)
		{
			if (left.isDataField(r, c) && right.isDataField(r, c))
				res->setDataAt(r, c, op(left.dataAt(r, c), right.dataAt(r, c)));
			else
				res->setNoDataValueAt(r, c);
		}
	}
	return *res;
};

inline GridP& operator*(const GridP& left, const GridP& right)
{
	return scalarMatrixOp(left, right, std::multiplies<float>());
}

inline GridP& operator-(const GridP& left, const GridP& right)
{
	return scalarMatrixOp(left, right, std::minus<float>());
}

inline GridP& operator+(const GridP& left, const GridP& right)
{
	return scalarMatrixOp(left, right, std::plus<float>());
}

inline GridP& operator/(const GridP& left, const GridP& right)
{
	return scalarMatrixOp(left, right, std::divides<float>());
}

inline vector<double> allDataAsLinearVector(const GridP* g)
{
	return g->allDataAsLinearVector();
}

std::vector<double> allDataAsLinearVector(const grid* g);

//----------------------------------------------------------------------------

//! just thin wrapper for easier access with subgrids, can be used as valueobject
class SubGridWrapper
{
public:
	SubGridWrapper(const GridP* grid, SubData subData)
		: _grid(grid), _subData(subData) {}

	float dataAt(int row, int col) const
	{
		return _grid->dataAt(_subData.row + row, _subData.col + col);
	}

	bool isNoDataField(int row, int col) const
	{
		return dataAt(row, col) == noDataValue();
	}

	bool isDataField(int row, int col) const
	{
		return !isNoDataField(row, col);
	}

	int noDataValue() const { return _grid->noDataValue(); }

private:
	const GridP* _grid;
	SubData _subData;
};

//----------------------------------------------------------------------------

//! hold just some information about the grid, without having to load it
struct GridProxy : public Loki::ObjectLevelLockable<GridProxy>
{
	enum State { eNew, eChanged, eNormal };

	GridProxy() : modificationTime(0), state(eNormal) { }
	GridProxy(const std::string& dsn, const std::string& fn,
						const std::string& ptgrid, time_t modTime = 0)
		: datasetName(dsn), fileName(fn), pathToGrid(ptgrid),
			modificationTime(modTime), state(eNew) { }

	GridProxy(const std::string& dsn, const std::string& fn,
						const std::string& pthdf, const std::string& hfn,
						time_t modTime, State s = eNormal)
		: datasetName(dsn), fileName(fn), pathToHdf(pthdf),
			hdfFileName(hfn), modificationTime(modTime), state(s) {}

	~GridProxy(){}

	void updateModificationTime(time_t modTime)
	{
		modificationTime = modTime;
		state = eChanged;
	}

	GridP* gridPtr();

	GridPPtr gridPPtr();

	//! resets gridproxy which in the end (without references to it) deletes possibly loaded grid
	void reset(){ g.reset(); }

	GridP* copyOfFullGrid() { return gridPtr()->clone(); }

	void resetToLoadFromAscii(const std::string& pathToGrid);

	std::string toString() const;

	std::string datasetName;
	std::string fileName;
	std::string pathToGrid;
	std::string pathToHdf;
	std::string hdfFileName;
	time_t modificationTime;
	State state;
protected:
	GridPPtr g;
};

typedef boost::shared_ptr<GridProxy> GridProxyPtr;

//----------------------------------------------------------------------------

std::pair<GridMetaData, time_t>
readGridMetadataFromHdf(const char* hdfFileName, const char* datasetName);
}

#endif
