#ifndef TESTUTILS_EVENTS_H
#define TESTUTILS_EVENTS_H
#include "condition.h"
#include "results.h"
#include "algo/temporal/time_point.h"
#include "generatives/variable.h"

using namespace serialist;


namespace serialist::test {
template<typename T>
class NodeRunnerEvent {
public:
    NodeRunnerEvent() = default;
    virtual ~NodeRunnerEvent() = default;
    NodeRunnerEvent(const NodeRunnerEvent&) = delete;
    NodeRunnerEvent& operator=(const NodeRunnerEvent&) = delete;
    NodeRunnerEvent(NodeRunnerEvent&&)  noexcept = default;
    NodeRunnerEvent& operator=(NodeRunnerEvent&&)  noexcept = default;

    /**
     * @return true if the NodeRunnerEvent is completed (and should be removed), otherwise false
     */
    virtual bool process(std::size_t step_index
                         , const TimePoint& t
                         , const TimePoint& t_prev
                         , const Vec<StepResult<T> >& v) = 0;
};


// ==============================================================================================

template<typename NodeRunnerType, typename VariableOutputType, typename VariableInternalType = VariableOutputType>
class VariableChangeEvent : public NodeRunnerEvent<NodeRunnerType> {
public:
    VariableChangeEvent(NodeRunnerCondition<NodeRunnerType>&& condition
                        , Variable<VariableOutputType, VariableInternalType>& node
                        , const VariableInternalType& new_value)
    : m_condition(std::move(condition)), m_node(node), m_new_value(new_value) {}


    bool process(std::size_t step_index, const TimePoint& t, const TimePoint& t_prev
        , const Vec<StepResult<NodeRunnerType>>& v) override {
        if (m_condition(step_index, t, t_prev, v)) {
            m_node.set_value(m_new_value);
            return true;
        }
        return false;
    }

private:
    NodeRunnerCondition<NodeRunnerType> m_condition;
    Variable<VariableOutputType, VariableInternalType>& m_node;
    const VariableInternalType m_new_value;


};

}

#endif //TESTUTILS_EVENTS_H
