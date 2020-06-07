#pragma once
#include <windows.h>
#include <sqlext.h>
#include <iostream>
#include <vector>
#include <string>

#define RESULT_NO_ID -1

/**
*@brief class for querying to DB.
*@author Gurnwoo Kim
*/
class DBMANAGER
{
public:
	DBMANAGER();
	~DBMANAGER();
	void init(); ///< Initializing ODBC.
	int get_uid(const char* id); ///< get clients uid.
	std::vector<std::string> get_friendlist(const char* id); ///< get clients friend list.
	void insert_friend(const char* friendA, const char* friendB); ///< insert friend relationship.

private:
	SQLHENV henv; ///< env handle for odbc.
	SQLHDBC hdbc; ///< connection handle for odbc.
	SQLHSTMT hstmt; ///< state handle for odbc.
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode); ///< display error with retcode.
};