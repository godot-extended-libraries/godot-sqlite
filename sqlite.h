#ifndef GDSQLITE_H
#define GDSQLITE_H

#include "core/config/engine.h"
#include "core/object/ref_counted.h"
#include "core/templates/local_vector.h"

// SQLite3
#include "thirdparty/sqlite/spmemvfs.h"
#include "thirdparty/sqlite/sqlite3.h"

class SQLite : public RefCounted {
	GDCLASS(SQLite, RefCounted);

	friend class SQLiteQuery;

private:
	// sqlite handler
	sqlite3 *db;

	// vfs
	spmemvfs_db_t p_db;
	bool memory_read;

	LocalVector<WeakRef *> queries;

	sqlite3_stmt *prepare(const char *statement);
	Array fetch_rows(String query, Array args, int result_type = RESULT_BOTH);
	sqlite3 *get_handler() const { return memory_read ? p_db.handle : db; }
	Dictionary parse_row(sqlite3_stmt *stmt, int result_type);

public:
	static bool bind_args(sqlite3_stmt *stmt, Array args);

protected:
	static void _bind_methods();

public:
	enum {
		RESULT_BOTH = 0,
		RESULT_NUM,
		RESULT_ASSOC
	};

	SQLite();
	~SQLite();

	// methods
	bool open(String path);
	bool open_in_memory();
	bool open_buffered(String name, PackedByteArray buffers, int64_t size);
	void close();

	/// Compiles the query into bytecode and returns an handle to it for a faster
	/// execution.
	/// Note: you can create the query at any time, but you can execute it only
	/// when the DB is open.
	Ref<SQLiteQuery> create_query(String p_query);

	bool query(String statement);
	bool query_with_args(String statement, Array args);
	Array fetch_array(String statement);
	Array fetch_array_with_args(String statement, Array args);
	Array fetch_assoc(String statement);
	Array fetch_assoc_with_args(String statement, Array args);

	String get_last_error_message() const;
};
#endif
