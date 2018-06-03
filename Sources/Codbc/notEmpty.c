/*
    #define SQL_SUCCESS                0
    #define SQL_SUCCESS_WITH_INFO      1
*/
#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>

#define RESULT_LEN 256
#define DEBUG_MODE 0

SQLHENV henv = SQL_NULL_HENV;    // Environment
SQLHDBC hdbc = SQL_NULL_HDBC;    // Connection handle
SQLHSTMT hstmt = SQL_NULL_HSTMT; // Statement handle
SQLRETURN retcode = 0;
SQLCHAR strResult[RESULT_LEN];
SQLCHAR outstr[1024];
SQLSMALLINT outstrlen;
SQLLEN rowCount = 0;

SQLCHAR **arr = NULL;
SQLCHAR *columnName = NULL;

int i = 0, j = 0;

const char *odbc_ColumnGetName(unsigned short pos)
{
    SQLSMALLINT nullablePtr = 0;
    SQLSMALLINT decimalDigitsPtr = 0;
    SQLULEN columnSizePtr = 0;
    SQLSMALLINT dataTypePtr = 0;
    SQLSMALLINT nameLengthPtr = 0;
    // hint: SQL_MAX_COLUMN_NAME_LEN is 30
    columnName = realloc(columnName, 63 * sizeof(unsigned char));

    SQLDescribeCol(
        hstmt,
        (pos + 1),
        columnName,
        (sizeof(unsigned char) * 63),
        &nameLengthPtr,
        &dataTypePtr,
        &columnSizePtr,
        &decimalDigitsPtr,
        &nullablePtr);

    return (const char *)columnName;
}

int odbc_Initialize()
{
    // step 1
    // Allocate environment handle
    retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (DEBUG_MODE)
        printf("SQLAllocHandle(SQL_HANDLE_ENV): %d\n", retcode);

    // Set the ODBC version environment attribute
    retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER *)SQL_OV_ODBC3, 0);
    if (DEBUG_MODE)
        printf("SQLSetEnvAttr(SQL_ATTR_ODBC_VERSION): %d\n", retcode);

    // Allocate connection handle
    retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (DEBUG_MODE)
        printf("SQLAllocHandle(SQL_HANDLE_DBC): %d\n", retcode);

    // Set login timeout to 5 seconds
    retcode = SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
    if (DEBUG_MODE)
        printf("SQLSetConnectAttr(SQL_LOGIN_TIMEOUT): %d\n", retcode);

    return retcode;
}

int odbc_ConnectionCreate(const char connStr[])
{
    // Connect to data source
    retcode = SQLDriverConnect(hdbc, NULL, (SQLCHAR *)connStr, SQL_NTS, outstr, sizeof(outstr), &outstrlen, SQL_DRIVER_NOPROMPT);
    if (DEBUG_MODE)
        printf("SQLDriverConnect(...): %d\n", retcode);

    return retcode;
}

int odbc_ExecuteStmt(const char sc[])
{
    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (DEBUG_MODE)
        printf("SQLAllocHandle(SQL_HANDLE_STMT): %d\n", retcode);

    // step 3
    retcode = SQLExecDirect(hstmt, (SQLCHAR *)sc, SQL_NTS);
    if (DEBUG_MODE)
        printf("SQLExecDirect(SQLHSTMT): %d\n", retcode);

    SQLSMALLINT columnCountPtr = 0;
    SQLNumResultCols(hstmt, &columnCountPtr);
    const int nCols = (int)columnCountPtr;
    arr = realloc(arr, sizeof(unsigned char *));

    for (i = 0; i < nCols; ++i)
    {
        arr[i] = malloc(RESULT_LEN * sizeof(unsigned char));
        retcode = SQLBindCol(hstmt, (i + 1), SQL_C_CHAR, arr[i], RESULT_LEN, 0);
    }

    return retcode;
}

int odbc_GetColumnCount()
{
    SQLSMALLINT columnCountPtr = 0;
    SQLNumResultCols(hstmt, &columnCountPtr);
    return (int)columnCountPtr;
}

const char *odbc_GetString(int pos)
{
    return (const char *)arr[pos];
}

int odbc_FetchNext()
{
    retcode = SQLFetch(hstmt);
    return retcode;
}

int odbc_GetAffectedRows()
{
    SQLRowCount(hstmt, &rowCount);
    return rowCount;
}

void odbc_Cleanup()
{
    SQLSMALLINT columnCountPtr = 0;
    SQLNumResultCols(hstmt, &columnCountPtr);
    const int nCols = (int)columnCountPtr;

    if (*arr[0] != 0)
    {
        for (i = 0; i < nCols; ++i)
        {
            free(arr[i]);
        }
    }

    if (*arr != 0)
        free(arr);

    //err hier
    if (*columnName != 0)
        free(columnName);

    // Free handles
    // Statement
    if (hstmt != SQL_NULL_HSTMT)
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    // Connection
    if (hdbc != SQL_NULL_HDBC)
    {
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    }
    // Environment
    if (henv != SQL_NULL_HENV)
        SQLFreeHandle(SQL_HANDLE_ENV, henv);

    if (DEBUG_MODE)
        printf("Complete.\n");
}