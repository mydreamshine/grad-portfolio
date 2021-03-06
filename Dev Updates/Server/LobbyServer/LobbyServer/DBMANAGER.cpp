#include "DBMANAGER.h"
#include <string>

constexpr int string_len = 32;

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

int DBMANAGER::get_uid(const wchar_t* id)
{
	SQLRETURN retcode;
	SQLWCHAR query[] = { L"{call get_uid(?)}" };
	SQLLEN idlen = SQL_NTS;

	retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, string_len, 0, (void*)id, sizeof(wchar_t) * string_len, &idlen);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);

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

int DBMANAGER::get_uid(const wchar_t* id, const wchar_t* pw)
{
	SQLRETURN retcode;
	SQLWCHAR query[] = { L"{call get_uid_with_pw(?, ?)}" };
	SQLLEN idlen = SQL_NTS;
	
	retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, string_len, 0, (void*)id, sizeof(wchar_t) * string_len, &idlen);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);

	retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, string_len, 0, (void*)pw, sizeof(wchar_t) * string_len, &idlen);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);

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

int DBMANAGER::get_rank(int uid)
{
	SQLRETURN retcode;
	SQLWCHAR query[] = { L"{call get_rank(?)}" };
	SQLLEN idlen = SQL_NTS;

	retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &uid, 0, NULL);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);

	retcode = SQLExecDirect(hstmt, query, SQL_NTS);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);

	SQLINTEGER rank;
	SQLLEN cuid;
	SQLBindCol(hstmt, 1, SQL_INTEGER, &rank, sizeof(rank), &cuid);
	retcode = SQLFetch(hstmt);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	SQLCloseCursor(hstmt);

	if (retcode == SQL_NO_DATA)
		return RESULT_NO_ID;

	return rank;
}

void DBMANAGER::update_rank(int uid, int rank)
{
	SQLRETURN retcode;
	SQLWCHAR query[] = { L"{call update_rank(?, ?)}" };
	SQLLEN idlen = SQL_NTS;
	
	retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &uid, 0, NULL);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);

	retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &rank, 0, NULL);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);

	retcode = SQLExecDirect(hstmt, query, SQL_NTS);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	SQLCloseCursor(hstmt);
}


std::vector<std::string> DBMANAGER::get_friendlist(const wchar_t* id)
{
	std::vector<std::string> friendlist;
	SQLRETURN retcode;
	SQLWCHAR query[] = { L"{call get_friendlist(?)}" };
	SQLLEN idlen = SQL_NTS;
	retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, string_len, 0, (void*)id, sizeof(wchar_t) * string_len, &idlen);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	retcode = SQLExecDirect(hstmt, query, SQL_NTS);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);

	SQLCHAR friends[string_len];
	SQLBindCol(hstmt, 1, SQL_CHAR, friends, string_len, NULL);
	while (SQLFetch(hstmt) != SQL_NO_DATA) {
		friendlist.emplace_back((const char*)friends);
	}
	SQLCloseCursor(hstmt);

	return friendlist;
}


void DBMANAGER::insert_friend(const char* friendA, const char* friendB)
{
	//새아이디 추가
	SQLRETURN retcode;
	SQLWCHAR query[100];
	if(strcmp(friendA, friendB) < 0)
		wsprintf(query, L"INSERT INTO friend_table VALUES ('%S', '%S')", friendA, friendB);
	else
		wsprintf(query, L"INSERT INTO friend_table VALUES ('%S', '%S')", friendB, friendA);
	retcode = SQLExecDirect(hstmt, query, SQL_NTS);
	HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	SQLCloseCursor(hstmt);
}