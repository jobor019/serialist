#ifndef TESTUTILS_EVENTS_H
#define TESTUTILS_EVENTS_H
#include "condition.h"
#include "results.h"
#include "algo/temporal/time_point.h"
#include "generatives/variable.h"
#include "generatives/sequence.h"

using namespace serialist;


namespace serialist::test {
template<typename T>
class NodeRunnerEvent {
public:
    NodeRunnerEvent() = default;
    virtual ~NodeRunnerEvent() = default;
    NodeRunnerEvent(const NodeRunnerEvent&) = delete;
    NodeRunnerEvent& operator=(const NodeRunnerEvent&) = delete;
    NodeRunnerEvent(NodeRunnerEvent&&) noexcept = default;
    NodeRunnerEvent& operator=(NodeRunnerEvent&&) noexcept = default;

    /**
     * @return true if the NodeRunnerEvent is completed (and should be removed), otherwise false
     */
    virtual bool process(std::size_t step_index
                         , const TimePoint& t
                         , const TimePoint& t_next
                         , const Vec<StepResult<T> >& v) = 0;


    /**
     * @return true if the event should be processed **after** output has been generated for the current step
     */
    virtual bool is_post_condition() const = 0;


    /**
     * If the event needs to clean up some state upon being deleted
     * (e.g. an active Pulse that has set its node to pulse::on and should be set back to pulse::off before deletion)
     */
    virtual void on_clear() {}

protected:
    static bool is_post_condition_helper(const RunnerCondition<T>& condition) {
        return condition.depends_on_step_output();
    }
};


// ==============================================================================================

template<typename NodeRunnerType, typename VariableOutputType, typename VariableInternalType = VariableOutputType>
class VariableChangeEvent : public NodeRunnerEvent<NodeRunnerType> {
public:
    VariableChangeEvent(Variable<VariableOutputType, VariableInternalType>& node
                        , const VariableInternalType& new_value
                        , RunnerCondition<NodeRunnerType>&& condition)
        : m_node(node), m_new_value(new_value), m_condition(std::move(condition)) {}


    bool process(std::size_t step_index, const TimePoint& t, const TimePoint& t_next
                 , const Vec<StepResult<NodeRunnerType> >& v) override {
        if (m_condition.matches(step_index, t, t_next, v)) {
            m_node.set_value(m_new_value);
            return true;
        }
        return false;
    }


    bool is_post_condition() const override {
        return NodeRunnerEvent<NodeRunnerType>::is_post_condition_helper(m_condition);
    }

private:
    Variable<VariableOutputType, VariableInternalType>& m_node;
    const VariableInternalType m_new_value;

    RunnerCondition<NodeRunnerType> m_condition;
};


// ==============================================================================================

template<typename NodeRunnerType, typename OutputType, typename StoredType = OutputType>
class SequenceChangeEvent : public NodeRunnerEvent<NodeRunnerType> {
public:
    template<typename VoicesLike>
    SequenceChangeEvent(Sequence<OutputType, StoredType>& node
                        , const VoicesLike& new_value
                        , RunnerCondition<NodeRunnerType>&& condition)
        : m_node(node)
          , m_new_value(Voices<StoredType>::from_voices_like(new_value))
          , m_condition(std::move(condition)) {
        static_assert(Voices<StoredType>::template is_voices_like_v<VoicesLike>
                      , "Value should be either a StoredType, Vec<StoredType> or Voices<StoredType>");
    }


    bool process(std::size_t step_index, const TimePoint& t, const TimePoint& t_next
                 , const Vec<StepResult<NodeRunnerType> >& v) override {
        if (m_condition.matches(step_index, t, t_next, v)) {
            m_node.set_values(m_new_value);
            return true;
        }
        return false;
    }


    bool is_post_condition() const override {
        return NodeRunnerEvent<NodeRunnerType>::is_post_condition_helper(m_condition);
    }

private:
    Sequence<OutputType, StoredType>& m_node;
    const Voices<StoredType> m_new_value;

    RunnerCondition<NodeRunnerType> m_condition;
};


// ==============================================================================================

/** Meter and tempo events: see node_runner.h */

}

#endif //TESTUTILS_EVENTS_H
