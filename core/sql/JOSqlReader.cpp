#include <memory>
extern "C"
{
#include "core/sql/sqlite3.h"
};
#include "core/sql/JOSqlReader.h"


JOSqlReader::JOSqlReader( struct sqlite3_stmt* stmt ):m_pStmt(stmt)
{
}

JOSqlReader::~JOSqlReader()
{
	//sqlite3_finalize(m_pStmt);
}

int JOSqlReader::readInt( int col )
{
	return sqlite3_column_int( m_pStmt, col );
}

int JOSqlReader::readInt64( int col )
{
	return sqlite3_column_int64( m_pStmt, col);
}

double JOSqlReader::readDouble( int col )
{
	return sqlite3_column_double( m_pStmt, col);
}

const char* JOSqlReader::readString( int col )
{
	const char* s = (const char*)sqlite3_column_text( m_pStmt, col );
	return s;
// 	int len = strlen(s);
// 	char* to = new char[ len+ 1];
// 	strcpy(to, s);
// 	to[len] = 0;
// 	return to;
}

JOSqlBlob JOSqlReader::readBlob( int col )
{
	JOSqlBlob blob = {0};
	memset(&blob, 0, sizeof(blob));
	//SFSqlBlob* pBlob = new SFSqlBlob;
	//memset(pBlob, 0, sizeof(SFSqlBlob));
	int bufSize = sqlite3_column_bytes( m_pStmt, col);
	if (bufSize == 0)
	{
		return blob;
	}
	blob.blobBuf = sqlite3_column_blob(m_pStmt, col);
	blob.blobSize = bufSize;
	return blob;
	//SFSqlBlob* pBlob = &blob; 
	//pBlob->blobSize = bufSize;
	//pBlob->blobBuf = new char[bufSize];
	//::memcpy(pBlob->blobBuf, sqlite3_column_blob(m_pStmt, col), bufSize);
	//return pBlob;
}

int JOSqlReader::getDataCount()
{
	return sqlite3_data_count(m_pStmt);
}
