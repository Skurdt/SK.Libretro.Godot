extends Node

@export var monitor_node: MeshInstance3D
@export_dir var core_directory: String
@export var core_name: String
@export_global_file var rom_path: String

@onready var options_scene = preload("res://Scenes/core_options.tscn")
@onready var category_scene = preload("res://Scenes/core_option_category.tscn")
@onready var option_scene = preload("res://Scenes/core_option.tscn")

var vbox

func _ready():
	Libretro.ConnectOptionsReady(Callable(self, "_on_options_ready"))


func _unhandled_input(event):
	if event.is_action_pressed("retro_start_emulation"):
		if !monitor_node:
			print("monitor node not found")
			return

		clear_options()
		Libretro.StartContent(monitor_node, core_directory, core_name, rom_path)

	if event.is_action_pressed("retro_stop_emulation"):
		Libretro.StopContent()
		clear_options()
		
	if event.is_action_pressed("retro_toggle_core_options"):
		if vbox:
			vbox.visible = !vbox.visible


func _on_options_ready(categories, definitions, values):
	vbox = options_scene.instantiate()
	get_tree().current_scene.add_child(vbox)
	vbox.set_position(Vector2.ZERO)
	vbox.set_size(get_viewport().size)

	for category_key in categories.keys():
		var category : LibretroOptionCategory = categories[category_key]
		var category_desc = category.GetDescription()
		var category_info = category.GetInfo()

		var category_box = category_scene.instantiate()
		vbox.add_child(category_box)
		var fold_button = category_box.get_node("CategoryHeaderHBoxContainer/CategoryFoldButton")
		var options_box = category_box.get_node("OptionsVBoxContainer")
		fold_button.connect("pressed", Callable(self, "_on_fold_pressed").bind(options_box, fold_button))
		var category_label = category_box.get_node("CategoryHeaderHBoxContainer/CategoryRichTextLabel")
		category_label.text = "[b][font_size=22]%s[/font_size][/b] [i][font_size=18]%s[/font_size][/i]" % [category_desc, category_info]

		for def_key in definitions.keys():
			var definition : LibretroOptionDefinition = definitions[def_key]
			if definition.GetCategoryKey() != category_key:
				continue

			var option_desc = definition.GetDescriptionCategorized()
			if option_desc.is_empty():
				option_desc = definition.GetDescription()

			var option_info = definition.GetInfoCategorized()
			if option_info.is_empty():
				option_info = definition.GetInfo()

			var option_box = option_scene.instantiate()
			var option_label = option_box.get_node("OptionRichTextLabel")
			option_label.text = "\t[b][font_size=18]%s[/font_size][/b] [i][font_size=14]%s[/font_size][/i]" % [option_desc, option_info]

			var core_option_dropdown = option_box.get_node("CoreOptionMenuButton")
			core_option_dropdown.connect("item_selected", Callable(self, "_on_core_option_selected").bind(def_key, core_option_dropdown))

			var game_option_dropdown = option_box.get_node("GameOptionMenuButton")
			for value in definition.GetValues():
				core_option_dropdown.add_item(value.GetValue())
				game_option_dropdown.add_item(value.GetValue())

			if values.has(def_key):
				for i in range(core_option_dropdown.item_count):
					var dropdown_value = core_option_dropdown.get_item_text(i)
					if dropdown_value == values[def_key]:
						core_option_dropdown.select(i)
						break

				for i in range(game_option_dropdown.item_count):
					var dropdown_value = game_option_dropdown.get_item_text(i)
					if dropdown_value == values[def_key]:
						game_option_dropdown.select(i)
						break
			
			# TODO: enable when game options will be available
			game_option_dropdown.visible = false
			
			options_box.add_child(option_box)

	vbox.visible = false

func _on_fold_pressed(options_box, fold_button):
	if options_box.visible:
		fold_button.text = "+"
	else:
		fold_button.text = "-"
	options_box.visible = !options_box.visible

func _on_core_option_selected(index, key, dropdown):
	Libretro.SetCoreOption(key, dropdown.get_item_text(index))

func clear_options():
	if vbox:
		vbox.queue_free()
