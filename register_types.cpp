#include "register_types.h"

#include "core/class_db.h"
#include "sqlite.h"

void register_sqlite_types() {
	ClassDB::register_class<SQLite>();
	ClassDB::register_class<SQLiteQuery>();
}

void unregister_sqlite_types() {
}