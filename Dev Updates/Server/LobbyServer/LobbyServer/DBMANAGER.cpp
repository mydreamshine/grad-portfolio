#include "DBMANAGER.h"

DBMANAGER::DBMANAGER() : 
	henv(),
	hdbc(),
	hstmt()
{
}

DBMANAGER::~DBMANAGER()
{
	SQLCancel(hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

void DBMANAGER::init()
{
	SQLRETURN retcode;
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
	retcode = SQLConnect(hdbc, (SQLWCHAR*)L"BattleArena", SQL_NTS, (SQLWCHAR*)NULL, SQL_NTS, NULL, SQL_NTS);
	SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
}

void DBMANAGER::HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	/************************************************************************
	/* HandleDiagnosticRecord : display error/warning information
	/*
	/* Parameters:
	/* hHandle ODBC handle
	/* hType Type of handle (SQL_HANDLE_STMT, SQL_HANDLE_ENV, SQL_HANDLE_DBC)
	/* RetCode Return code of failing command
	/************************************************************************/
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

int DBMANAGER::id_check(const char* id)
{
	SQLRETURN retcode;
	SQLWCHAR query[100];
	wsprintf(query, L"SELECT uid FROM user_table WHERE ID = '%S'", id);
	retcode = SQLExecDirect(hstmt, query, SQL_NTS);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);

	SQLINTEGER uid;
	SQLLEN cuid;
	SQLBindCol(hstmt, 1, SQL_INTEGER, &uid, sizeof(uid), &cuid);
	retcode = SQLFetch(hstmt);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	SQLCloseCursor(hstmt);

	if (retcode == SQL_NO_DATA)
		return RESULT_NO_ID;

	return uid;
}