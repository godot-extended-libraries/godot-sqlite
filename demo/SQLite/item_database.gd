extends Node

# SQLite module
# Variables
var item_list = []


func _ready() -> void:
	# Create new gdsqlite instance
	var db = SQLite.new()

	# Open item database
	if !open_database(db, "res://items.db"):
		print("Failed opening database.")
		return

	var get_item_list : SQLiteQuery = db.create_query("SELECT * FROM potion ORDER BY id ASC")
	var pots = get_item_list.execute([])
	if pots.is_empty():
		return
	
	var columns = get_item_list.get_columns()
	for pot in pots:
		# Create new item from database
		var item = {
			"id": pot[columns.find("id")], "name": pot[columns.find("name")], "price": pot[columns.find("price")], "heals": pot[columns.find("heals")]
		}

		# Add to item list
		item_list.append(item)

	# Print all item
	for i in item_list:
		print("Item ", i.id, " (", i.name, ") $", i.price, " +", i.heals, "hp")


func open_database(db: SQLite, path: String) -> bool:
	if path.begins_with("res://"):
		# Open packed database
		var file = FileAccess.open(path, FileAccess.READ)
		if not file:
			return false
		var size = file.get_length()
		var buffers = file.get_buffer(size)
		return db.open_buffered(path, buffers, size)

	# Open database normally
	return db.open(path)
