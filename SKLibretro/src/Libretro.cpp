#include "Libretro.hpp"

#include "Wrapper.hpp"

using namespace godot;

namespace SK
{
Libretro* Libretro::m_instance = nullptr;

Libretro::Libretro()
{
    if (m_instance)
    {
        print_error("Libretro singleton already exists! Freeing duplicate.");
        queue_free();
        return;
    }

    m_instance = this;
}

void Libretro::ConnectOptionsReady(const godot::Callable& callable, uint32_t flags)
{
    if (m_instance)
        m_instance->connect("options_ready", callable, flags);
}

void Libretro::StartContent(MeshInstance3D* node, String root_directory, String core_name, String game_path)
{
    Wrapper::GetInstance()->StartContent(node, root_directory.utf8().get_data(), core_name.utf8().get_data(), game_path.utf8().get_data());
}

void Libretro::StopContent()
{
    Wrapper::GetInstance()->StopContent();
}

void Libretro::SetCoreOption(const godot::String& key, const godot::String& value)
{
    Wrapper::GetInstance()->SetCoreOption(key.utf8().get_data(), value.utf8().get_data());
}

void Libretro::SetGameOption(const godot::String& key, const godot::String& value)
{
    Wrapper::GetInstance()->SetGameOption(key.utf8().get_data(), value.utf8().get_data());
}

void Libretro::_exit_tree()
{
    StopContent();

    if (m_instance == this)
        m_instance = nullptr;
}

void Libretro::_input(const Ref<InputEvent>& event)
{
    Wrapper::GetInstance()->_input(event);
}

void Libretro::_process(double delta)
{
    Wrapper::GetInstance()->_process(delta);
}

void Libretro::NotifyOptionsReady()
{
    if (!m_instance)
        return;

    auto categories = m_instance->GetOptionCategories();
    auto options    = m_instance->GetOptions();
    m_instance->call_deferred("emit_signal", "options_ready", categories, options);
}

Dictionary Libretro::GetOptionCategories()
{
    Dictionary result;
    const auto& categories = Wrapper::GetInstance()->GetOptionCategories();
    for (const auto& [key, value] : categories)
    {
        Ref<LibretroOptionCategory> category = memnew(LibretroOptionCategory);
        category->m_desc = value.desc.c_str();
        category->m_info = value.info.c_str();
        result[String(key.c_str())] = category;
    }
    return result;
}

Dictionary Libretro::GetOptions()
{
    Dictionary result;
    const auto& definitions = Wrapper::GetInstance()->GetOptions();
    for (const auto& [key, value] : definitions)
    {
        Ref<LibretroOptionDefinition> definition = memnew(LibretroOptionDefinition);
        definition->m_desc = value.desc.c_str();
        definition->m_desc_categorized = value.desc_categorized.c_str();
        definition->m_info = value.info.c_str();
        definition->m_info_categorized = value.info_categorized.c_str();
        definition->m_category_key = value.category_key.c_str();
        definition->m_possible_values = Array();
        for (const auto& val : value.possible_values)
        {
            Ref<LibretroOptionValue> option_value = memnew(LibretroOptionValue);
            option_value->m_value = val.value.c_str();
            option_value->m_label = val.label.c_str();
            definition->m_possible_values.append(option_value);
        }
        definition->m_default_value = value.default_value.c_str();
        definition->m_core_value = value.core_value.c_str();
        definition->m_game_value = value.game_value.c_str();
        result[String(key.c_str())] = definition;
    }
    return result;
}

void Libretro::_bind_methods()
{
    ClassDB::bind_static_method("Libretro", D_METHOD("ConnectOptionsReady", "callable", "flags"), &ConnectOptionsReady, DEFVAL(0u));
    ClassDB::bind_static_method("Libretro", D_METHOD("StartContent", "node", "root_directory", "core_name", "game_path"), &StartContent);
    ClassDB::bind_static_method("Libretro", D_METHOD("StopContent"), &StopContent);
    ClassDB::bind_static_method("Libretro", D_METHOD("SetCoreOption"), &SetCoreOption);
    ClassDB::bind_static_method("Libretro", D_METHOD("SetGameOption"), &SetGameOption);

    ADD_SIGNAL(MethodInfo("options_ready", PropertyInfo(Variant::DICTIONARY, "categories"), PropertyInfo(Variant::DICTIONARY, "options")));
}
}
