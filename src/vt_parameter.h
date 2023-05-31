

#ifndef SERIALISTPLAYGROUND_VT_PARAMETER_H
#define SERIALISTPLAYGROUND_VT_PARAMETER_H

#include <iostream>
#include <juce_data_structures/juce_data_structures.h>
#include "exceptions.h"
#include "interpolator.h"
#include "serializable.h"


class VTParameterHandler {
public:

    // Public ctor, same template as NopParameterHandler
    VTParameterHandler(const std::string& identifier, VTParameterHandler& parent)
            : m_value_tree({identifier})
              , m_undo_manager(parent.get_undo_manager())
              , m_parent(&parent) {
        m_parent->add_child(*this);
    }


    // create root
    VTParameterHandler(juce::ValueTree& vt, juce::UndoManager& um)
            : m_value_tree(vt), m_undo_manager(um), m_parent(nullptr) {
        if (!m_value_tree.isValid())
            throw ParameterError("VTParameterHandler is not registered");
    }


    ~VTParameterHandler() {
        if (m_parent)
            m_parent->m_value_tree.removeChild(m_value_tree, &m_undo_manager);
    }


    VTParameterHandler(const VTParameterHandler&) = delete;
    VTParameterHandler& operator=(const VTParameterHandler&) = delete;
    VTParameterHandler(VTParameterHandler&&) noexcept = delete;
    VTParameterHandler& operator=(VTParameterHandler&&) noexcept = delete;


    juce::ValueTree& get_value_tree() {
        return m_value_tree;
    }

    juce::Identifier get_identifier() const {
        return m_value_tree.getType();
    }


    juce::UndoManager& get_undo_manager() {
        return m_undo_manager;
    }


private:
    void add_child(VTParameterHandler& child) {
        if (!m_value_tree.isValid())
            throw ParameterError("VTParameterHandler needs to have a valid root");

        m_value_tree.addChild(child.m_value_tree, -1, &m_undo_manager);
    }


    juce::ValueTree m_value_tree;
    juce::UndoManager& m_undo_manager;
    VTParameterHandler* m_parent;
};


// ==============================================================================================

class VTParameterListener {
public:
    VTParameterListener() = default;
    virtual ~VTParameterListener() = default;
    VTParameterListener(const VTParameterListener&) = default;
    VTParameterListener& operator=(const VTParameterListener&) = default;
    VTParameterListener(VTParameterListener&&) noexcept = default;
    VTParameterListener& operator=(VTParameterListener&&) noexcept = default;

    virtual void on_parameter_changed(VTParameterHandler& handler, const std::string& id) = 0;
};


// ==============================================================================================

template<typename T>
class VTParameter : private juce::ValueTree::Listener {
public:
    VTParameter(T initial_value, const std::string& id, VTParameterHandler& parent)
            : m_identifier(id), m_parent(parent) {

        auto& value_tree = m_parent.get_value_tree();
        if (!value_tree.isValid())
            throw ParameterError("Cannot register VTParameter for invalid tree");

        update_value_tree(initial_value);
        value_tree.addListener(this);
    }


    ~VTParameter() override {
        m_parent.get_value_tree().removeListener(this);
        m_parent.get_value_tree().removeProperty(m_identifier, &m_parent.get_undo_manager());
    }


    VTParameter(const VTParameter&) = delete;
    VTParameter& operator=(const VTParameter&) = delete;
    VTParameter(VTParameter&&) noexcept = default;
    VTParameter& operator=(VTParameter&&) noexcept = default;


    void add_parameter_listener(VTParameterListener& listener) {
        m_listeners.push_back(&listener);
    }


    void remove_parameter_listener(VTParameterListener& listener) {
        m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), &listener), m_listeners.end());
    }


    void add_value_tree_listener(juce::ValueTree::Listener& listener) {
        m_parent.get_value_tree().addListener(&listener);
    }


    void remove_value_tree_listener(juce::ValueTree::Listener& listener) {
        m_parent.get_value_tree().removeListener(&listener);
    }


    bool equals_property(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) {
        return treeWhosePropertyHasChanged == m_parent.get_value_tree() && property == m_identifier;
    }


    T get() {
        return get_internal_value();
    }


    void set(const T& new_value) {
        set_internal_value(new_value);
        update_value_tree(new_value);

        for (auto* listener: m_listeners) {
            listener->on_parameter_changed(m_parent, m_identifier.toString().toStdString());
        }
    }


    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged
                                  , const juce::Identifier& property) override {
        if (equals_property(treeWhosePropertyHasChanged, property)) {
            set_internal_value(deserialize(treeWhosePropertyHasChanged.getProperty(property)));
        }
    }


protected:
    virtual T get_internal_value() = 0;
    virtual void set_internal_value(T new_value) = 0;


private:
    void update_value_tree(T new_value) {
        m_parent.get_value_tree().setProperty(m_identifier, serialize(new_value), &m_parent.get_undo_manager());
    }


    template<typename U =T, std::enable_if_t<is_serializable<U>::value, int> = 0>
    juce::var serialize(const T& value) {
        return {value.to_string()};
    }


    template<typename U = T, std::enable_if_t<std::is_enum_v<U>, int> = 0>
    juce::var serialize(const T& value) {
        return static_cast<int>(value);
    }


    template<typename U = T, std::enable_if_t<!is_serializable<U>::value && !std::is_enum_v<U>, int> = 0>
    juce::var serialize(const T& value) {
        return value;
    }


    template<typename U = T, std::enable_if_t<is_serializable<U>::value, int> = 0>
    T deserialize(const juce::var& obj) {
        return T::from_string(obj.toString().toStdString());
    }


    template<typename U = T, std::enable_if_t<std::is_enum_v<U>, int> = 0>
    T deserialize(const juce::var& obj) {
        return T(static_cast<int>(obj));
    }


    template<typename U = T, std::enable_if_t<!is_serializable<U>::value && !std::is_enum_v<U>, int> = 0>
    T deserialize(const juce::var& obj) {
        return obj;
    }


    juce::Identifier m_identifier;
    VTParameterHandler& m_parent;

    std::vector<VTParameterListener*> m_listeners;

};


// ==============================================================================================

template<typename T>
class AtomicVTParameter : public VTParameter<T> {
public:
    AtomicVTParameter(T initial_value, const std::string& id, VTParameterHandler& parent)
            : VTParameter<T>(initial_value, id, parent), m_value(initial_value) {
        static_assert(std::atomic<T>::is_always_lock_free, "DataType must be lock-free");
    }


private:
    T get_internal_value() override {
        return m_value;
    }


    void set_internal_value(T new_value) override {
        m_value = new_value;
    }


    std::atomic<T> m_value;
};

// ==============================================================================================

template<typename T>
class LockingVTParameter : VTParameter<T> {
public:
    LockingVTParameter(T initial_value, const std::string& id, VTParameterHandler& parent)
            : VTParameter<T>(initial_value, id, parent), m_value(initial_value) {
        static_assert(std::is_copy_constructible_v<T>, "DataType must be copyable");
    }


private:
    T get_internal_value() override {
        std::lock_guard<std::mutex> lock{m_mutex};
        return m_value;

    }


    void set_internal_value(T new_value) override {
        std::lock_guard<std::mutex> lock{m_mutex};

        if constexpr (std::is_move_constructible_v<T> && !std::is_trivially_copyable_v<T>) {
            m_value = std::move(new_value);
        } else {
            m_value = new_value;
        }
    }


    std::mutex m_mutex;
    T m_value;

};


// ==============================================================================================

template<typename T>
class CollectionVTParameter {
}; // TODO: Current VTParametrizedSequence should rather be ParametrizedCollection
//       and VTParametrizedSequence should be std::vector<std::vector<DataType>>


// ==============================================================================================

template<typename T>
class VTParametrizedSequence : private juce::ValueTree::Listener {
public:

    VTParametrizedSequence(const std::string& id, VTParameterHandler& parent, const std::vector<T>& initial)
            : m_value_tree({id}), m_parent(parent) {
        auto& parent_tree = m_parent.get_value_tree();
        if (!parent_tree.isValid())
            throw ParameterError("Cannot register VTParameter for invalid tree");

        parent_tree.addChild(m_value_tree, -1, &m_parent.get_undo_manager());

        for (auto& v: initial) {
            insert(v, -1);
        }

        parent_tree.addListener(this);
    }


    ~VTParametrizedSequence() {
        m_parent.get_value_tree().removeListener(this);
        m_parent.get_value_tree().removeChild(m_value_tree, &m_parent.get_undo_manager());
    }


    VTParametrizedSequence(const VTParametrizedSequence&) = delete;
    VTParametrizedSequence& operator=(const VTParametrizedSequence&) = delete;
    VTParametrizedSequence(VTParametrizedSequence&&) noexcept = default;
    VTParametrizedSequence& operator=(VTParametrizedSequence&&) noexcept = default;


    void add_parameter_listener(VTParameterListener& listener) {
        m_listeners.push_back(&listener);
    }


    void remove_parameter_listener(VTParameterListener& listener) {
        m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), &listener), m_listeners.end());
    }


    void add_value_tree_listener(juce::ValueTree::Listener& listener) {
        m_parent.get_value_tree().addListener(&listener);
    }


    void remove_value_tree_listener(juce::ValueTree::Listener& listener) {
        m_parent.get_value_tree().removeListener(&listener);
    }


    T at(int index) {
        std::lock_guard<std::mutex> lock{m_values_mutex};

        index = adjust_index_range(index, false);
        return m_values.at(static_cast<std::size_t>(index));
    }


    std::vector<T> clone_values() {
        std::lock_guard<std::mutex> lock{m_values_mutex};

        return std::vector<T>(m_values);
    }


    void reset_values(std::vector<T> new_values) {
        std::lock_guard<std::mutex> lock{m_values_mutex};
        m_value_tree.removeAllChildren(&m_parent.get_undo_manager());
        for (auto& value: new_values) {
            internal_insert(value, -1);
        }
    }


    std::vector<T> interpolate(double position, const InterpolationStrategy<T>& strategy) {
        std::lock_guard<std::mutex> lock{m_values_mutex};
        return Interpolator<T>::interpolate(position, strategy, m_values);
    }


    void insert(T value, int index) {
        std::lock_guard<std::mutex> lock{m_values_mutex};
        internal_insert(value, index);

    }


    void move(int index_from, int index_to) {
        std::lock_guard<std::mutex> lock{m_values_mutex};

        index_from = adjust_index_range(index_from, false);
        index_to = adjust_index_range(index_to, true);

        std::rotate(m_values.begin() + index_from, m_values.begin() + index_from + 1, m_values.begin() + index_to);

        m_value_tree.moveChild(index_from, index_to, &m_parent.get_undo_manager());
    }


    void remove(int index) {
        std::lock_guard<std::mutex> lock{m_values_mutex};

        index = adjust_index_range(index, false);

        m_values.erase(m_values.begin() + index);

        m_value_tree.removeChild(index, &m_parent.get_undo_manager());
    }


    const std::size_t& size() {
        std::lock_guard<std::mutex> lock{m_values_mutex};
        return m_values.size();
    }


    bool empty() {
        std::lock_guard<std::mutex> lock{m_values_mutex};
        return m_values.empty();
    }


private:

    int adjust_index_range(int index, bool for_insertion) {

        // negative indices: insert/access from back
        if (index < 0)
            index += static_cast<int>(m_values.size()) + static_cast<int>(for_insertion);

        return std::clamp(index, 0, static_cast<int>(m_values.size()) - static_cast<int>(!for_insertion));
    }


    void internal_insert(const T& value, int index) {
        index = adjust_index_range(index, true);

        m_values.insert(m_values.begin() + index, value);

        juce::ValueTree child({"c" + std::to_string(used_vt_names++)});
        child.setProperty("v1", value, nullptr); // TODO: Generalize for trees with multiple properties
        m_value_tree.addChild(child, index, &m_parent.get_undo_manager());
    }


    std::mutex m_values_mutex;
    std::vector<T> m_values;

    long used_vt_names = 0;

    juce::ValueTree m_value_tree;
    VTParameterHandler& m_parent;

    std::vector<VTParameterListener*> m_listeners;

};

#endif //SERIALISTPLAYGROUND_VT_PARAMETER_H
