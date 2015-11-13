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

#ifndef AUTO_DELETER_H_
#define AUTO_DELETER_H_

#include "tools/helper.h"

namespace Tools
{
	/*!
	 * functions deletes all elements in collection after possibly
	 * extracting elements from the collection entries (e.g. from the pairs
	 * in a map)
	 * @param coll
	 */
	template<class T, template<class> class E>
	void deleteAll(T& coll){
		E<typename T::value_type> extract;
		for(typename T::iterator i = coll.begin(); i != coll.end(); i++)
			delete extract(*i);
	}

	/*!
	 * convenience function to delete all elements from a collection if
	 * the collection's items shall be deleted themself
	 * @param coll
	 */
	template<class T>
	void deleteAll(T& coll){ deleteAll<T, Id>(coll); }

	/*!
	 * automatically delete elements in collection if AutoDeleter gets destroyed
	 * possibly with a given extraction functor which can extract to be deleted
	 * pointers from the collection's elements
	 */
	template<class T, template<class> class E = Id>
	struct AutoDeleter {
		T collection;
		AutoDeleter()  {}
		~AutoDeleter(){ deleteAll<T, E>(collection); }
	};

}

#endif
