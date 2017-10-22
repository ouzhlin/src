#include "core/sql/JOSqlQuary.h"


NS_JOFW_BEGIN

namespace sqlitequary
{
	sqlite3 * sdb = 0;
	bool openSQLite(const char* filename)
	{
		int result;
		char * errmsg = NULL;
		result = sqlite3_open(filename, &(sdb));
		if( result != SQLITE_OK )
		{
			sdb = 0;
			return false;
		}
		return true;
	}
	bool closeSQLite()
	{
		sqlite3_close( sdb );
		return true;
	}
}
//template <typename T>
//bool SQLiteBaseQuary<T>::execute(const std::string& sql)
//{
//	char * errmsg = NULL
//	int result = sqlite3_exec( sqlitequary::sdb, sql.c_str(), 0, 0, &errmsg )
//	if(result != SQLITE_OK )
//	{
//		return false;
//	}
//	return true;
//}
//template <typename T>
//bool SQLiteBaseQuary<T>::executeInsert(T* t, const std::string& sql)
//{
//	sqlite3_stmt * stat;
//	sqlite3_prepare( sqlitequary::sdb, sql.c_str(), -1, &stat, 0 );
//	this->insertToRow(stat, t);
//	sqlite3_step( stat );
//	sqlite3_finalize( stat );
//	return true;
//}
//template <typename T>
//std::list<T*> SQLiteBaseQuary<T>::executeSelect(const std::string& sql)
//{
//	std::list<T*> out;
//	sqlite3_stmt * stat;
//	sqlite3_prepare( sqlitequary::sdb, sql.c_str(), -1, &stat, 0 );
//	int result = sqlite3_step( stat );
//	while (result != SQLITE_ROW)
//	{
//		T* t = this->getFromRow(stat);
//		out.push_back(t);
//		result = sqlite3_step( stat );
//	}
//	sqlite3_finalize(stat);
//	return out;
//}

NS_JOFW_END