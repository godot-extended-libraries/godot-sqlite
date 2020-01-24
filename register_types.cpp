#include "register_types.h"

#include "sqlite.h"
#include "core/class_db.h"

void register_sqlite_types() {
    ClassDB::register_class<SQLite>();
}

void unregister_sqlite_types() {
   // Nothing to do here in this example.
}