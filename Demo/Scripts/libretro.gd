extends Node

@export var monitor_node: MeshInstance3D
@export_global_dir var root_directory: String
@export var core_name: String
@export_global_file var rom_path: String

@onready var options_scene = preload("res://Scenes/core_options.tscn")
@onready var category_scene = preload("res://Scenes/core_option_category.tscn")
@onready var option_scene = preload("res://Scenes/core_option.tscn")

var scroll_container

func _ready():
	Libretro.ConnectOptionsReady(Callable(self, "_on_options_ready"))


func _unhandled_input(event):
	if event.is_action_pressed("retro_start_emulation"):
		if !monitor_node:
			print("monitor node not found")
			return
		
		clear_options()
		Libretro.StartContent(monitor_node, root_directory, core_name, rom_path)

	if event.is_action_pressed("retro_stop_emulation"):
		Libretro.StopContent()
		clear_options()
		
	if event.is_action_pressed("retro_toggle_core_options"):
		if scroll_container:
			scroll_container.visible = !scroll_container.visible


func _on_options_ready(categories, options):
	scroll_container = options_scene.instantiate()
	get_tree().current_scene.add_child(scroll_container)
	scroll_container.set_position(Vector2.ZERO)
	scroll_container.set_size(get_viewport().size)

	# Group option keys by category, collect uncategorized keys separately
	var options_by_category = {}
	for category_key in categories.keys():
		options_by_category[category_key] = []

	var other_option_keys = []
	for def_key in options.keys():
		var option : LibretroOptionDefinition = options[def_key]
		var cat_key = option.GetCategoryKey()
		if cat_key == "" or not categories.has(cat_key):
			other_option_keys.append(def_key)
		else:
			options_by_category[cat_key].append(def_key)
			
	var vbox_container = scroll_container.get_node("MarginContainer/VBoxContainer")
	while vbox_container.get_child_count() > 0:
		vbox_container.get_child(0).queue_free()

	# Render only categories that actually have options
	for category_key in categories.keys():
		var option_keys = options_by_category[category_key]
		if option_keys.is_empty():
			continue

		var category : LibretroOptionCategory = categories[category_key]
		var category_desc = category.GetDescription()
		var category_info = category.GetInfo()

		var category_box = category_scene.instantiate()
		vbox_container.add_child(category_box)
		var fold_button = category_box.get_node("CategoryHeaderHBoxContainer/CategoryFoldButton")
		var options_box = category_box.get_node("OptionsVBoxContainer")
		fold_button.connect("pressed", Callable(self, "_on_fold_pressed").bind(options_box, fold_button))
		var category_label = category_box.get_node("CategoryHeaderHBoxContainer/CategoryRichTextLabel")
		category_label.text = "[b][font_size=22]%s[/font_size][/b] [i][font_size=18]%s[/font_size][/i]" % [category_desc, category_info]

		for def_key in option_keys:
			var option : LibretroOptionDefinition = options[def_key]

			var option_desc = option.GetDescriptionCategorized()
			if option_desc.is_empty():
				option_desc = option.GetDescription()

			var option_info = option.GetInfoCategorized()
			if option_info.is_empty():
				option_info = option.GetInfo()

			var option_box = option_scene.instantiate()
			var option_label = option_box.get_node("OptionRichTextLabel")
			option_label.text = "\t[b][font_size=18]%s[/font_size][/b] [i][font_size=14]%s[/font_size][/i]" % [option_desc, option_info]

			var core_option_dropdown = option_box.get_node("CoreOptionMenuButton")
			core_option_dropdown.connect("item_selected", Callable(self, "_on_core_option_selected").bind(def_key, core_option_dropdown))

			var game_option_dropdown = option_box.get_node("GameOptionMenuButton")
			game_option_dropdown.connect("item_selected", Callable(self, "_on_game_option_selected").bind(def_key, game_option_dropdown))

			for value in option.GetPossibleValues():
				core_option_dropdown.add_item(value.GetValue())
				game_option_dropdown.add_item(value.GetValue())

			for i in range(core_option_dropdown.item_count):
				var dropdown_value = core_option_dropdown.get_item_text(i)
				var current_value = option.GetCoreValue()
				if dropdown_value == current_value:
					core_option_dropdown.select(i)
					break

			for i in range(game_option_dropdown.item_count):
				var dropdown_value = game_option_dropdown.get_item_text(i)
				var current_value = option.GetGameValue()
				if dropdown_value == current_value:
					game_option_dropdown.select(i)
					break

			options_box.add_child(option_box)

	if not other_option_keys.is_empty():
		var category_box = category_scene.instantiate()
		vbox_container.add_child(category_box)
		var fold_button = category_box.get_node("CategoryHeaderHBoxContainer/CategoryFoldButton")
		var options_box = category_box.get_node("OptionsVBoxContainer")
		fold_button.connect("pressed", Callable(self, "_on_fold_pressed").bind(options_box, fold_button))
		var category_label = category_box.get_node("CategoryHeaderHBoxContainer/CategoryRichTextLabel")
		category_label.text = "[b][font_size=22]Uncategorized[/font_size][/b]"

		for def_key in other_option_keys:
			var option : LibretroOptionDefinition = options[def_key]

			var option_desc = option.GetDescriptionCategorized()
			if option_desc.is_empty():
				option_desc = option.GetDescription()

			var option_info = option.GetInfoCategorized()
			if option_info.is_empty():
				option_info = option.GetInfo()

			var option_box = option_scene.instantiate()
			var option_label = option_box.get_node("OptionRichTextLabel")
			option_label.text = "\t[b][font_size=18]%s[/font_size][/b] [i][font_size=14]%s[/font_size][/i]" % [option_desc, option_info]

			var core_option_dropdown = option_box.get_node("CoreOptionMenuButton")
			core_option_dropdown.connect("item_selected", Callable(self, "_on_core_option_selected").bind(def_key, core_option_dropdown))

			var game_option_dropdown = option_box.get_node("GameOptionMenuButton")
			game_option_dropdown.connect("item_selected", Callable(self, "_on_game_option_selected").bind(def_key, game_option_dropdown))

			for value in option.GetPossibleValues():
				core_option_dropdown.add_item(value.GetValue())
				game_option_dropdown.add_item(value.GetValue())

			for i in range(core_option_dropdown.item_count):
				var dropdown_value = core_option_dropdown.get_item_text(i)
				var current_value = option.GetCoreValue()
				if dropdown_value == current_value:
					core_option_dropdown.select(i)
					break

			for i in range(game_option_dropdown.item_count):
				var dropdown_value = game_option_dropdown.get_item_text(i)
				var current_value = option.GetGameValue()
				if dropdown_value == current_value:
					game_option_dropdown.select(i)
					break

			options_box.add_child(option_box)

	scroll_container.visible = false

func _on_fold_pressed(options_box, fold_button):
	if options_box.visible:
		fold_button.text = "+"
	else:
		fold_button.text = "-"
	options_box.visible = !options_box.visible

func _on_core_option_selected(index, key, dropdown):
	Libretro.SetCoreOption(key, dropdown.get_item_text(index))

func _on_game_option_selected(index, key, dropdown):
	Libretro.SetGameOption(key, dropdown.get_item_text(index))

func clear_options():
	if scroll_container:
		scroll_container.queue_free()
		scroll_container = null
