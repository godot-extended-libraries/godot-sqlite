extends Node

onready var db = SQLite.new();

var db_path = "user://bytes_db.sqlite"

var create_table_query = """
CREATE TABLE IF NOT EXISTS byte_data (
	id INTEGER PRIMARY KEY,
	dict BLOB NOT NULL
);
"""

var insert_data_query = """
INSERT INTO byte_data VALUES (?, ?)
""" # ? are SQLite prepared statement substitutes

var select_data_query = """
SELECT dict FROM byte_data WHERE id=? LIMIT 1
"""

func _ready():
	print(OS.get_user_data_dir());
	var file = File.new();
	if file.file_exists(db_path):
		load_db();
	else:
		create_db();

func load_db():
	db.open(db_path);
	var result = db.fetch_assoc_with_args(select_data_query, [0]);
	if result:
		var data = bytes2var(result[0]["dict"]);
		print("Byte data retrieved. Time of creation: %d:%d:%d" % 
			[data["time_created"]["hour"], 
			data["time_created"]["minute"], 
			data["time_created"]["second"]]);
	else:
		print("Failed to retrieve byte data");

func create_db():
	db.open(db_path);
	var data = Dictionary();
	data["time_created"] = OS.get_time(true);
	print("New byte data created. Current time: %d:%d:%d" % 
		[data["time_created"]["hour"], 
		data["time_created"]["minute"], 
		data["time_created"]["second"]]);
	var bytes = var2bytes(data);
	print(bytes.size());
	db.query(create_table_query)
	db.query_with_args(insert_data_query, [0, bytes]);
	db.close();
