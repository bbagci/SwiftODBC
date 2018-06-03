int odbc_Initialize();
int odbc_ConnectionCreate(const char connStr[]);
int odbc_ExecuteStmt(const char sc[]);
int odbc_FetchNext();
const char *odbc_ColumnGetName(unsigned short pos);
const char *odbc_GetString(int pos);
int odbc_GetColumnCount();
int odbc_GetAffectedRows();
void odbc_Cleanup();