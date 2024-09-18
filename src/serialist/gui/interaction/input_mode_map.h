
#ifndef SERIALISTLOOPER_INPUT_MODE_MAP_H
#define SERIALISTLOOPER_INPUT_MODE_MAP_H

#include <memory>
#include "input_mode.h"
#include "input_condition.h"

namespace serialist {

class InputModeMap {
public:
    void add(std::unique_ptr<InputCondition> condition, std::unique_ptr<InputMode> mode) {
        if (auto index = m_modes.index([&mode](auto& m) {
            return m.get() == mode.get();
        })) {
            m_condition_map.insert(std::make_pair<std::unique_ptr<InputCondition>, std::size_t>(
                    std::move(condition), std::move(*index)));
        } else {
            m_modes.append(std::move(mode));
            m_condition_map.insert(std::make_pair(std::move(condition), m_modes.size() - 1));
        }
    }

    void add(Vec<std::unique_ptr<InputCondition>> conditions, std::unique_ptr<InputMode> mode) {
        for (auto& condition: conditions) {
            add(std::move(condition), std::move(mode));
        }
    }


    InputMode* get_active() {
        for (const auto& [condition, index]: m_condition_map) {
            if (condition->is_met()) {
                return m_modes[index].get();
            }
        }
        return nullptr;
    }


private:


    Vec<std::unique_ptr<InputMode>> m_modes;

    std::unordered_map<std::unique_ptr<InputCondition>, std::size_t> m_condition_map;


};


} // namespace serialist

#endif //SERIALISTLOOPER_INPUT_MODE_MAP_H
