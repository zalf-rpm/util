#ifndef AUTO_DELETER_H_
#define AUTO_DELETER_H_

#include "tools/helper.h"

namespace Tools {

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
