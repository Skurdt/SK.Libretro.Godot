#include "Wrapper.hpp"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_action.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

#include <filesystem>
#include <fstream>
#include <chrono>

#include "Libretro.hpp"
#include "Debug.hpp"
#include "ThreadCommandInitAudio.hpp"
#include "ThreadCommandCreateTexture.hpp"
#include "ThreadCommandUpdateTexture.hpp"

using namespace godot;

namespace SK
{
Wrapper* Wrapper::GetInstance()
{
    static Wrapper instance;
    static std::mutex instance_mutex;
    std::lock_guard<std::mutex> lock(instance_mutex);
    return &instance;
}

static retro_key GodotToLibretroKeycode(const Ref<InputEventKey>& keyEvent)
{
    switch (keyEvent->get_keycode())
    {
        case KEY_NONE: return RETROK_UNKNOWN;
        case KEY_BACKSPACE: return RETROK_BACKSPACE;
        case KEY_TAB: return RETROK_TAB;
        case KEY_CLEAR: return RETROK_CLEAR;
        case KEY_ENTER: return RETROK_RETURN;
        case KEY_PAUSE: return RETROK_PAUSE;
        case KEY_ESCAPE: return RETROK_ESCAPE;
        case KEY_SPACE: return RETROK_SPACE;
        case KEY_EXCLAM: return RETROK_EXCLAIM;
        case KEY_QUOTEDBL: return RETROK_QUOTEDBL;
        case KEY_NUMBERSIGN: return RETROK_HASH;
        case KEY_DOLLAR: return RETROK_DOLLAR;
        case KEY_AMPERSAND: return RETROK_AMPERSAND;
        case KEY_APOSTROPHE: return RETROK_QUOTE;
        case KEY_PARENLEFT: return RETROK_LEFTPAREN;
        case KEY_PARENRIGHT: return RETROK_RIGHTPAREN;
        case KEY_ASTERISK: return RETROK_ASTERISK;
        case KEY_PLUS: return RETROK_PLUS;
        case KEY_COMMA: return RETROK_COMMA;
        case KEY_MINUS: return RETROK_MINUS;
        case KEY_PERIOD: return RETROK_PERIOD;
        case KEY_SLASH: return RETROK_SLASH;
        case KEY_0: return RETROK_0;
        case KEY_1: return RETROK_1;
        case KEY_2: return RETROK_2;
        case KEY_3: return RETROK_3;
        case KEY_4: return RETROK_4;
        case KEY_5: return RETROK_5;
        case KEY_6: return RETROK_6;
        case KEY_7: return RETROK_7;
        case KEY_8: return RETROK_8;
        case KEY_9: return RETROK_9;
        case KEY_COLON: return RETROK_COLON;
        case KEY_SEMICOLON: return RETROK_SEMICOLON;
        case KEY_LESS: return RETROK_LESS;
        case KEY_EQUAL: return RETROK_EQUALS;
        case KEY_GREATER: return RETROK_GREATER;
        case KEY_QUESTION: return RETROK_QUESTION;
        case KEY_AT: return RETROK_AT;
        case KEY_BRACKETLEFT: return RETROK_LEFTBRACKET;
        case KEY_BACKSLASH: return RETROK_BACKSLASH;
        case KEY_BRACKETRIGHT: return RETROK_RIGHTBRACKET;
        case KEY_ASCIICIRCUM: return RETROK_CARET;
        case KEY_UNDERSCORE: return RETROK_UNDERSCORE;
        case KEY_QUOTELEFT: return RETROK_BACKQUOTE;
        case KEY_A: return RETROK_a;
        case KEY_B: return RETROK_b;
        case KEY_C: return RETROK_c;
        case KEY_D: return RETROK_d;
        case KEY_E: return RETROK_e;
        case KEY_F: return RETROK_f;
        case KEY_G: return RETROK_g;
        case KEY_H: return RETROK_h;
        case KEY_I: return RETROK_i;
        case KEY_J: return RETROK_j;
        case KEY_K: return RETROK_k;
        case KEY_L: return RETROK_l;
        case KEY_M: return RETROK_m;
        case KEY_N: return RETROK_n;
        case KEY_O: return RETROK_o;
        case KEY_P: return RETROK_p;
        case KEY_Q: return RETROK_q;
        case KEY_R: return RETROK_r;
        case KEY_S: return RETROK_s;
        case KEY_T: return RETROK_t;
        case KEY_U: return RETROK_u;
        case KEY_V: return RETROK_v;
        case KEY_W: return RETROK_w;
        case KEY_X: return RETROK_x;
        case KEY_Y: return RETROK_y;
        case KEY_Z: return RETROK_z;
        case KEY_BRACELEFT: return RETROK_LEFTBRACE;
        case KEY_BAR: return RETROK_BAR;
        case KEY_BRACERIGHT: return RETROK_RIGHTBRACE;
        case KEY_ASCIITILDE: return RETROK_TILDE;
        case KEY_DELETE: return RETROK_DELETE;
        case KEY_KP_0: return RETROK_KP0;
        case KEY_KP_1: return RETROK_KP1;
        case KEY_KP_2: return RETROK_KP2;
        case KEY_KP_3: return RETROK_KP3;
        case KEY_KP_4: return RETROK_KP4;
        case KEY_KP_5: return RETROK_KP5;
        case KEY_KP_6: return RETROK_KP6;
        case KEY_KP_7: return RETROK_KP7;
        case KEY_KP_8: return RETROK_KP8;
        case KEY_KP_9: return RETROK_KP9;
        case KEY_KP_PERIOD: return RETROK_KP_PERIOD;
        case KEY_KP_DIVIDE: return RETROK_KP_DIVIDE;
        case KEY_KP_MULTIPLY: return RETROK_KP_MULTIPLY;
        case KEY_KP_SUBTRACT: return RETROK_KP_MINUS;
        case KEY_KP_ADD: return RETROK_KP_PLUS;
        case KEY_KP_ENTER: return RETROK_KP_ENTER;
        // case KEY_KP_EQUALS: return RETROK_KP_EQUALS;
        case KEY_UP: return RETROK_UP;
        case KEY_DOWN: return RETROK_DOWN;
        case KEY_RIGHT: return RETROK_RIGHT;
        case KEY_LEFT: return RETROK_LEFT;
        case KEY_INSERT: return RETROK_INSERT;
        case KEY_HOME: return RETROK_HOME;
        case KEY_END: return RETROK_END;
        case KEY_PAGEUP: return RETROK_PAGEUP;
        case KEY_PAGEDOWN: return RETROK_PAGEDOWN;
        case KEY_F1: return RETROK_F1;
        case KEY_F2: return RETROK_F2;
        case KEY_F3: return RETROK_F3;
        case KEY_F4: return RETROK_F4;
        case KEY_F5: return RETROK_F5;
        case KEY_F6: return RETROK_F6;
        case KEY_F7: return RETROK_F7;
        case KEY_F8: return RETROK_F8;
        case KEY_F9: return RETROK_F9;
        case KEY_F10: return RETROK_F10;
        case KEY_F11: return RETROK_F11;
        case KEY_F12: return RETROK_F12;
        case KEY_F13: return RETROK_F13;
        case KEY_F14: return RETROK_F14;
        case KEY_F15: return RETROK_F15;
        case KEY_NUMLOCK: return RETROK_NUMLOCK;
        case KEY_CAPSLOCK: return RETROK_CAPSLOCK;
        case KEY_SCROLLLOCK: return RETROK_SCROLLOCK;
        case KEY_SHIFT:
        {
            if (keyEvent->get_location() == KeyLocation::KEY_LOCATION_LEFT)
                return RETROK_LSHIFT;
            if (keyEvent->get_location() == KeyLocation::KEY_LOCATION_RIGHT)
                return RETROK_RSHIFT;
        }
        break;
        case KEY_CTRL:
        {
            if (keyEvent->get_location() == KeyLocation::KEY_LOCATION_LEFT)
                return RETROK_LCTRL;
            if (keyEvent->get_location() == KeyLocation::KEY_LOCATION_RIGHT)
                return RETROK_RCTRL;
        }
        break;
        case KEY_ALT:
        {
            if (keyEvent->get_location() == KeyLocation::KEY_LOCATION_LEFT)
                return RETROK_LALT;
            if (keyEvent->get_location() == KeyLocation::KEY_LOCATION_RIGHT)
                return RETROK_RALT;
        }
        break;
        case KEY_META:
        {
            // NOTE: may need to return RETROK_LSUPER/RETK_RSUPER instead for some platforms
            if (keyEvent->get_location() == KeyLocation::KEY_LOCATION_LEFT)
                return RETROK_LMETA;
            if (keyEvent->get_location() == KeyLocation::KEY_LOCATION_RIGHT)
                return RETROK_RMETA;
        }
        break;
        // case KEY_MODE: return RETROK_MODE;
        // case KEY_COMPOSE: return RETROK_COMPOSE;
        case KEY_HELP: return RETROK_HELP;
        case KEY_PRINT: return RETROK_PRINT;
        case KEY_SYSREQ: return RETROK_SYSREQ;
        // case KEY_BREAK: return RETROK_BREAK;
        case KEY_MENU: return RETROK_MENU;
        // case KEY_POWER: return RETROK_POWER;
        // case KEY_EURO: return RETROK_EURO;
        // case KEY_UNDO: return RETROK_UNDO;
        // case KEY_OEM_102: return RETROK_OEM_102;
        case KEY_BACK: return RETROK_BROWSER_BACK;
        case KEY_FORWARD: return RETROK_BROWSER_FORWARD;
        case KEY_REFRESH: return RETROK_BROWSER_REFRESH;
        case KEY_STOP: return RETROK_BROWSER_STOP;
        case KEY_SEARCH: return RETROK_BROWSER_SEARCH;
        case KEY_FAVORITES: return RETROK_BROWSER_FAVORITES;
        case KEY_HOMEPAGE: return RETROK_BROWSER_HOME;
        case KEY_VOLUMEMUTE: return RETROK_VOLUME_MUTE;
        case KEY_VOLUMEDOWN: return RETROK_VOLUME_DOWN;
        case KEY_VOLUMEUP: return RETROK_VOLUME_UP;
        case KEY_MEDIANEXT: return RETROK_MEDIA_NEXT;
        case KEY_MEDIAPREVIOUS: return RETROK_MEDIA_PREV;
        case KEY_MEDIASTOP: return RETROK_MEDIA_STOP;
        case KEY_MEDIAPLAY: return RETROK_MEDIA_PLAY_PAUSE;
        case KEY_LAUNCHMAIL: return RETROK_LAUNCH_MAIL;
        case KEY_LAUNCHMEDIA: return RETROK_LAUNCH_MEDIA;
        // case KEY_LAUNCH_APP1: return RETROK_LAUNCH_APP1;
        // case KEY_LAUNCH_APP2: return RETROK_LAUNCH_APP2;

        // Unhandled godot keys
        case KEY_SPECIAL: return RETROK_UNKNOWN;
        case KEY_BACKTAB: return RETROK_UNKNOWN;
        case KEY_F16: return RETROK_UNKNOWN;
        case KEY_F17: return RETROK_UNKNOWN;
        case KEY_F18: return RETROK_UNKNOWN;
        case KEY_F19: return RETROK_UNKNOWN;
        case KEY_F20: return RETROK_UNKNOWN;
        case KEY_F21: return RETROK_UNKNOWN;
        case KEY_F22: return RETROK_UNKNOWN;
        case KEY_F23: return RETROK_UNKNOWN;
        case KEY_F24: return RETROK_UNKNOWN;
        case KEY_F25: return RETROK_UNKNOWN;
        case KEY_F26: return RETROK_UNKNOWN;
        case KEY_F27: return RETROK_UNKNOWN;
        case KEY_F28: return RETROK_UNKNOWN;
        case KEY_F29: return RETROK_UNKNOWN;
        case KEY_F30: return RETROK_UNKNOWN;
        case KEY_F31: return RETROK_UNKNOWN;
        case KEY_F32: return RETROK_UNKNOWN;
        case KEY_F33: return RETROK_UNKNOWN;
        case KEY_F34: return RETROK_UNKNOWN;
        case KEY_F35: return RETROK_UNKNOWN;
        case KEY_HYPER: return RETROK_UNKNOWN;
        case KEY_MEDIARECORD: return RETROK_UNKNOWN;
        case KEY_STANDBY: return RETROK_UNKNOWN;
        case KEY_OPENURL: return RETROK_UNKNOWN;
        case KEY_LAUNCH0: return RETROK_UNKNOWN;
        case KEY_LAUNCH1: return RETROK_UNKNOWN;
        case KEY_LAUNCH2: return RETROK_UNKNOWN;
        case KEY_LAUNCH3: return RETROK_UNKNOWN;
        case KEY_LAUNCH4: return RETROK_UNKNOWN;
        case KEY_LAUNCH5: return RETROK_UNKNOWN;
        case KEY_LAUNCH6: return RETROK_UNKNOWN;
        case KEY_LAUNCH7: return RETROK_UNKNOWN;
        case KEY_LAUNCH8: return RETROK_UNKNOWN;
        case KEY_LAUNCH9: return RETROK_UNKNOWN;
        case KEY_LAUNCHA: return RETROK_UNKNOWN;
        case KEY_LAUNCHB: return RETROK_UNKNOWN;
        case KEY_LAUNCHC: return RETROK_UNKNOWN;
        case KEY_LAUNCHD: return RETROK_UNKNOWN;
        case KEY_LAUNCHE: return RETROK_UNKNOWN;
        case KEY_LAUNCHF: return RETROK_UNKNOWN;
        case KEY_GLOBE: return RETROK_UNKNOWN;
        case KEY_KEYBOARD: return RETROK_UNKNOWN;
        case KEY_JIS_EISU: return RETROK_UNKNOWN;
        case KEY_JIS_KANA: return RETROK_UNKNOWN;
        case KEY_UNKNOWN: return RETROK_UNKNOWN;
        case KEY_PERCENT: return RETROK_UNKNOWN;
        case KEY_YEN: return RETROK_UNKNOWN;
        case KEY_SECTION: return RETROK_UNKNOWN;
        default:
            LogError("Unmapped key code from Godot to Libretro: " + std::to_string(keyEvent->get_keycode()));
            break;
    };

    return RETROK_UNKNOWN;
}

static int16_t ToShort(float floatValue, int mul = 1)
{
    return static_cast<int16_t>(Math::clamp(Math::round(floatValue), static_cast<float>(INT16_MIN), static_cast<float>(INT16_MAX)) * mul);
}

void Wrapper::StartContent(MeshInstance3D* node, const std::string& root_directory, const std::string& core_name, const std::string& game_path)
{
    if (!node)
        return;

    StopEmulationThread();

    m_node = node;

    auto audio_stream_player = memnew(AudioStreamPlayer);
    audio_stream_player->set_name("AudioStreamPlayer");
    m_node->add_child(audio_stream_player);

    std::filesystem::path core_path = std::filesystem::path(root_directory).append("cores").append(core_name + "_libretro").replace_extension(".dll");

    m_core = std::make_unique<Core>(core_path.string());
    m_environment_handler = std::make_unique<EnvironmentHandler>();
    m_video_handler = std::make_unique<VideoHandler>();
    m_audio_handler = std::make_unique<AudioHandler>();
    m_input_handler = std::make_unique<InputHandler>();
    m_options_handler = std::make_unique<OptionsHandler>();
    m_message_handler = std::make_unique<MessageHandler>();
    m_log_handler = std::make_unique<LogHandler>();

    m_video_handler->Init(node);

    m_root_directory = root_directory;
    m_temp_directory = std::filesystem::path(root_directory).append("temp").string();
    m_game_path = game_path;
    std::string system_directory = std::filesystem::path(root_directory).append("system").append(core_name).string();
    std::string save_directory = std::filesystem::path(root_directory).append("save").append(core_name).string();
    std::string core_assets_directory = std::filesystem::path(root_directory).append("core_assets").append(core_name).string();
    m_environment_handler->SetDirectories(system_directory, save_directory, core_assets_directory);

    if (!std::filesystem::is_directory(m_temp_directory))
    {
        std::error_code ec;
        std::filesystem::create_directories(m_temp_directory, ec);
        if (ec)
        {
            LogError("Failed to create temp directory: " + m_temp_directory + " - " + ec.message());
            return;
        }
    }

    m_thread = std::thread(&Wrapper::EmulationThreadLoop, this);
}

void Wrapper::StopContent()
{
    StopEmulationThread();
}

void Wrapper::SetCoreOption(const std::string& key, const std::string& value)
{
    if (m_options_handler)
        m_options_handler->SetCoreOption(key, value);
}

void Wrapper::SetGameOption(const std::string& key, const std::string& value)
{
    if (m_options_handler)
        m_options_handler->SetGameOption(key, value);
}

void Wrapper::_input(const godot::Ref<godot::InputEvent>& event)
{
    if (!m_running)
        return;

    Ref<InputEventMouseMotion> mouseMotion = event;
    if (mouseMotion.is_valid())
    {
        auto mouseMotionValue = mouseMotion->get_relative();
        m_input_handler->SetMousePosition(0, ToShort(mouseMotionValue.x), ToShort(mouseMotionValue.y));
    }

    Ref<InputEventMouseButton> mouseButton = event;
    if (mouseButton.is_valid())
    {
        int button_index = mouseButton->get_button_index();
        bool pressed     = mouseButton->is_pressed();

        auto buttons = m_input_handler->GetMouseButtons(0);

        switch (button_index)
        {
            case MouseButton::MOUSE_BUTTON_LEFT:
                if (pressed)
                    buttons |= (1 << RETRO_DEVICE_ID_MOUSE_LEFT);
                else
                    buttons &= ~(1 << RETRO_DEVICE_ID_MOUSE_LEFT);
                break;
            case MouseButton::MOUSE_BUTTON_RIGHT:
                if (pressed)
                    buttons |= (1 << RETRO_DEVICE_ID_MOUSE_RIGHT);
                else
                    buttons &= ~(1 << RETRO_DEVICE_ID_MOUSE_RIGHT);
                break;
            case MouseButton::MOUSE_BUTTON_MIDDLE:
                if (pressed)
                    buttons |= (1 << RETRO_DEVICE_ID_MOUSE_MIDDLE);
                else
                    buttons &= ~(1 << RETRO_DEVICE_ID_MOUSE_MIDDLE);
                break;
            case MouseButton::MOUSE_BUTTON_WHEEL_UP:
                if (pressed)
                    buttons |= (1 << RETRO_DEVICE_ID_MOUSE_WHEELUP);
                else
                    buttons &= ~(1 << RETRO_DEVICE_ID_MOUSE_WHEELUP);
                break;
            case MouseButton::MOUSE_BUTTON_WHEEL_DOWN:
                if (pressed)
                    buttons |= (1 << RETRO_DEVICE_ID_MOUSE_WHEELDOWN);
                else
                    buttons &= ~(1 << RETRO_DEVICE_ID_MOUSE_WHEELDOWN);
                break;
            case MouseButton::MOUSE_BUTTON_WHEEL_LEFT:
                if (pressed)
                    buttons |= (1 << RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP);
                else
                    buttons &= ~(1 << RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP);
                break;
            case MouseButton::MOUSE_BUTTON_WHEEL_RIGHT:
                if (pressed)
                    buttons |= (1 << RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN);
                else
                    buttons &= ~(1 << RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN);
                break;
            case MouseButton::MOUSE_BUTTON_XBUTTON1:
                if (pressed)
                    buttons |= (1 << RETRO_DEVICE_ID_MOUSE_BUTTON_4);
                else
                    buttons &= ~(1 << RETRO_DEVICE_ID_MOUSE_BUTTON_4);
                break;
            case MouseButton::MOUSE_BUTTON_XBUTTON2:
                if (pressed)
                    buttons |= (1 << RETRO_DEVICE_ID_MOUSE_BUTTON_5);
                else
                    buttons &= ~(1 << RETRO_DEVICE_ID_MOUSE_BUTTON_5);
                break;
            default:
                print_line_rich("[color=red][SK.Libretro][/color] Unhandled mouse button: " + String::num_int64(button_index));
                break;
        }

        m_input_handler->SetMouseButtons(0, buttons);
    }

    Ref<InputEventKey> keyEvent = event;
    if (keyEvent.is_valid())
    {
        auto keys = m_input_handler->GetKeyboardKeys(0);

        bool down             = keyEvent->is_pressed();
        uint32_t keycode      = GodotToLibretroKeycode(keyEvent);
        uint32_t character    = keyEvent->get_unicode();
        uint16_t keyModifiers = 0;
        auto mods             = keyEvent->get_modifiers_mask();
        if (mods & KeyModifierMask::KEY_MASK_SHIFT)
            keyModifiers |= RETROKMOD_SHIFT;
        if (mods & KeyModifierMask::KEY_MASK_CTRL)
            keyModifiers |= RETROKMOD_CTRL;
        if (mods & KeyModifierMask::KEY_MASK_ALT)
            keyModifiers |= RETROKMOD_ALT;
        if (mods & KeyModifierMask::KEY_MASK_META)
            keyModifiers |= RETROKMOD_META;

        if (down)
            keys |= (1 << keycode);
        else
            keys &= ~(1 << keycode);

        m_input_handler->SetKeyboardKeys(0, keys);

        m_input_handler->CallKeyboardEventCallback(down, keycode, character, keyModifiers);
    }
}

void Wrapper::_process(double delta)
{
    if (!m_running)
        return;

    std::unique_ptr<ThreadCommand> command;
    while (m_main_thread_commands_queue.try_dequeue(command))
        command->Execute();

    auto input = godot::Input::get_singleton();

    uint32_t joypad_buttons = 0;

    if (input->is_action_pressed("RETRO_JOYPAD_B"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_B);
    if (input->is_action_pressed("RETRO_JOYPAD_Y"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_Y);
    if (input->is_action_pressed("RETRO_JOYPAD_SELECT"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_SELECT);
    if (input->is_action_pressed("RETRO_JOYPAD_START"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_START);
    if (input->is_action_pressed("RETRO_JOYPAD_UP"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_UP);
    if (input->is_action_pressed("RETRO_JOYPAD_DOWN"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_DOWN);
    if (input->is_action_pressed("RETRO_JOYPAD_LEFT"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_LEFT);
    if (input->is_action_pressed("RETRO_JOYPAD_RIGHT"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT);
    if (input->is_action_pressed("RETRO_JOYPAD_A"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_A);
    if (input->is_action_pressed("RETRO_JOYPAD_X"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_X);
    if (input->is_action_pressed("RETRO_JOYPAD_L"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_L);
    if (input->is_action_pressed("RETRO_JOYPAD_R"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_R);
    if (input->is_action_pressed("RETRO_JOYPAD_L2"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_L2);
    if (input->is_action_pressed("RETRO_JOYPAD_R2"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_R2);
    if (input->is_action_pressed("RETRO_JOYPAD_L3"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_L3);
    if (input->is_action_pressed("RETRO_JOYPAD_R3"))
        joypad_buttons |= (1 << RETRO_DEVICE_ID_JOYPAD_R3);

    m_input_handler->SetJoypadButtonStates(0, joypad_buttons);

    Vector2 analog_left = {};

    if (input->get_action_strength("RETRO_ANALOG_LEFT_X_NEGATIVE"))
        analog_left.x -= 1.0f;
    if (input->get_action_strength("RETRO_ANALOG_LEFT_X_POSITIVE"))
        analog_left.x += 1.0f;
    if (input->get_action_strength("RETRO_ANALOG_LEFT_Y_POSITIVE"))
        analog_left.y -= 1.0f;
    if (input->get_action_strength("RETRO_ANALOG_LEFT_Y_NEGATIVE"))
        analog_left.y += 1.0f;

    if (analog_left.length_squared() > 1.0f)
        analog_left = analog_left.normalized();

    m_input_handler->SetAnalogLeft(0, ToShort(analog_left.x) * 0x7fff, ToShort(analog_left.y) * 0x7fff);

    Vector2 analog_right = {};

    if (input->get_action_strength("RETRO_ANALOG_RIGHT_X_NEGATIVE"))
        analog_right.x -= 1.0f;
    if (input->get_action_strength("RETRO_ANALOG_RIGHT_X_POSITIVE"))
        analog_right.x += 1.0f;
    if (input->get_action_strength("RETRO_ANALOG_RIGHT_Y_POSITIVE"))
        analog_right.y -= 1.0f;
    if (input->get_action_strength("RETRO_ANALOG_RIGHT_Y_NEGATIVE"))
        analog_right.y += 1.0f;

    if (analog_right.length_squared() > 1.0f)
        analog_right = analog_right.normalized();

    m_input_handler->SetAnalogRight(0, ToShort(analog_right.x) * 0x7fff, ToShort(analog_right.y) * 0x7fff);
}

const std::string& Wrapper::GetRootDirectory() const
{
    return m_root_directory;
}

const std::string& Wrapper::GetTempDirectory() const
{
    return m_temp_directory;
}

std::string Wrapper::GetCoreName() const
{
    return m_core ? m_core->GetName() : std::string();
}

std::string Wrapper::GetGameName() const
{
    return m_game_path.empty() ? std::string() : std::filesystem::path(m_game_path).stem().string();
}

void Wrapper::StopEmulationThread()
{
    if (!m_core)
    {
        return;
    }

    m_running = false;
    m_thread.join();

    m_video_handler->DeInit();
    m_audio_handler->DeInit();

    m_core->Unload();

    m_core = nullptr;
    m_environment_handler = nullptr;
    m_video_handler = nullptr;
    m_audio_handler = nullptr;
    m_input_handler = nullptr;
    m_options_handler = nullptr;
    m_message_handler = nullptr;
    m_log_handler = nullptr;

    m_node = nullptr;
}

void Wrapper::EmulationThreadLoop()
{
    Log("Libretro Thread starting...");

    if (!m_core->Load())
        return;

    if (m_game_path.empty())
    {
        if (!m_core->GetSupportsNoGame())
        {
            LogError("Game not set and this core does not support no game mode.");
            return;
        }

        retro_game_info game_info = {};

        if (!m_core->retro_load_game(&game_info))
        {
            LogError("Failed to load game");
            return;
        }
    }
    else
    {
        if (!std::filesystem::is_regular_file(m_game_path))
        {
            LogError("Game not found: " + m_game_path);
            return;
        }

        std::ifstream file(m_game_path, std::ios::binary | std::ios::ate);
        if (!file)
        {
            LogError("Failed to open game file: " + m_game_path);
            return;
        }

        size_t game_size = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);
        
        m_game_buffer.resize(game_size);
        if (!file.read(reinterpret_cast<char*>(m_game_buffer.data()), game_size))
        {
            LogError("Failed to read game file: " + m_game_path);
            return;
        }

        retro_game_info game_info = {};
        game_info.path = m_game_path.c_str();
        game_info.data = reinterpret_cast<const void*>(m_game_buffer.data());
        game_info.size = game_size;
        game_info.meta = nullptr;

        if (!m_core->retro_load_game(&game_info))
        {
            LogError("Failed to load game");
            return;
        }
    }

    retro_system_av_info systemAvInfo = {};
    m_core->retro_get_system_av_info(&systemAvInfo);

    Log("FPS: " + std::to_string(systemAvInfo.timing.fps) + " Sample Rate: " + std::to_string(systemAvInfo.timing.sample_rate));

    if (!m_video_handler->InitHwRenderContext(systemAvInfo.geometry.base_width, systemAvInfo.geometry.base_height))
    {
        LogError("Failed to initialize video");
        return;
    }

    m_running = true;

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_mutex_done = false;
        m_main_thread_commands_queue.enqueue(std::make_unique<ThreadCommandInitAudio>(0.1f, systemAvInfo.timing.sample_rate));
        m_condition_variable.wait(lock, [&]{ return m_mutex_done; });
    }

    double frame_duration_ms = 1000.0 / systemAvInfo.timing.fps;
    auto last_time = std::chrono::steady_clock::now();
    double accumulator = 0.0;

    Libretro::NotifyOptionsReady();

    while (m_running)
    {
        if (!m_running)
            break;

        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - last_time).count();
        last_time = now;
        accumulator += elapsed;

        while (accumulator >= frame_duration_ms)
        {
            m_audio_handler->CallAudioBufferStatusCallback();

            m_core->retro_run();

            accumulator -= frame_duration_ms;
        }
    }    
    m_core->retro_unload_game();
    m_core->retro_deinit();

    m_running = false;
    Log("Libretro thread stopped.");
}

void Wrapper::CreateTexture(Image::Format image_format, PackedByteArray pixel_data, int32_t width, int32_t height, bool flip_y)
{
    m_video_handler->SetImageFormat(image_format);
    m_main_thread_commands_queue.enqueue(std::make_unique<ThreadCommandCreateTexture>(image_format, pixel_data, width, height, flip_y));
}

void Wrapper::UpdateTexture(PackedByteArray pixel_data, bool flip_y)
{
    m_main_thread_commands_queue.enqueue(std::make_unique<ThreadCommandUpdateTexture>(pixel_data, flip_y));
}

bool Wrapper::Shutdown()
{
    Log("Shutting down from core...");
    StopEmulationThread();
    return true;
}

void Wrapper::LedInterfaceSetLedState(int32_t led, int32_t state)
{
    print_line_rich("[color=cyan][LedInterfaceSetLedState][/color] LED " + String::num_int64(led) + " set to " + (state ? "on" : "off"));
}
}
