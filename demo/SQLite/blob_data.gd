extends Node

@onready var db = SQLite.new()

var db_path = "user://bytes_db.sqlite"

var create_table_query = """
CREATE TABLE IF NOT EXISTS byte_data (
	id INTEGER PRIMARY KEY,
	dict BLOB NOT NULL
);
"""

var insert_data_query = """
INSERT INTO byte_data VALUES (?, ?)
"""  # ? are SQLite prepared statement substitutes

var select_data_query = """
SELECT dict FROM byte_data WHERE id=? LIMIT 1
"""


func _ready():
	print(OS.get_user_data_dir())
	if FileAccess.file_exists(db_path):
		load_db()
	else:
		create_db()


func load_db():
	db.open(db_path)
	var query = db.create_query(select_data_query)
	var result = query.execute([0])
	if result and not result.is_empty():
		var columns = query.get_columns()
		var data = bytes_to_var(result[0][columns.find("dict")])
		print("Byte data retrieved. Time of creation: %d:%d:%d" % [data["time_created"]["hour"], data["time_created"]["minute"], data["time_created"]["second"]])
	else:
		print("Failed to retrieve byte data")


func create_db():
	db.open(db_path)
	var data = Dictionary()
	data["time_created"] = Time.get_time_dict_from_unix_time(true)
	print("New byte data created. Current time: %d:%d:%d" % [data["time_created"]["hour"], data["time_created"]["minute"], data["time_created"]["second"]])
	var bytes = var_to_bytes(data)
	print(bytes.size())
	db.create_query(create_table_query).execute()
	db.create_query(insert_data_query).execute([0, bytes])
	db.close()
