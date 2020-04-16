#pragma once
#include <windows.h>
#include <sqlext.h>
#include <iostream>

#define RESULT_NO_ID -1

class DBMANAGER
{
private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt;
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

public:
	DBMANAGER();
	~DBMANAGER();
	void init();
	int id_check(const char* id);
};