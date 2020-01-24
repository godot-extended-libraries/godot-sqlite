#ifndef GDSQLITE_H
#define GDSQLITE_H

#include "core/engine.h"
#include "core/reference.h"

// SQLite3
#include "thirdparty/sqlite/sqlite3.h"
#include "thirdparty/sqlite/spmemvfs.h"

class SQLite : public Reference {
	GDCLASS(SQLite, Reference);

private:
	// sqlite handler
	sqlite3 *db;

	// vfs
	spmemvfs_db_t p_db;
	bool memory_read;

public:
	enum {
		RESULT_BOTH = 0,
		RESULT_NUM,
		RESULT_ASSOC
	};

	static void _bind_methods();

	// constructor
	SQLite();
	~SQLite();
	void _init() {}

	// methods
	bool open(String path);
	bool open_buffered(String name, PoolByteArray buffers, int64_t size);
	void close();

	bool query(String statement);
	bool query_with_args(String statement, PoolStringArray args);
	Array fetch_array(String statement);
	Array fetch_array_with_args(String statement, PoolStringArray args);
	Array fetch_assoc(String statement);
	Array fetch_assoc_with_args(String statement, PoolStringArray args);

private:
	sqlite3_stmt* prepare(const char* statement);
	Array fetch_rows(String query, PoolStringArray args, int result_type = RESULT_BOTH);
	Dictionary parse_row(sqlite3_stmt *stmt, int result_type);
	sqlite3* get_handler() { return (memory_read ? p_db.handle : db); }
};
#endif
