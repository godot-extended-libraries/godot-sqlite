#include "sqlite.h"
#include "core/bind/core_bind.h"
#include "core/os/os.h"
#include "editor/project_settings_editor.h"

SQLite::SQLite() {
	db = nullptr;
	memory_read = false;
}
/*
	Open a database file.
	If this is running outside of the editor, databases under res:// are assumed to be packed.
	@param path The database resource path.
	@return status
*/
bool SQLite::open(String path) {
	if (!path.strip_edges().length())
		return false;

	if (!Engine::get_singleton()->is_editor_hint() && path.begins_with("res://")) {
		Ref<_File> dbfile;
		dbfile.instance();
		if (dbfile->open(path, _File::READ) != Error::OK) {
			print_error("Cannot open packed database!");
			return false;
		}
		int64_t size = dbfile->get_len();
		PoolByteArray buffer = dbfile->get_buffer(size);
		return open_buffered(path, buffer, size);
	}

	String real_path = ProjectSettings::get_singleton()->globalize_path(path.strip_edges());

	int result = sqlite3_open(real_path.utf8().get_data(), &db);

	if (result != SQLITE_OK) {
		print_line("Cannot open database!");
		return false;
	}

	return true;
}
/*
  Open the database and initialize memory buffer.
  @param name Name of the database.
  @param buffers The database buffer.
  @param size Size of the database; 
  @return status
*/
bool SQLite::open_buffered(String name, PoolByteArray buffers, int64_t size) {
	if (!name.strip_edges().length()) {
		return false;
	}

	if (!buffers.size() || !size) {
		return false;
	}

	spmembuffer_t *p_mem = (spmembuffer_t *)calloc(1, sizeof(spmembuffer_t));
	p_mem->total = p_mem->used = size;
	p_mem->data = (char *)malloc(size + 1);
	memcpy(p_mem->data, buffers.read().ptr(), size);
	p_mem->data[size] = '\0';

	//
	spmemvfs_env_init();
	int err = spmemvfs_open_db(&p_db, name.utf8().get_data(), p_mem);

	if (err != SQLITE_OK || p_db.mem != p_mem) {
		print_line("Cannot open buffered database!");
		return false;
	}

	memory_read = true;
	return true;
}

void SQLite::close() {
	if (db) {
		// Cannot close database!
		if (sqlite3_close_v2(db) != SQLITE_OK) {
			print_line("Cannot close database!");
		} else {
			db = nullptr;
		}
	}

	if (memory_read) {
		// Close virtual filesystem database
		spmemvfs_close_db(&p_db);
		spmemvfs_env_fini();
		memory_read = false;
	}
}

sqlite3_stmt *SQLite::prepare(const char *query) {
	// Get database pointer
	sqlite3 *dbs = get_handler();

	if (!dbs) {
		print_line("Cannot prepare query! Database is not opened.");
		return nullptr;
	}

	// Prepare the statement
	sqlite3_stmt *stmt;
	int result = sqlite3_prepare_v2(dbs, query, -1, &stmt, nullptr);

	// Cannot prepare query!
	if (result != SQLITE_OK) {
		print_line("SQL Error: " + String(sqlite3_errmsg(dbs)));
		return nullptr;
	}

	return stmt;
}

bool SQLite::query_with_args(String query, PoolStringArray args) {
	sqlite3_stmt *stmt = prepare(query.utf8().get_data());

	// Failed to prepare the query
	if (!stmt) {
		return false;
	}

	int param_count = sqlite3_bind_parameter_count(stmt);
	if (param_count != args.size()) {
		String output = args.join("");
		OS::get_singleton()->print("Query failed, expected %d args, got %s", param_count, output.c_str());
		return false;
	}

	for (int i = 0; i < param_count; i++)
		sqlite3_bind_text(stmt, i + 1, args[i].utf8().get_data(), -1, SQLITE_TRANSIENT);

	// Evaluate the sql query
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return true;
}

bool SQLite::query(String query) {
	return this->query_with_args(query, PoolStringArray());
}

Array SQLite::fetch_rows(String statement, PoolStringArray args, int result_type) {
	Array result;

	// Empty statement
	if (!statement.strip_edges().length()) {
		return result;
	}

	// Cannot prepare query
	sqlite3_stmt *stmt = prepare(statement.strip_edges().utf8().get_data());
	if (!stmt) {
		return result;
	}

	// Check parameter count
	int param_count = sqlite3_bind_parameter_count(stmt);
	if (param_count != args.size()) {
		OS::get_singleton()->print("Fetch failed, expected %d args, got %s", param_count, args.join("").c_str());
		return result;
	}

	// Bind parameters
	for (int i = 0; i < param_count; i++)
		sqlite3_bind_text(stmt, i + 1, args[i].utf8().get_data(), -1, SQLITE_TRANSIENT);

	// Fetch rows
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		// Do a step
		result.append(parse_row(stmt, result_type));
	}

	// Delete prepared statement
	sqlite3_finalize(stmt);

	// Return the result
	return result;
}

Dictionary SQLite::parse_row(sqlite3_stmt *stmt, int result_type) {
	Dictionary result;

	// Get column count
	int col_count = sqlite3_column_count(stmt);

	// Fetch all column
	for (int i = 0; i < col_count; i++) {
		// Key name
		const char *col_name = sqlite3_column_name(stmt, i);
		String key = String(col_name);

		// Value
		int col_type = sqlite3_column_type(stmt, i);
		Variant value;

		// Get column value
		switch (col_type) {
			case SQLITE_INTEGER:
				value = Variant(sqlite3_column_int(stmt, i));
				break;

			case SQLITE_FLOAT:
				value = Variant(sqlite3_column_double(stmt, i));
				break;

			case SQLITE_TEXT:
				value = Variant((char *)sqlite3_column_text(stmt, i));
				break;

			default:
				break;
		}

		// Set dictionary value
		if (result_type == RESULT_NUM)
			result[i] = value;
		else if (result_type == RESULT_ASSOC)
			result[key] = value;
		else {
			result[i] = value;
			result[key] = value;
		}
	}

	return result;
}

Array SQLite::fetch_array(String query) {
	return fetch_rows(query, PoolStringArray(), RESULT_BOTH);
}

Array SQLite::fetch_array_with_args(String query, PoolStringArray args) {
	return fetch_rows(query, args, RESULT_BOTH);
}

Array SQLite::fetch_assoc(String query) {
	return fetch_rows(query, PoolStringArray(), RESULT_ASSOC);
}

Array SQLite::fetch_assoc_with_args(String query, PoolStringArray args) {
	return fetch_rows(query, args, RESULT_ASSOC);
}

SQLite::~SQLite() {
	// Close database
	close();
}

void SQLite::_bind_methods() {
	ClassDB::bind_method(D_METHOD("open"), &SQLite::open);
	ClassDB::bind_method(D_METHOD("open_buffered"), &SQLite::open_buffered);
	ClassDB::bind_method(D_METHOD("query"), &SQLite::query);
	ClassDB::bind_method(D_METHOD("query_with_args"), &SQLite::query_with_args);
	ClassDB::bind_method(D_METHOD("close"), &SQLite::close);
	ClassDB::bind_method(D_METHOD("fetch_array"), &SQLite::fetch_array);
	ClassDB::bind_method(D_METHOD("fetch_array_with_args"), &SQLite::fetch_array_with_args);
	ClassDB::bind_method(D_METHOD("fetch_assoc"), &SQLite::fetch_assoc);
	ClassDB::bind_method(D_METHOD("fetch_assoc_with_args"), &SQLite::fetch_assoc_with_args);
}
