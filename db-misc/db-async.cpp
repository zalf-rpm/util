#include <list>

#include "db-async.h"

using namespace Db;
using namespace std;
using namespace boost;

DB* Db::connectionFor(string abstractSchema)
{
	return newConnection(abstractSchema);
}

struct QueryRequestData
{
	QueryRequestData() {}
	QueryRequestData(QueryRequestData& other)
		: abstractSchema(other.abstractSchema),
			queryStatement(other.queryStatement),
			resultPromise(std::move(other.resultPromise)) {}
	QueryRequestData(const QueryRequestData& other)
		: abstractSchema(other.abstractSchema),
			queryStatement(other.queryStatement),
			resultPromise(std::move(other.resultPromise)) {}
	string abstractSchema;
	string queryStatement;
	promise<QueryResult> resultPromise;
};

void runQuery(DB* con, QueryRequestData qrd)
{
	con->select(qrd.queryStatement.c_str());
	DBRow row;

	unsigned int cols = con->getNumberOfFields();
	QueryResult qr;

	while(!(row=con->getRow()).empty())
	{
		vector<string> v(cols);
		for(int col = 0; col < cols; col++)
			v[col] = row[col];
		qr.push_back(v);
	}

	qrd.resultPromise.set_value(qr);
}

unique_future<QueryResult> Db::query(string abstractSchema, std::string queryStatement)
{
	condition_variable cond;
	mutex mut;

	struct RunQueries
	{
		condition_variable& cond;
		mutex& mut;
		list<QueryRequestData> jobs;

		RunQueries(condition_variable& cond, mutex& mut)
			: cond(cond), mut(mut) {}

		void operator()()
		{
			unique_lock<mutex> lock(mut);
			for(;;)
			{
				while(jobs.empty())
				{
					cond.wait(lock);
				}

				QueryRequestData qrd = jobs.front();
				DB* con = connectionFor(qrd.abstractSchema);
				runQuery(con, qrd);
				jobs.pop_front();
				delete con;
			}
		}
	};
	static RunQueries runQueries(cond, mut);
	static boost::thread t(runQueries);

	QueryRequestData qrd;
	qrd.queryStatement = queryStatement;
	qrd.abstractSchema = abstractSchema;

	unique_future<QueryResult> f = qrd.resultPromise.get_future();

	{
		lock_guard<mutex> lock(runQueries.mut);
		runQueries.jobs.push_back(qrd);
	}
	runQueries.cond.notify_one();

	return std::move(f);
}


