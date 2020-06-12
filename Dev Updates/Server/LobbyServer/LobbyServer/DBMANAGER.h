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

	/**
	@brief Initializing ODBC.
	@details set ODBC envionment, connect to ODBC, alloc state handle.
	*/
	void init();

	/**
	@brief get clients uid.
	@param id clients id.
	@return clients uid. if there's no id on DB, return -1.
	*/
	int get_uid(const char* id);

	/**
	@brief get clients friend list.
	@param id clients id.
	@return clients friend list.
	*/
	std::vector<std::string> get_friendlist(const char* id);

	/**
	@brief insert friend relationship.
	@param friendA client As id.
	@param friendB client Bs id.
	*/
	void insert_friend(const char* friendA, const char* friendB);

private:
	SQLHENV henv; ///< env handle for odbc.
	SQLHDBC hdbc; ///< connection handle for odbc.
	SQLHSTMT hstmt; ///< state handle for odbc.

	/**
	@brief display error with retcode.
	@param hHandle ODBC handle
	@param hHandle ODBC handle
	@param hType Type of handle (SQL_HANDLE_STMT, SQL_HANDLE_ENV, SQL_HANDLE_DBC)
	@param RetCode Return code of failing command
	*/
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode); 
};