import Codbc

public class ODBC {

    public init() {
        guard odbc_Initialize() == 0 else {
            print("Initialize error")
            return
        }
    }

    deinit {
        odbc_Cleanup()
    }

    @discardableResult public func connect(driver: String, ip: String, port: String, uid: String, password: String, database: String) -> Bool {
        return (odbc_ConnectionCreate("Driver=\(driver);Server=tcp:\(ip),\(port);Uid=\(uid);Pwd=\(password);database=\(database)") == 0)
    }

    @discardableResult public func query(sql: String) -> Bool {
        return (odbc_ExecuteStmt(sql) == 0)
    }

    public func affectedRows() -> Int32 {
        return odbc_GetAffectedRows()
    }

    public func close() {
        odbc_Cleanup()
    }

    public func storeResults() -> Results {
        return Results()
    }
}


public struct Results: Sequence, IteratorProtocol {

    public var columnCount: Int32

    internal init() {
        self.columnCount = odbc_GetColumnCount()
    }

    public func next() -> [String: String]? {
        var row = [String: String]()
        if(odbc_FetchNext() == 0 ) {
            for index in 0...(self.columnCount-1) {
                row[ String(validatingUTF8: odbc_ColumnGetName(UInt16(index)))! ] = String(validatingUTF8: odbc_GetString(index))!
            }
            return row
        } else {
            return nil
        }
    }

    public func numFields() -> Int32 {
        return self.columnCount
    }
}