#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/input_event.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>

#include <SDL3/SDL_video.h>

#include <libretro.h>
#include <readerwriterqueue.h>

#include "ThreadCommand.hpp"
#include "Core.hpp"
#include "EnvironmentHandler.hpp"
#include "VideoHandler.hpp"
#include "AudioHandler.hpp"
#include "InputHandler.hpp"
#include "OptionsHandler.hpp"
#include "MessageHandler.hpp"
#include "LogHandler.hpp"

class SDL_Window;

namespace SK
{
class Wrapper
{
public:
    ~Wrapper() = default;

    static Wrapper* GetInstance();

    void StartContent(godot::MeshInstance3D* node, const std::string& root_directory, const std::string& core_name, const std::string& game_path);
    void StopContent();

    const std::unordered_map<std::string, OptionCategory>& GetOptionCategories() const { return m_options_handler->GetCategories(); }
    const std::unordered_map<std::string, OptionDefinition>& GetOptions() const { return m_options_handler->GetOptions(); }
    void SetCoreOption(const std::string& key, const std::string& value);
    void SetGameOption(const std::string& key, const std::string& value);

    void _input(const godot::Ref<godot::InputEvent>& event);
    void _process(double delta);

    godot::MeshInstance3D* m_node;

    const std::string& GetRootDirectory() const;
    const std::string& GetTempDirectory() const;
    std::string GetCoreName() const;
    std::string GetGameName() const;

    std::unique_ptr<Core> m_core = nullptr;
    std::unique_ptr<EnvironmentHandler> m_environment_handler = nullptr;
    std::unique_ptr<VideoHandler> m_video_handler = nullptr;
    std::unique_ptr<AudioHandler> m_audio_handler = nullptr;
    std::unique_ptr<InputHandler> m_input_handler = nullptr;
    std::unique_ptr<OptionsHandler> m_options_handler = nullptr;
    std::unique_ptr<MessageHandler> m_message_handler = nullptr;
    std::unique_ptr<LogHandler> m_log_handler = nullptr;

    std::thread m_thread;
    moodycamel::ReaderWriterQueue<std::unique_ptr<ThreadCommand>> m_main_thread_commands_queue;
    std::mutex m_mutex;
    bool m_mutex_done = false;
    std::condition_variable m_condition_variable;
    bool m_running = false;

    std::string m_root_directory;
    std::string m_temp_directory;
    std::string m_username = "DefaultUser";
    retro_log_level m_log_level = RETRO_LOG_WARN;

    std::string m_game_path;

    std::vector<unsigned char> m_game_buffer;

    void StopEmulationThread();
    void EmulationThreadLoop();
    void CreateTexture(godot::Image::Format image_format, godot::PackedByteArray pixel_data, int32_t width, int32_t height, bool flip_y);
    void UpdateTexture(godot::PackedByteArray pixel_data, bool flip_y);

    bool Shutdown();

    static void LedInterfaceSetLedState(int32_t led, int32_t state);

private:
    Wrapper() = default;
    Wrapper(const Wrapper&) = delete;
    Wrapper& operator=(const Wrapper&) = delete;
    Wrapper(Wrapper&&) = delete;
    Wrapper& operator=(Wrapper&&) = delete;
};
}
