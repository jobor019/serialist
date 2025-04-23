#ifndef SERIALIST_ROUTER_H
#define SERIALIST_ROUTER_H

#include "core/param/socket_handler.h"
#include "core/generative.h"
#include "core/types/trigger.h"
#include "core/types/facet.h"
#include "core/temporal/time_gate.h"
#include "core/collections/multi_voiced.h"
#include "sequence.h"
#include "variable.h"
#include "types/index.h"

#include "types/phase.h"


namespace serialist {

// Namespace to avoid templating all arguments
namespace router {
enum class Mode { route, through, mix, split };

struct Defaults {
    static constexpr auto MODE = Mode::route;
    static constexpr auto USE_INDEX = true;
    static constexpr auto BROADCAST = false;
};

}


template<typename T>
class Router {
public:
    using MultiVoices = Vec<Voices<T>>;
    using OutletSpec = Vec<std::optional<Index>>;

    // template<typename U = T, bool Enabled = false>
    // struct HeldPulses {};
    //
    // template<typename U = T, std::is_same_v<U, Trigger>>
    // struct HeldPulses {
    //     Vec<MultiVoiceHeld<PulseIdentifier>> held_pulses;
    // };

    Router(std::size_t num_inlets, std::size_t num_outlets)
        : m_num_inlets(num_inlets)
        , m_num_outlets(num_outlets) {
        assert(m_num_inlets > 0);
        assert(m_num_outlets > 0);
    }


    MultiVoices process(MultiVoices&& input, const Voices<Facet>& spec, router::Mode mode, bool is_index) {
        if (spec.is_empty_like()) {
            return default_empty();
        }

        // Single only supports modes route and through, hence a separate implementation
        if (m_num_inlets == 1 && m_num_outlets == 1) {
            return process_single(std::move(input), spec, mode, is_index);
        }

        if (mode == router::Mode::through) {
            return multi_through(std::move(input), spec, mode, is_index);
        } else if (mode == router::Mode::mix) {
            return mix(std::move(input), spec, mode, is_index);
        } else if (mode == router::Mode::split) {
            return split(std::move(input), spec, mode, is_index);
        }

        return multi_route(std::move(input), spec, mode, is_index);
    }

private:
    MultiVoices process_single(MultiVoices&& input, const Voices<Facet>& spec, router::Mode mode, bool is_index) {
        if (mode == router::Mode::through) {
            throw std::runtime_error("mode: through not implemented");

        } else {
            // All other modes: default to `route`. `mix` and `split` are not relevant for within-voices routing

            auto voices = input[0];
            auto input_size = voices.size();
            auto output_size = spec.size();

            // map spec to indices corresponding to elements in input voices
            auto indices = to_outlet_spec(spec, input_size, is_index);

            auto output = Vec<Voice<T>>::allocated(output_size);
            for (const auto& i : indices) {
                if (!i) {
                    output.append(Voice<T>{});
                } else {
                    if (auto clipped_index = i->get_pass(input_size)) {
                        output.append(voices[static_cast<std::size_t>(*clipped_index)]);
                    } else {
                        output.append(Voice<T>{});
                    }
                }
            }
            return {Voices<T>{output}};
        }
    }


    MultiVoices multi_route(MultiVoices&& input, const Voices<Facet>& spec, router::Mode mode, bool is_index) {
        auto input_size = input.size();
        auto output_size = spec.size();

        auto indices = to_outlet_spec(spec, input_size, is_index);

        auto output = MultiVoices::allocated(output_size);
        for (const auto& i : indices) {
            if (!i) {
                output.append(Voices<T>::empty_like());
            } else {
                if (auto clipped_index = i->get_pass(input_size)) {
                    output.append(input[static_cast<std::size_t>(*clipped_index)]);
                } else {
                    output.append(Voices<T>::empty_like());
                }
            }
        }
        return output;

    }


    MultiVoices multi_through(MultiVoices&& input, const Voices<Facet>& spec, router::Mode mode, bool is_index) {
        throw std::runtime_error("mode: through not implemented");
    }


    MultiVoices mix(MultiVoices&& input, const Voices<Facet>& spec, router::Mode mode, bool is_index) {
        throw std::runtime_error("mode: mix not implemented");
    }


    MultiVoices split(MultiVoices&& input, const Voices<Facet>& spec, router::Mode mode, bool is_index) {
        throw std::runtime_error("mode: split not implemented");
    }

public:
    MultiVoices default_empty() const {
        return MultiVoices::repeated(m_num_outlets, Voices<T>::empty_like());
    }


    // TODO: Isn't it a bit inconvenient to have different signatures like this? It'll be inconsistent
    //       with our `flush` implementation, which we can't really expect to have anything but MultiVoices output.
    // std::optional<MultiVoices> flush();


    static OutletSpec to_outlet_spec(const Vec<bool>& enabled_mask);


    static OutletSpec to_outlet_spec(const Voices<Facet>& spec, std::size_t num_indices, bool is_indices) {
        auto firsts = spec.firsts<>();

        auto type = is_indices ? Index::Type::index : Index::Type::phase;

        auto indices = OutletSpec::allocated(firsts.size());
        for (auto& f : firsts) {
            if (f) {
                indices.append(Index::from(*f, type, num_indices));
            } else {
                indices.append(std::nullopt);
            }
        }

        return indices;
    }


    std::size_t num_inlets() const { return m_num_inlets; }
    std::size_t num_outlets() const { return m_num_outlets; }

private:
    const std::size_t m_num_inlets;
    const std::size_t m_num_outlets;


    OutletSpec m_previous_outlet_spec;

    // HeldPulses<> m_held_pulses;
};


// ==============================================================================================


// Note: If this ever is used in another class, it should be moved to socket_base.h
template<typename T>
class MultiSocket {
public:
    // explicit MultiSocket(std::size_t num_voices) : m_sockets(num_voices) {
    //     assert(num_voices > 0);
    // }


    // explicit MultiSocket(Vec<std::unique_ptr<Socket<T>>> sockets) : m_sockets(std::move(sockets)) {
    //     assert(!m_sockets.empty());
    // }


    MultiSocket(Vec<Node<T>*> nodes, SocketHandler& socket_handler, const std::string& base_name)
        : m_sockets{create_sockets(std::move(nodes), socket_handler, base_name)} {
        assert(!m_sockets.empty());
    }


    static Vec<std::reference_wrapper<Socket<T>>> create_sockets(Vec<Node<T>*> nodes
                                                                 , SocketHandler& socket_handler
                                                                 , const std::string& base_name) {
        auto sockets = Vec<std::reference_wrapper<Socket<T>>>::allocated(nodes.size());
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            sockets.append(std::ref(socket_handler.create_socket(base_name + std::to_string(i), nodes[i])));
        }

        return sockets;
    }


    Vec<Voices<T>> process() {
        auto result = Vec<Voices<T>>::allocated(m_sockets.size());
        for (const auto& socket : m_sockets) {
            result.append(socket.get().process());
        }
        return result;
    }


    bool any_is_connected() const {
        for (const auto& socket : m_sockets) {
            if (socket.get().is_connected()) {
                return true;
            }
        }
        return false;
    }


    std::size_t size() const { return m_sockets.size(); }

private:
    Vec<std::reference_wrapper<Socket<T>>> m_sockets;

};


// ==============================================================================================

template<typename T>
class RouterNode : public MultiNode<T> {
public:
    using MultiVoices = typename Router<T>::MultiVoices;
    using OutletSpec = typename Router<T>::OutletSpec;

    static constexpr bool is_trigger = std::is_same_v<T, Trigger>;


    struct Keys {
        static const inline std::string INPUT = "input";
        static const inline std::string ROUTING_MAP = "routing_map";
        static const inline std::string MODE = "mode";
        static const inline std::string USES_INDEX = "uses_index";
        // static const inline std::string BROADCAST = "broadcast";

        static const inline std::string CLASS_NAME = "router";
    };


    RouterNode(const std::string& id
               , ParameterHandler& parent
               , std::size_t num_outlets
               , Vec<Node<T>*> inputs
               , Node<Facet>* routing_map = nullptr
               , Node<Facet>* mode = nullptr
               , Node<Facet>* uses_index = nullptr
               // , Node<Facet>* broadcast = nullptr
               , Node<Facet>* enabled = nullptr
               // , Node<Facet>* num_voices = nullptr
    )
        : m_parameter_handler(Specification(param::types::generative)
                              .with_identifier(id)
                              .with_static_property(param::properties::template_class, Keys::CLASS_NAME)
                              , parent)
        , m_socket_handler(m_parameter_handler)
        , m_router(inputs.size(), num_outlets)
        , m_inputs(std::move(inputs), m_socket_handler, Keys::INPUT)
        , m_routing_map(m_socket_handler.create_socket<Facet>(Keys::ROUTING_MAP, routing_map))
        , m_mode(m_socket_handler.create_socket<Facet>(Keys::MODE, mode))
        , m_uses_index(m_socket_handler.create_socket<Facet>(Keys::USES_INDEX, uses_index))
        // , m_broadcast(m_socket_handler.create_socket<Facet>(Keys::BROADCAST, broadcast))
        , m_enabled(m_socket_handler.create_socket<Facet>(param::properties::enabled, enabled))
    // , m_num_voices(m_socket_handler.create_socket<Facet>(param::properties::num_voices, num_voices))
    {}


    void update_time(const TimePoint& t) override { m_time_gate.push_time(t); }


    Vec<Voices<T>> process() override {
        if (auto t = m_time_gate.pop_time(); !t) {
            return m_current_value;
        }

        if (auto flushed = process_enabled_state()) {
            m_current_value = *flushed;
            return m_current_value;
        }

        auto mode = m_mode.process().first_or(router::Defaults::MODE);
        auto uses_index = m_uses_index.process().first_or(router::Defaults::USE_INDEX);

        auto input = m_inputs.process();
        auto routing_map = m_routing_map.process();

        assert(input.size() == m_router.num_inlets());

        m_current_value = m_router.process(std::move(input), routing_map, mode, uses_index);
        return m_current_value;
    }


    std::vector<Generative*> get_connected() override { return m_socket_handler.get_connected(); }
    ParameterHandler& get_parameter_handler() override { return m_parameter_handler; }
    void disconnect_if(Generative& connected_to) override { m_socket_handler.disconnect_if(connected_to); }

private:
    std::optional<MultiVoices> process_enabled_state() {
        if constexpr (is_trigger) {
            // TODO: Use EnabledGate to handle this correctly
            throw std::runtime_error("RouterNode::process_enabled_state: not implemented");
        } else {
            if (!is_enabled()) {
                return m_router.default_empty();
            }
            return std::nullopt;
        }
    }


    bool is_enabled() {
        return m_enabled.process().first_or(true) && m_inputs.any_is_connected() && m_routing_map.is_connected();
    }


    ParameterHandler m_parameter_handler;
    SocketHandler m_socket_handler;

    Router<T> m_router;

    TimeGate m_time_gate;

    MultiSocket<T> m_inputs;         // Note: this is not registered in the SocketHandler!
    Socket<Facet>& m_routing_map;    // Sequence

    Socket<Facet>& m_mode;          // Variable
    Socket<Facet>& m_uses_index;    // Variable
    // Socket<Facet>& m_broadcast;     // Variable
    // Socket<Facet>& m_num_voices;    // Sequence

    Socket<Facet>& m_enabled;

    MultiVoices m_current_value;
};


// ==============================================================================================

template<typename T, typename FloatType = double>
struct RouterWrapper {
    using Keys = typename RouterNode<T>::Keys;
    using Defaults = router::Defaults;

    using SequenceType = std::conditional_t<
        std::is_same_v<T, Facet>,
        Sequence<Facet, double>,
        Sequence<Trigger>
    >;


    RouterWrapper(std::size_t num_inlets, std::size_t num_outlets)
        : inputs(create_inputs(num_inlets, ph))
        , router_node(Keys::CLASS_NAME
                      , ph
                      , num_outlets
                      , cast_inputs(inputs)
                      , &routing_map
                      , &mode
                      , &uses_index
                      , &enabled) {
        static_assert(std::is_same_v<T, Trigger> || std::is_same_v<T, Facet>);
    }


    static Vec<std::unique_ptr<SequenceType>> create_inputs(std::size_t num_inlets, ParameterHandler& ph) {
        auto v = Vec<std::unique_ptr<SequenceType>>::allocated(num_inlets);
        for (std::size_t i = 0; i < num_inlets; ++i) {
            v.append(std::make_unique<SequenceType>(Keys::INPUT + std::to_string(i), ph));
        }

        return v;
    }


    template<typename U = FloatType>
    void set_input(std::size_t index, const Voices<U>& value) {
        static_assert(std::is_same_v<U, FloatType> || std::is_same_v<U, Trigger>);
        assert(index < inputs.size());

        inputs[index]->set_values(value);
    }


    ParameterHandler ph;

    Vec<std::unique_ptr<SequenceType>> inputs;
    Sequence<Facet, FloatType> routing_map{Keys::ROUTING_MAP, ph};
    Variable<Facet, router::Mode> mode{Keys::MODE, ph, Defaults::MODE};
    Variable<Facet, bool> uses_index{Keys::USES_INDEX, ph, Defaults::USE_INDEX};

    Variable<Facet, bool> enabled{param::properties::enabled, ph, true};

    RouterNode<T> router_node;

private:
    static Vec<Node<T>*> cast_inputs(const Vec<std::unique_ptr<SequenceType>>& uptrs) {
        auto raw_ptrs = Vec<Node<T>*>::allocated(uptrs.size());
        for (const auto& uptr : uptrs) {
            raw_ptrs.append(uptr.get());
        }
        return raw_ptrs;
    }

};


using RouterFacetWrapper = RouterWrapper<Facet>;
using RouterTriggerWrapper = RouterWrapper<Trigger>;

}

#endif //SERIALIST_ROUTER_H
