#pragma once
#include <windows.h>
#include <sqlext.h>
#include <iostream>
#include <vector>
#include <string>

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
	int get_uid(const char* id);
	std::vector<std::string> get_friendlist(const char* id);
	void insert_friend(const char* friendA, const char* friendB);
};