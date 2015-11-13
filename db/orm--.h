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

#ifndef ORM_MINUS_MINUS_H_
#define ORM_MINUS_MINUS_H_

#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <list>
#include <memory>
#include <mutex>

#include "tools/auto-deleter.h"
#include "db/abstract-db-connections.h"
#include "tools/algorithms.h"
#include "tools/helper.h"
#include "orm--typedefs.h"

namespace Db
{
	//! interface for objects which are displayable
  struct Printable
  {
		//! virtual destructor for polymorphic use of subclasses
		virtual ~Printable(){}
		/*!
		 * output the objects contents as a string
		 * indent indentation, by default ""
		 * detailed, optionally offer two modes, one outputing more details
		 * this might be used to skip substructures
		 * @return string output of object
		 */
		virtual std::string toString(const std::string& /*indent*/ = std::string(),
																 bool /*detailed*/ = false) const
		{
			return std::string();
		}
	};

	//----------------------------------------------------------------------------

	//! interface for an identifiable object with id and name
  struct Identifiable : public Printable
  {
		//! default constructor
		Identifiable(int id = -1, const std::string& name = std::string())
      : id(id), name(name) {}

		//! the objects id
		int id;
		//! the objects dane
		std::string name;

		//! default comparision based on id
    bool operator<(const Identifiable& other) const { return id < other.id; }
		//! default equality based on id
    bool operator==(const Identifiable& other) const { return id == other.id; }
		//! outputable
		virtual std::string toString(const std::string& indent = std::string(),
		                             bool detailed = false) const;
	};

	//! comparision functor of pointers to objects of this type in for instance maps
  struct PtrComp
  {
		//! comparision operator
    bool operator()(const Identifiable* l, const Identifiable * r) const
    {
			return l->id < r->id;
		}
	};

	//----------------------------------------------------------------------------

	//! interface if an object is describable, thus has an description
	struct Describable : public Identifiable
  {
		//! default constructor
		Describable(int id = -1, const std::string& name = std::string(),
		            const std::string& desc = std::string())
                  : Identifiable(id, name), description(desc) {}

		//! the description
		std::string description;

		//! output will include the description
		virtual std::string toString(const std::string& indent = std::string(),
		                             bool detailed = false) const;
	};

	//----------------------------------------------------------------------------

	//! convenience template function to create a map of a collection from id to elements
	template<class T>
  T* t4id(int id)
  {
    static std::mutex lockable;
		static bool mapInitialized = false;
		typedef std::map<int, T*> Id2T;
		static Id2T m;
    if(!mapInitialized)
    {
      std::lock_guard<std::mutex> lock(lockable);
      if(!mapInitialized)
      {
				typedef typename T::Collection::const_iterator CI;
				for(CI ci = T::all().begin(); ci != T::all().end(); ci++)
					m.insert(std::make_pair((*ci)->id, *ci));
				mapInitialized = true;
			}
		}
		typename Id2T::const_iterator ci = m.find(id);
		return ci == m.end() ? NULL : ci->second;
	}

	//----------------------------------------------------------------------------

	//! create a string representation of a collection of toString():able pointers
	template<class T>
	std::string ptrCollectionToString(const T& coll,
	                                  const std::string& intro = std::string(),
	                                  bool detailed = false,
                                    const std::string& indent = std::string())
  {
		std::ostringstream s;
    if(!intro.empty())
      s << intro << std::endl;
    for(auto tptr : coll)
      s << indent << tptr->toString(indent+"\t", detailed) << std::endl;
    //		for(typename T::const_iterator ci = coll.begin(); ci != coll.end(); ci++)
    //			s << indent << (*ci)->toString(indent+"\t", detailed) << std::endl;
		return s.str();
	}

	//----------------------------------------------------------------------------

//	//! functor to call toString on class types (general template)
//	template<class T, bool isStdFundamental>
//  struct ToString
//  {
//		//! the call operator
//		std::string operator()(const T& t, const std::string& indent = std::string(),
//                           bool detailed = false){
//			return t.toString(indent, detailed);
//		}
//	};

	//----------------------------------------------------------------------------

//	//! functor to call toString on a pair (full template specialization)
//	template<typename F, typename S>
//  struct ToString<std::pair<F, S>, false>
//  {
//		//! call operator
//		std::string operator()(std::pair<F, S> t,
//                           const std::string& /*indent*/ = std::string(),
//                           bool /*detailed*/ = false)
//    {
//			const bool firstIsStdF = Loki::TypeTraits<F>::isStdFundamental;
//			const bool secondIsStdF = Loki::TypeTraits<S>::isStdFundamental;
//			std::ostringstream s;
//			s << "(" << ToString<F, firstIsStdF>()(t.first) << ", "
//          << ToString<S, secondIsStdF>()(t.second) << ")";
//			return s.str();
//		}
//	};

	//----------------------------------------------------------------------------

	//! functor to call toString on Pointer types (full template specialization)
//	template<class T>
//  struct ToString<T*, false>
//  {
//		//! call operator
//		std::string operator()(T* t, const std::string& indent = std::string(),
//                           bool detailed = false)
//    {
//			return t->toString(indent, detailed);
//		}
//	};

	//----------------------------------------------------------------------------

	//! functor to output to string primitive types (partial specialization)
//	template<class T>
//	struct ToString<T, true> {
//		//! call operator
//		std::string operator()(T t, const std::string& = std::string(),
//                           bool = false){
//			std::ostringstream s; s << t; return s.str();
//		}
//	};

  template<typename T>
  std::string toString(T t, const std::string& indent = std::string(), bool detailed = false)
  {
    return t.toString(indent, detailed);
  }

  inline std::string toString(int i, const std::string& = std::string(), bool = false)
  {
    std::ostringstream s; s << i; return s.str();
  }

  inline std::string toString(double d, const std::string& = std::string(), bool = false)
  {
    std::ostringstream s; s << d; return s.str();
  }

  inline std::string toString(float f, const std::string& = std::string(), bool = false)
  {
    std::ostringstream s; s << f; return s.str();
  }

  inline std::string toString(bool b, const std::string& = std::string(), bool = false)
  {
    std::ostringstream s; s << b; return s.str();
  }

  template<typename T, bool isStdFundamental = true>
  std::string toString(T t, const std::string& = std::string(), bool = false)
  {
    std::ostringstream s; s << t; return s.str();
  }

  template<typename T>
  std::string toString(T* t, const std::string& indent = std::string(), bool detailed = false)
  {
    return t->toString(indent, detailed);
  }

  template<typename F, typename S>
  std::string toString(std::pair<F, S> t,
                       const std::string& /*indent*/ = std::string(),
                       bool /*detailed*/ = false)
  {
    std::ostringstream s;
    s << "(" << toString(t.first) << ", " << toString(t.second) << ")";
    return s.str();
  }

	//----------------------------------------------------------------------------

	/*!
	 * create a string representation of a toString():able pointer map
	 * (general template)
	 */
//	template<class Map, int recDepth>
//  struct PtrMapToString
//  {
//		//! call operator
//		std::string operator()(const Map& map,
//                           const std::string& intro = std::string(), bool detailed = false,
//                           const std::string& indent = std::string()) const
//    {
//			std::ostringstream s;
//      if(!intro.empty())
//        s << intro << std::endl;
//      else
//        s << std::endl;
//			typedef typename Map::const_iterator CI;
//			typedef typename Map::key_type KT;
//			typedef typename Map::mapped_type MT;
//			const bool isStdF = Loki::TypeTraits<KT>::isStdFundamental;
//      for(CI ci = map.begin(); ci != map.end(); ci++)
//      {
//				s << indent << ToString<KT, isStdF>()(ci->first, indent+"\t")
//            << " ---> "
//            << PtrMapToString<MT, recDepth-1>()
//            (ci->second, "", detailed, indent+"\t") << std::endl;
//			}
//			return s.str();
//		}
//	};

  template<class Map, int recDepth>
  struct PtrMapToString
  {
    //! call operator
    std::string operator()(const Map& map,
                           const std::string& intro = std::string(), bool detailed = false,
                           const std::string& indent = std::string()) const
    {
      std::ostringstream s;
      if(!intro.empty())
        s << intro << std::endl;
      else
        s << std::endl;
      typedef typename Map::const_iterator CI;
      typedef typename Map::key_type KT;
      typedef typename Map::mapped_type MT;
//			const bool isStdF = Loki::TypeTraits<KT>::isStdFundamental;
      for(CI ci = map.begin(); ci != map.end(); ci++)
      {
        s << indent << toString(ci->first, indent+"\t")
            << " ---> "
            << PtrMapToString<MT, recDepth-1>()
            (ci->second, "", detailed, indent+"\t") << std::endl;
      }
      return s.str();
    }
  };

	//----------------------------------------------------------------------------

	//! specialization for base case (recursion depth = 0)
//	template<class T>
//  struct PtrMapToString<T, 0>
//  {
//		//! call operator
//		std::string operator()(T t, const std::string&, bool,
//                           const std::string& indent) const
//    {
//			const bool isStdF = Loki::TypeTraits<T>::isStdFundamental;
//			std::ostringstream s;
//			s << ToString<T, isStdF>()(t, indent+"\t");
//			return s.str();
//		}
//	};

  template<class T>
  struct PtrMapToString<T, 0>
  {
    //! call operator
    std::string operator()(T t, const std::string&, bool,
                           const std::string& indent) const
    {
//      const bool isStdF = Loki::TypeTraits<T>::isStdFundamental;
      std::ostringstream s;
      s << toString(t, indent+"\t");
      return s.str();
    }
  };

	//----------------------------------------------------------------------------

	//! noop functor
  struct NoCode
  {
		//! noop call operator
		template<class T> void operator()(T&){}
	};

	//----------------------------------------------------------------------------

	//! functor to sort a Identifiable list by its name
  struct SortListByName
  {
		//! call operator
		template<class T>
    void operator()(T& list)
    {
			list.sort([](Identifiable* left, const Identifiable* right){
				return left->name < right->name; });
		}
	};

	//----------------------------------------------------------------------------

//	//! functor to sort the lists elements by its orderNo (if there)
//  struct SortListByOrderNo
//  {
//		//! call operator
//		template<class T>
//    void operator()(T& list)
//    {
//			typedef typename Loki::TypeTraits<typename T::value_type>::PointeeType VT;
//      list.sort([](VT vt1, VT vt2){ return vt1->orderNo < vt2->orderNo; });
//		}
//	};

	//----------------------------------------------------------------------------

	/*!
	 * functor to load all elements in a table with a certain structure
	 * (template can be customized with code being called after loading
	 * and upon every element)
	 */
	template<class SetFields, class PostCode = NoCode, class ElemCode = NoCode>
  struct LoadAllOfT
  {
		LoadAllOfT(const std::string& abstractConnection,
							 const std::string& characterSet = "utf8")
			: abstractConnection(abstractConnection),
				characterSet(characterSet) {}

		std::string abstractConnection;
		std::string characterSet;

		//! short cut to the element type
		typedef typename SetFields::ElemType T;
		//! call operator
    typename T::Collection& operator()(const std::string& query)
    {
      static std::mutex lockable;
			static bool initialized = false;
			static Tools::AutoDeleter<typename T::Collection> ad;
			static typename T::Collection& ts = ad.collection;
      if(!initialized)
      {
        std::lock_guard<std::mutex> lock(lockable);
        if(!initialized)
        {
					Db::DBPtr con(Db::newConnection(abstractConnection));
					con->setCharacterSet(characterSet);
					Db::DBRow row;

					con->select(query);
					while(!(row=con->getRow()).empty())
          {
						T* t = new T;
						for(int i = 0; i  < SetFields::noOfCols; i++)
							SetFields()(i, t, row[i]);

						ElemCode()(t);

						ts.push_back(t);
					}

					PostCode()(ts);

					initialized = true;
				}
			}

			return ts;
		}
	};

	//----------------------------------------------------------------------------

	//! interface template
	template<class T>
  struct SetFields
  {
		//! type of template parameter accessible as ElemType
		typedef T ElemType;
	};

	//! set the id and name fields
	template<class T>
  struct SetNameFields : public SetFields<T>
  {
		//! 2 columns
		enum { noOfCols = 2 };
		//! call operator
		void operator()(int col, T* t, const std::string& value)
    {
      switch(col)
      {
			case 0: t->id = Tools::satoi(value); break;
			case 1: t->name = value; break;
			}
		}
	};

	//! set additionally the description fields
	template<class T>
  struct SetDescFields : public SetFields<T>
  {
		//! 3 columns
		enum { noOfCols = 3 };
		//! call operator
		void operator()(int col, T* t, const std::string& value)
    {
      switch(col)
      {
      case 0:
      case 1: SetNameFields<T>()(col, t, value); break;
				case 2: t->description = value; break;
      }
		}
	};

  template<class T, class Collection>
  T* findTWithId(const Collection& c, int id)
  {
    typename Collection::const_iterator ci =
				std::find_if(c.begin(), c.end(), [id](T* t){ return t->id == id; });
    return ci != c.end() ? *ci : NULL;
  }

}


#endif
