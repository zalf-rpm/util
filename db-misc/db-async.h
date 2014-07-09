#ifndef DBASYNC_H_
#define DBASYNC_H_

#include <vector>

#include "boost/tuple/tuple.hpp"
#include "boost/thread.hpp"

#include "db.h"

namespace Db
{
	//can block until a connection is free
	//uses a certain amount of connections for the given connection data
	DB* connectionFor(std::string abstractSchema);

	//vector of result rows
	typedef std::vector<std::vector<std::string> > QueryResult;
	boost::unique_future<QueryResult> query(std::string abstractSchema, std::string queryStatement);

	//returns a vector of typed result fows
	template<typename Tuple>
	std::vector<Tuple> query(std::string queryStatement)
	{

	}



}

#endif
