//
//#ifndef SERIALISTLOOPER_AUTO_PULSATOR_H
//#define SERIALISTLOOPER_AUTO_PULSATOR_H
//
//#include "core/collections/scheduler.h"
//#include "core/algo/temporal/pulse.h"
//#include "core/algo/temporal/trigger.h"
//#include "core/generatives/stereotypes/base_stereotypes.h"
//#include "core/generatives/stereotypes/pulsator_stereotypes.h"
//#include "core/algo/temporal/time_point.h"
//
//
//
//// ==============================================================================================
//
//class AutoPulsator : public Pulsator {
//public:
//    explicit AutoPulsator(std::unique_ptr<TimePointGenerator> ts = std::make_unique<FreePeriodicTimePoint>()
//                          , double legato_amount = 1.0
//                          , bool sample_and_hold = true)
//            : m_ts(std::move(ts))
//              , m_legato_amount(legato_amount)
//              , m_sample_and_hold(sample_and_hold) {}
//
//    ~AutoPulsator() override = default;
//
//
//    AutoPulsator(const AutoPulsator& other)
//            : m_pulses(other.m_pulses)
//              , m_ts(other.m_ts->clone())
//              , m_legato_amount(other.m_legato_amount)
//              , m_sample_and_hold(other.m_sample_and_hold)
//              , m_configuration_changed(other.m_configuration_changed)
//              , m_next_trigger_time(other.m_next_trigger_time)
//              , m_last_callback_time(other.m_last_callback_time)
//              , m_running(other.m_running) {}
//
//    AutoPulsator& operator=(const AutoPulsator& other) {
//        if (this == &other) return *this;
//        m_ts = other.m_ts->clone();
//        m_legato_amount = other.m_legato_amount;
//        m_sample_and_hold = other.m_sample_and_hold;
//        m_running = other.m_running;
//        m_last_callback_time = other.m_last_callback_time;
//        m_configuration_changed = other.m_configuration_changed;
//        m_next_trigger_time = other.m_next_trigger_time;
//        m_pulses = other.m_pulses;
//        return *this;
//    }
//
//    AutoPulsator(AutoPulsator&&) noexcept = default;
//
//    AutoPulsator& operator=(AutoPulsator&&) noexcept = default;
//
//
//    void start(const TimePoint& time, const std::optional<DomainTimePoint>&) override {
//        if (m_running) {
//            return;
//        }
//
//        m_next_trigger_time = m_ts->next(time, true);
//        m_last_callback_time = time;
//        m_running = true;
//        m_configuration_changed = false;
//    }
//
//    Voice<Trigger> stop() override {
//        m_running = false;
//        return flush();
//    }
//
//    Voice<Trigger> flush() override {
//        return m_pulses.flush();
//    }
//
//    bool is_running() const override {
//        return m_running;
//    }
//
//    Voice<Trigger> poll(const TimePoint& time) override {
//        if (!is_running())
//            return {};
//
//        if (m_configuration_changed && !m_sample_and_hold) {
//            reschedule();
//        }
//        m_configuration_changed = false;
//
//        auto output = m_pulses.drain_elapsed_as_triggers(time, true);
//
//        if (auto next_pulse_on = handle_next(time)) {
//            output.append(*next_pulse_on);
//        }
//
//        m_last_callback_time = time;
//        return output;
//    }
//
//    Voice<Trigger> handle_external_triggers(const TimePoint& t, const Voice<Trigger>&) override {
//        // Not relevant for AutoPulsator
//        (void) t;
//        return {};
//    }
//
//    Voice<Trigger> handle_time_skip(const TimePoint& new_time) override {
//        // TODO: Not sure if this strategy is ideal for all instances of ts.
//        //       For non-pulse-based ts'es, we may want to compute the elapsed time
//        //       (i.e. up until m_last_callback_time and reschedule based on remanining time)
//        auto flushed = flush();
//
//        // TODO: Handle exception or explain why this is safe!
//        m_next_trigger_time = m_ts->next(new_time, false);
//        m_last_callback_time = new_time;
//        return flushed;
//    }
//
//    Vec<Pulse> export_pulses() override {
//        return m_pulses.vec_mut().drain();
//    }
//
//    void import_pulses(const Vec<Pulse>& pulses) override {
//        // adding pulses without defined pulse_off is ok here since we're considering those as elapsed in process()
//        m_pulses.vec_mut().extend(pulses);
//    }
//
//    void set_ts(std::unique_ptr<TimePointGenerator> ts) {
//        m_ts = std::move(ts);
//        m_configuration_changed = true;
//    }
//
//    void set_legato_amount(double legato_amount) {
//        m_legato_amount = utils::clip(legato_amount, {0.0});
//        m_configuration_changed = true;
//    }
//
//    void set_sample_and_hold(bool sample_and_hold) {
//        m_sample_and_hold = sample_and_hold;
//    }
//
//    const TimePointGenerator& get_ts() const { return *m_ts; }
//
//    double get_legato_amount() const { return m_legato_amount; }
//
//    std::optional<DomainTimePoint> next_scheduled_pulse_on() override {
//        return m_next_trigger_time;
//    }
//
//
//private:
//    void reschedule() {
//        // TODO: Don't forget to handle the case when the time format has changed (e.g. from Beats to Bars)
//        throw std::runtime_error("AutoPulsator::reschedule not implemented");
//    }
//
//    std::optional<Trigger> handle_next(const TimePoint& current_time) {
//        if (!m_next_trigger_time.elapsed(current_time)) {
//            return {};
//        }
//
//
//        // Note: `last_callback_time` rather than `current_time` to ensure a point in time before m_next_trigger_time
//        DomainTimePoint trigger_time = m_ts->supports(m_next_trigger_time)
//                                       ? m_next_trigger_time
//                                       : m_next_trigger_time.as_type(m_ts->get_type(), m_last_callback_time);
//
//        // TODO: Handle exception or explain why this is safe!
//        m_next_trigger_time = m_ts->next(trigger_time, current_time);
//
//        auto period = (m_next_trigger_time - trigger_time) * m_legato_amount;
//        auto trigger = m_pulses.new_pulse(trigger_time, trigger_time + period);
//
//        return trigger;
//    }
//
//
//    Pulses m_pulses;
//
//    std::unique_ptr<TimePointGenerator> m_ts;
//    double m_legato_amount;
//    bool m_sample_and_hold;
//
//    bool m_configuration_changed = false;
//
//    DomainTimePoint m_next_trigger_time = DomainTimePoint::zero();
//    TimePoint m_last_callback_time = TimePoint::zero();
//
//    bool m_running = false;
//};
//
//
//// ==============================================================================================
//
//class AutoPulsatorNode : public PulsatorNodeBase<AutoPulsator> {
//public:
//
//    static const inline std::string DURATION = "period";
//    static const inline std::string DURATION_TYPE = "duration_type";
//    static const inline std::string OFFSET = "offset_type";
//    static const inline std::string OFFSET_TYPE = "offset_type";
//    static const inline std::string OFFSET_ENABLED = "offset_enabled";
//    static const inline std::string LEGATO = "legato";
//
//    static const inline std::string CLASS_NAME = "pulsator";
//
//    AutoPulsatorNode(const std::string& id
//                     , ParameterHandler& parent
//                     , Node<Facet>* period = nullptr
//                     , Node<Facet>* duration_type = nullptr
//                     , Node<Facet>* offset = nullptr
//                     , Node<Facet>* offset_type = nullptr
//                     , Node<Facet>* offset_enabled = nullptr
//                     , Node<Facet>* legato_amount = nullptr
//                     , Node<Facet>* enabled = nullptr
//                     , Node<Facet>* num_voices = nullptr)
//            : PulsatorNodeBase<AutoPulsator>(id, parent, enabled, num_voices, CLASS_NAME)
////              , m_trigger(add_socket(ParameterKeys::TRIGGER, trigger))
//              , m_duration(add_socket(DURATION, period))
//              , m_duration_type(add_socket(DURATION_TYPE, duration_type))
//              , m_offset(add_socket(OFFSET, offset))
//              , m_offset_type(add_socket(OFFSET_TYPE, offset_type))
//              , m_offset_enabled(add_socket(OFFSET_ENABLED, offset_enabled))
//              , m_legato_amount(add_socket(LEGATO, legato_amount)) {
//    }
//
//    std::size_t get_voice_count() override {
//        return voice_count(
////                m_trigger.voice_count()
//                m_duration.voice_count()
//                , m_offset.voice_count()
//                , m_legato_amount.voice_count());
//
//    }
//
//    void update_parameters(std::size_t num_voices, bool size_has_changed) override {
//        if (size_has_changed ||
//            m_duration.has_changed() || m_duration_type.has_changed()
//            || m_offset.has_changed() || m_offset_type.has_changed()) {
//
//            auto period = m_duration.process().adapted_to(num_voices).firsts_or(1.0);
//            auto duration_type = m_duration_type.process().first_or(DomainType::ticks);
//
//            auto offset = m_offset.process().adapted_to(num_voices).firsts_or(0.0);
//            auto offset_type = m_offset_type.process().first_or(DomainType::ticks);
//            auto offset_enabled = m_offset_enabled.process().first_or(false);
//
//
//            auto tses = temporal::ts_from_durations_offsets(period, duration_type, offset, offset_type, offset_enabled);
//            pulsators().set(&AutoPulsator::set_ts, std::move(tses));
//        }
//
//        if (size_has_changed || m_legato_amount.has_changed()) {
//            auto legato = m_legato_amount.process().adapted_to(num_voices).firsts_or(1.0);
//            pulsators().set(&AutoPulsator::set_legato_amount, legato.as_type<double>());
//        }
//    }
//
//    Voices<Trigger> get_incoming_triggers(const TimePoint&, std::size_t) override {
//        return {};
//    }
//
//private:
////    Socket<Trigger>& m_trigger;
//    Socket<Facet>& m_duration;
//    Socket<Facet>& m_duration_type;
//    Socket<Facet>& m_offset;
//    Socket<Facet>& m_offset_type;
//    Socket<Facet>& m_offset_enabled;
//    Socket<Facet>& m_legato_amount;
//};
//
//#endif //SERIALISTLOOPER_AUTO_PULSATOR_H
