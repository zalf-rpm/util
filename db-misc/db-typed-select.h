#ifndef DBTYPEDSELECT_H_
#define DBTYPEDSELECT_H_

#include <vector>

#include "db/db.h"
#include "loki/HierarchyGenerators.h"
#include "loki/TypeManip.h"

namespace Db {
	
	//! convert a c-string to an int (via the given type via cstdlib functions)
	int convert(char* s, Loki::Type2Type<int>);
	
	//! convert a c-string to an double (via the given type via cstdlib functions)
	double convert(char* s, Loki::Type2Type<double>);
		
	//! iterate over an typelist and traverse db rows on index of type
	template<class TypeList, int column> struct Iterate;

	template<class TypeList>
	struct Iterate<TypeList, 0> {
		typedef typename TypeList::Head Head;
		void operator()(DB&, MYSQL_ROW row, Loki::Tuple<TypeList>& t){
			Loki::Field<0>(t) = convert(row[0], Loki::Type2Type<Head>());
		}	
	};

	template<class TypeList, int column>
	struct Iterate {
		typedef typename Loki::TL::TypeAt<TypeList, column>::Result Type;
		void operator()(DB& con, MYSQL_ROW row, Loki::Tuple<TypeList>& t){
			Loki::Field<column>(t) = convert(row[column], Loki::Type2Type<Type>());
			Iterate<TypeList, column-1>()(con, row, t);
		}	
	};

	template<class TypeList>
	std::vector<Loki::Tuple<TypeList> > 
	typedSelect(DB& con, const char* query){
		const int columns = Loki::TL::Length<TypeList>::value;
		typedef Loki::Tuple<TypeList> Row;
		std::vector<Row> r(columns);
	    
	  con.select(query);
	    
		MYSQL_ROW row;
		while((row = con.getRow())){
			Row t;
			Iterate<TypeList, columns-1>()(con, row, t);
			r.push_back(t);
		}
				  
	  return r;
	}
	
	/*
	template<class TypeList>
	class TypedSelector {
		typedef Loki::Tuple<TypeList> Row;
		const int columns = Loki::TL::Length<TypeList>::value;
		
	public:
		TypedSelector(DB& con) 
		: _connection(con), _noElement(true), _iterator(NULL)	{}
		
		~TypedSelector(){
			delete _iterator;
		}
		
		void select(const char* query){
			_connection.select(query);
		}
		
		Iterator* begin(){
			return _iterator = new Iterator(_connection, );
		}
		
		
		
		template<class TL>
		class Iterator {
		public:
			Iterator(DB& con) : _connection(con), _noElement(true) {
				MYSQL_ROW row;
				if(row = con.getRow()){
					_noElement = false;
					Iterate<TypeList, columns-1>()(_connection, row, _firstElement);
					_nextElement = _firstElement;
				}
				Row* begin(){
				return _noElement ? NULL : & _firstElement;
			}
			Row* end(){ return NULL; }
			
		private:
			DB& _connection;
			bool _noElement;
			Row _firstElement;
			Row _nextElement;
		};
		
	private:
		Iterator* _iterator;
		
		std::vector<Loki::Tuple<TypeList> > 
		typedSelect(DB& con, const char* query){
			const int columns = Loki::TL::Length<TypeList>::value;
			typedef Loki::Tuple<TypeList> Row;
			std::vector<Row> r(columns);
			
			con.select(query);
		    
			MYSQL_ROW row;
			while((row = con.getRow())){
				Row t;
				Iterate<TypeList, columns-1>()(con, row, t);
				r.push_back(t);
			}
					  
			return r;
		}
	};
	//*/
	
}

#endif 
