
#ifndef SERIALISTLOOPER_OSC_H
#define SERIALISTLOOPER_OSC_H

#include "juce_osc/juce_osc.h"
#include "core/generative.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/algo/temporal/trigger.h"

namespace serialist {

class OscParser {
public:
    OscParser() = delete;
};


// ==============================================================================================

class OscFormatter {
public:
    OscFormatter() = delete;


    template<typename T, typename = std::enable_if_t<utils::is_osc_convertible_v<T>>>
    static juce::OSCMessage voices2message(const std::string& address, const Voices<T>& voices, bool flatten = false) {
        if (flatten) {
            return voices2message_flattened(address, voices);
        } else {
            return voices2message_non_flattened(address, voices);
        }
    }


private:
    template<typename T, typename = std::enable_if_t<utils::is_osc_convertible_v<T>>>
    static juce::OSCMessage voices2message_flattened(const std::string& address, const Voices<T>& voices) {
        juce::OSCMessage msg({address});
        for (const auto& voice: voices) {
            for (const auto& elem: voice) {
                add_argument(msg, elem);
            }
        }
        return msg;
    }


    template<typename T, typename = std::enable_if_t<utils::is_osc_convertible_v<T>>>
    static juce::OSCMessage voices2message_non_flattened(const std::string& address, const Voices<T>& voices) {
        juce::OSCMessage msg({address});
        for (const auto& voice: voices) {
            msg.addString("[");
            for (const auto& elem: voice) {
                add_argument(msg, elem);
            }
            msg.addString("]");
        }
        return msg;
    }


    template<typename T, typename = std::enable_if_t<utils::is_osc_convertible_v<T>>>
    static void add_argument(juce::OSCMessage& msg, const T& arg) {
        if constexpr (std::is_floating_point_v<T>) {
            msg.addFloat32(static_cast<float>(arg));
        } else if constexpr (std::is_integral_v<T>) {
            msg.addInt32(static_cast<juce::int32>(arg));
        } else {
            msg.addString(static_cast<std::string>(arg));
        }
    }


};

// ==============================================================================================

class OscSender {
public:
    /**
     * @throws IOError if connection fails
     */
    explicit OscSender(const std::string& ip, int port) : m_ip(ip), m_port(port) {
        if (!m_sender.connect(ip, port))
            throw IOError("Could not connect to " + ip + ":" + std::to_string(port));
    }


    bool send(const juce::OSCMessage& msg) {
        return m_sender.send(msg);
    }


    template<typename T>
    bool send(const std::string& address, const Voices<T>& message, bool flatten) {
        return send(OscFormatter::voices2message(address, message, flatten));
    }


    const std::string& get_ip() const { return m_ip; }


    int get_port() const { return m_port; }


private:

    juce::OSCSender m_sender;
    std::string m_ip;
    int m_port;
};

} // namespace serialist

#endif //SERIALISTLOOPER_OSC_H
