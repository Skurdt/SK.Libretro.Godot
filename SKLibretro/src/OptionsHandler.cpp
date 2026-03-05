#include "OptionsHandler.hpp"

#include <algorithm>
#include <string_view>
#include <fstream>
#include <filesystem>

#include "Wrapper.hpp"
#include <unordered_set>

using namespace godot;

namespace SK
{
static std::vector<std::string> Split(const std::string_view& s, char delim)
{
    std::vector<std::string> out;
    size_t start = 0;
    while (true)
    {
        const auto& pos = s.find(delim, start);
        if (pos == std::string_view::npos)
        {
            out.emplace_back(s.substr(start));
            break;
        }
        out.emplace_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    return out;
}

static std::string Trim(const std::string_view& s)
{
    const auto& start = std::find_if_not(s.cbegin(),
                                         s.cend(),
                                         [](unsigned char c)
                                         {
                                             return std::isspace(c);
                                         });
    const auto& end = std::find_if_not(s.crbegin(),
                                       s.crend(),
                                       [](unsigned char c)
                                       {
                                           return std::isspace(c);
                                       }).base();
    if (start >= end)
        return "";
    return std::string(start, end);
}

static std::filesystem::path GetCoreOptionsFilePath()
{
    const auto& root_directory = Wrapper::GetInstance()->GetRootDirectory();
    const auto& core_name = Wrapper::GetInstance()->GetCoreName();
    return std::filesystem::path(root_directory) / "core_options" / (core_name + ".opt");
}

static std::filesystem::path GetGameOptionsFilePath()
{
    const auto& root_directory = Wrapper::GetInstance()->GetRootDirectory();
    const auto& core_name = Wrapper::GetInstance()->GetCoreName();
    const auto& game_name = Wrapper::GetInstance()->GetGameName();
    return !game_name.empty() ? std::filesystem::path(root_directory) / "core_options" / (core_name + "_" + game_name + ".opt") : std::filesystem::path();
}
    
static bool EnsureOptionsDirectory(const std::filesystem::path& filePath)
{
	auto parent = filePath.parent_path();
	if (parent.empty())
	{
		LogError("[OptionsHandler::EnsureOptionsDirectory] Options directory is empty (file path: " + filePath.string() + ")");
		return false;
	}

	if (std::filesystem::is_directory(parent))
		return true;

	std::error_code ec;
	std::filesystem::create_directories(filePath.parent_path(), ec);
	if (ec)
	{
		LogError("[OptionsHandler::EnsureOptionsDirectory] Failed to create options directory: " + filePath.parent_path().string() + " - " + ec.message());
		return false;
	}

	return true;
}

static void WriteOptionsFile(const std::filesystem::path& filePath, const std::unordered_map<std::string, std::string>& options)
{
	std::ofstream file(filePath, std::ofstream::trunc);
	if (!file.is_open())
	{
		LogError("[OptionsHandler::WriteOptionsFile] Failed to open options file for writing: " + filePath.string());
		return;
	}

	for (const auto& [key, value] : options)
		file << key << " = \"" << value << "\"\n";
}

static void ReadOptionsFile(const std::filesystem::path& filePath, const std::unordered_set<std::string>& allowedKeys, std::unordered_map<std::string, std::string>& outOptions)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		LogError("[OptionsHandler::ReadOptionsFile] Failed to open options file: " + filePath.string());
		return;
	}

	std::string line_str;
	while (std::getline(file, line_str))
	{
		std::string_view line(line_str);

		auto eq_pos = line.find('=');
		if (eq_pos == std::string_view::npos)
			continue;

		auto key = line.substr(0, eq_pos);
		auto key_begin = key.find_first_not_of(" \t");
		auto key_end = key.find_last_not_of(" \t");
		if (key_begin == std::string_view::npos || key_end == std::string_view::npos)
			continue;

		key = key.substr(key_begin, key_end - key_begin + 1);
		if (key.empty())
			continue;

		std::string key_str(key);
		if (!allowedKeys.contains(key_str))
			continue;

		auto value = line.substr(eq_pos + 1);
		auto value_begin = value.find_first_not_of(" \t");
		auto value_end = value.find_last_not_of(" \t");
		if (value_begin == std::string_view::npos || value_end == std::string_view::npos)
			continue;

		value = value.substr(value_begin, value_end - value_begin + 1);
		if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
			value = value.substr(1, value.size() - 2);

		outOptions[key_str] = std::string(value);
	}
}

bool OptionsHandler::GetCoreOptionsVersion(uint32_t* version)
{
    if (!version)
        return true;

    *version = OptionsHandler::SUPPORTED_CORE_OPTIONS_VERSION;
    return true;
}

bool OptionsHandler::GetVariable(retro_variable* variable)
{
    if (!variable)
    {
        Log("[OptionsHandler::GetVariable] Core passed a null retro_variable pointer (check for availability).");
        return true;
    }

    if (!variable->key)
    {
        LogWarning("[OptionsHandler::GetVariable] Core passed a null retro_variable->key pointer.");
        variable->value = nullptr;
        return true;
    }

    std::string key(variable->key);
    if (auto eff = GetEffectiveValue(key))
    {
        variable->value = eff->c_str();
        return true;
    }

    LogWarning("[OptionsHandler::GetVariable] Core requested unset option key: " + key);
    return true;
}

bool OptionsHandler::GetVariableUpdate(bool* variable_update)
{
    if (!variable_update)
        return true;

    *variable_update = m_variable_update;
    return SetVariableUpdate(false);
}

bool OptionsHandler::SetVariable(const retro_variable* variable)
{
	if (!variable)
	{
		LogWarning("[OptionsHandler::SetVariable] Core passed a null retro_variable pointer.");
		return true;
	}

	if (!variable->key
	 || strlen(variable->key) == 0
	 || !variable->value
	 || strlen(variable->value) == 0
	 || !m_options.contains(variable->key))
		return false;

    std::string key(variable->key);
    if (!m_options[key].game_value.empty())
        m_options[key].game_value = variable->value;
    else
        m_options[key].core_value = variable->value;

	SerializeToFile();
	return SetVariableUpdate(true);
}

bool OptionsHandler::SetVariables(const retro_variable* variables)
{
    if (!variables)
    {
        LogWarning("[OptionsHandler::SetVariables] Core passed a null retro_variable pointer.");
        return true;
    }

    m_options.clear();
    for (int i = 0; variables[i].key; ++i)
    {
        std::string value = variables[i].value ? variables[i].value : "";
        auto parts = Split(value, ';');
        if (parts.size() > 1)
        {
            auto choices = Split(parts[1], '|');
            if (!choices.empty())
            {
                std::vector<OptionValue> possible_values;
                for (auto choice : choices)
                    possible_values.emplace_back(Trim(choice), Trim(choice));
                m_options.emplace(variables[i].key, OptionDefinition{ .possible_values = std::move(possible_values), .default_value = Trim(choices[0]) });
            }
        }
    }

    DeserializeFromFile();
    return true;
}

bool OptionsHandler::SetVariableUpdate(bool update)
{
    m_variable_update = update;
    return true;
}

bool OptionsHandler::SetCoreOptions(const retro_core_option_definition* definitions)
{
    if (!definitions)
    {
        LogWarning("[OptionsHandler::SetCoreOptions] Core passed a null retro_core_option_definition pointer.");
        return true;
    }
    
    m_options.clear();
    for (auto definition = definitions; definition->key; ++definition)
    {
        std::vector<OptionValue> possible_values;
        for (auto option_value = definition->values; option_value->value; ++option_value)
            possible_values.emplace_back(option_value->value ? option_value->value : "",
                                         option_value->label ? option_value->label : "");
        m_options.emplace(definition->key, OptionDefinition{ .desc = definition->desc ? definition->desc : "",
                                                             .info = definition->info ? definition->info : "",
                                                             .possible_values = std::move(possible_values),
                                                             .default_value = definition->default_value ? definition->default_value : "" });
    }

    DeserializeFromFile();
    return true;
}

bool OptionsHandler::SetCoreOptions(const retro_core_options_intl* options_intl)
{
    if (!options_intl)
    {
        LogWarning("[OptionsHandler::SetCoreOptions] Core passed a null retro_core_options_intl pointer.");
        return true;
    }

    return SetCoreOptions(options_intl->local ? options_intl->local : options_intl->us);
}

bool OptionsHandler::SetCoreOptionsV2(const retro_core_options_v2* options)
{
    if (!options)
    {
        LogWarning("[OptionsHandler::SetCoreOptionsV2] Core passed a null retro_core_options_v2 pointer.");
        return true;
    }

    m_categories.clear();
    m_options.clear();

    if (options->categories)
        for (auto category = options->categories; category->key; ++category)
            m_categories.emplace(category->key, OptionCategory{ category->desc, category->info });

    for (auto definition = options->definitions; definition->key; ++definition)
    {
        std::vector<OptionValue> option_definition_values;
        for (auto option_definition_value = definition->values; option_definition_value->value; ++option_definition_value)
            option_definition_values.emplace_back(option_definition_value->value,
                                                  option_definition_value->label ? option_definition_value->label : option_definition_value->value);

        m_options.emplace(definition->key, OptionDefinition{ .desc = definition->desc ? definition->desc : "",
                                                             .desc_categorized = definition->desc_categorized ? definition->desc_categorized : definition->desc ? definition->desc : "",
                                                             .info = definition->info ? definition->info : "",
                                                             .info_categorized = definition->info_categorized ? definition->info_categorized : definition->info ? definition->info : "",
                                                             .category_key = definition->category_key ? definition->category_key : "",
                                                             .possible_values = std::move(option_definition_values),
                                                             .default_value = definition->default_value ? definition->default_value : "" });
    }

    DeserializeFromFile();
    return true;
}

bool OptionsHandler::SetCoreOptionsV2Intl(const retro_core_options_v2_intl* options)
{
    if (!options)
    {
        LogWarning("[OptionsHandler::SetCoreOptionsV2Intl] Core passed a null retro_core_options_v2_intl pointer.");
        return true;
    }

    return SetCoreOptionsV2(options->local ? options->local : options->us);
}

bool OptionsHandler::SetCoreOptionsUpdateDisplayCallback(const retro_core_options_update_display_callback* update_display_callback)
{
    m_core_options_update_display_callback = nullptr;

    if (update_display_callback)
        m_core_options_update_display_callback = update_display_callback->callback;

    return true;
}

void OptionsHandler::SetCoreOption(const std::string& key, const std::string& value)
{
    if (key.empty())
        return;

    if (!m_options.contains(key))
        return;

    m_options[key].core_value = value;
    SerializeToFile();
    SetVariableUpdate(true);
}

void OptionsHandler::SetGameOption(const std::string& key, const std::string& value)
{
    if (key.empty())
        return;

    if (!m_options.contains(key))
        return;

    m_options[key].game_value = value;
    SerializeToFile();
    SetVariableUpdate(true);
}

const std::string* const OptionsHandler::GetEffectiveValue(const std::string& key) const
{
    auto it_option = m_options.find(key);
    if (it_option == m_options.end())
        return nullptr;

    const auto& option = it_option->second;

    const auto& game_value = option.game_value;
    if (!game_value.empty())
        return &game_value;

    const auto& core_value = option.core_value;
    if (!core_value.empty())
        return &core_value;

    const auto& default_value = option.default_value;
    if (!default_value.empty())
        return &default_value;
    
	return nullptr;
}

void OptionsHandler::SerializeToFile()
{
	auto coreFilePath = GetCoreOptionsFilePath();
	if (coreFilePath.empty())
		return;

	if (!EnsureOptionsDirectory(coreFilePath))
		return;

	std::unordered_map<std::string, std::string> coreToWrite;
	for (const auto& [key, option] : m_options)
	{
		coreToWrite[key] = !option.core_value.empty() ? option.core_value : option.default_value;
	}
	WriteOptionsFile(coreFilePath, coreToWrite);

	auto gameFilePath = GetGameOptionsFilePath();
	if (gameFilePath.empty())
		return;

	std::unordered_map<std::string, std::string> gameToWrite;
	for (const auto& [key, option] : m_options)
	{
		if (!option.game_value.empty())
			gameToWrite[key] = option.game_value;
	}
	if (gameToWrite.empty())
		return;

	if (!EnsureOptionsDirectory(gameFilePath))
		return;

	WriteOptionsFile(gameFilePath, gameToWrite);
}

void OptionsHandler::DeserializeFromFile()
{
	auto core_file_path = GetCoreOptionsFilePath();
	if (core_file_path.empty())
		return;

	std::unordered_set<std::string> allowed;
	for (const auto& [k, v] : m_options) allowed.insert(k);
    if (!std::filesystem::is_regular_file(core_file_path))
    {
        // If core file missing, initialize core_value from defaults for all options
        for (auto& kv : m_options)
        {
            auto& opt = kv.second;
            opt.core_value = opt.default_value;
        }

        // Persist the initialized core values to disk.
        SerializeToFile();
    }
    else
    {
        std::unordered_map<std::string, std::string> core_overrides;
        ReadOptionsFile(core_file_path, allowed, core_overrides);

        // Update core_value for keys present in file, reset others to default
        for (auto& kv : m_options)
        {
            const auto& key = kv.first;
            auto& opt = kv.second;
            auto it = core_overrides.find(key);
            if (it != core_overrides.end())
                opt.core_value = it->second;
            else
                opt.core_value = opt.default_value;
        }
    }

    auto game_file_path = GetGameOptionsFilePath();
    // If game file exists, use it to set per-game overrides; otherwise clear all game_value
    if (!game_file_path.empty() && std::filesystem::is_regular_file(game_file_path))
    {
        std::unordered_map<std::string, std::string> game_overrides;
        ReadOptionsFile(game_file_path, allowed, game_overrides);

        for (auto& kv : m_options)
        {
            const auto& key = kv.first;
            auto& opt = kv.second;
            auto it = game_overrides.find(key);
            if (it != game_overrides.end())
                opt.game_value = it->second;
            else
                opt.game_value.clear();
        }
    }
    else
    {
        for (auto& kv : m_options)
            kv.second.game_value.clear();
    }
}
}
