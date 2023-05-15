

#ifndef SERIALISTLOOPER_MIDI_CONFIG_H
#define SERIALISTLOOPER_MIDI_CONFIG_H

#include <string>
#include <mutex>

class MidiConfig {
public:
    static MidiConfig& get_instance() {
        static MidiConfig instance;
        return instance;
    }


    MidiConfig(const MidiConfig&) = delete;
    MidiConfig& operator=(const MidiConfig&) = delete;
    MidiConfig(MidiConfig&&) noexcept = delete;
    MidiConfig& operator=(MidiConfig&&) noexcept = delete;


    const std::string& get_default_device_name() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_default_device_name;
    }


    void set_default_device_name(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_default_device_name = name;
    }


private:
    MidiConfig() = default;

    std::mutex m_mutex;

    std::string m_default_device_name = "IAC Driver IAC1";
};


#endif //SERIALISTLOOPER_MIDI_CONFIG_H
