#include "register_types.h"

#include "core/object/class_db.h"
#include "sqlite.h"

void initialize_sqlite_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SERVERS) {
    return;
  }
  ClassDB::register_class<SQLite>();
  ClassDB::register_class<SQLiteQuery>();
}

void uninitialize_sqlite_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SERVERS) {
    return;
  }
}
