

#ifndef SERIALISTPLAYGROUND_VT_PARAMETER_H
#define SERIALISTPLAYGROUND_VT_PARAMETER_H

#include <iostream>
#include <regex>
#include <juce_data_structures/juce_data_structures.h>
#include "exceptions.h"
#include "interpolator.h"
#include "serializable.h"
#include "parameter_keys.h"
#include "facet.h"
#include "voice.h"

class VTSerialization {
public:
    VTSerialization() = delete;

    template<typename T>
    static juce::var serialize(const T& value)  {
        static_assert(!std::is_same_v<T, Facet>, "Facet type is not serializable.");

        if constexpr (is_serializable<T>::value) {
            return {value.to_string()};
        } else if constexpr (std::is_enum_v<T>) {
            return static_cast<int>(value);
        } else {
            return value;
        }
    }


    template<typename T>
    static T deserialize(const juce::var& obj) {
        static_assert(!std::is_same_v<T, Facet>, "Facet type is not serializable.");

        if constexpr (is_serializable<T>::value) {
            return T::from_string(obj.toString().toStdString());
        } else if constexpr (std::is_enum_v<T>) {
            return T(static_cast<int>(obj));
        } else {
            return obj;
        }
    }
};



class VTParameterHandler {
public:

    // Public ctor, same template as NopParameterHandler
    VTParameterHandler(const std::string& id_property
                       , VTParameterHandler& parent
                       , const std::string& value_tree_id = ParameterKeys::GENERATIVE)
            : m_value_tree({value_tree_id})
              , m_undo_manager(parent.get_undo_manager())
              , m_parent(&parent) {
        if (!id_property.empty()) {
            m_value_tree.setProperty({ParameterKeys::ID_PROPERTY}, {id_property}, &m_undo_manager);
        }

        m_parent->add_child(*this);
    }


    // create root
    explicit VTParameterHandler(juce::UndoManager& um)
            : m_value_tree({ParameterKeys::ROOT_TREE}), m_undo_manager(um), m_parent(nullptr) {}


    ~VTParameterHandler() {
        if (m_parent)
            m_parent->m_value_tree.removeChild(m_value_tree, &m_undo_manager);
    }


    VTParameterHandler(const VTParameterHandler&) = delete;
    VTParameterHandler& operator=(const VTParameterHandler&) = delete;
    VTParameterHandler(VTParameterHandler&&) noexcept = default;
    VTParameterHandler& operator=(VTParameterHandler&&) noexcept = delete;


    [[nodiscard]] juce::ValueTree& get_value_tree() {
        return m_value_tree;
    }


    [[nodiscard]] std::string get_id() const {
        return m_value_tree.getProperty({ParameterKeys::ID_PROPERTY}).toString().toStdString();
    }


    /**
     * @throws: RuntimeError if property_value already exists
     */
    template<typename T>
    void add_static_property(const std::string& property_name, T property_value) {
        if (m_value_tree.hasProperty({property_name}))
            throw std::runtime_error("A property_value with the name '" + property_value + "' already exists");

        m_value_tree.setProperty({property_name}, {property_value}, &m_undo_manager);
    }


    /**
     * matches, e.g. `base_name` "osc" matches identifiers "osc", "osc::freq", "osc::freq::value" but not "osc1"
     */
    [[nodiscard]] bool identifier_matches(const std::string& base_name) const {
        return identifier_matches(std::regex("^" + base_name + "(:{2}.*)?$"));
    }


    [[nodiscard]] bool identifier_matches(const std::regex& base_name_regex) const {
        return std::regex_match(get_id(), base_name_regex);
    }


    /**
     * equals, e.g. `exact_name` "osc" only state_equals the exact identifier "osc"
     */
    [[nodiscard]] bool identifier_equals(const std::string& exact_name) const {
        return get_id() == exact_name;
    }


    /**
     * begins with, e.g. `name_root` "osc" is true for identifiers "osc", "osc1"
     *  but false for "osc::freq" and "osc1::freq"
     */
    [[nodiscard]] bool identifier_begins_with(const std::string& name_root) const {
        return identifier_matches(std::regex("^" + name_root + "[^:]*$"));
    }


    juce::UndoManager& get_undo_manager() {
        return m_undo_manager;
    }


private:
    void add_child(VTParameterHandler const& child) {
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
class VTParameterBase : private juce::ValueTree::Listener {
public:
    VTParameterBase(T initial_value, const std::string& id, VTParameterHandler& parent)
            : m_identifier(id), m_parent(parent) {

        auto& value_tree = m_parent.get_value_tree();
        if (!value_tree.isValid())
            throw ParameterError("Cannot register VTParameterBase for invalid tree");

        update_value_tree(initial_value);
        value_tree.addListener(this);
    }


    ~VTParameterBase() override {
        m_parent.get_value_tree().removeListener(this);
        m_parent.get_value_tree().removeProperty(m_identifier, &m_parent.get_undo_manager());
    }


    VTParameterBase(const VTParameterBase&) = delete;
    VTParameterBase& operator=(const VTParameterBase&) = delete;
    VTParameterBase(VTParameterBase&&) noexcept = default;
    VTParameterBase& operator=(VTParameterBase&&) noexcept = default;


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


    bool equals_property(const juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) {
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


protected:
    virtual T get_internal_value() = 0;
    virtual void set_internal_value(T new_value) = 0;


private:
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged
                                  , const juce::Identifier& property) override {
        if (equals_property(treeWhosePropertyHasChanged, property)) {
            set_internal_value(VTSerialization::deserialize<T>(treeWhosePropertyHasChanged.getProperty(property)));
        }
    }


    void update_value_tree(T new_value) {
        m_parent.get_value_tree().setProperty(m_identifier
                                              , VTSerialization::serialize(new_value)
                                              , &m_parent.get_undo_manager());
    }


    juce::Identifier m_identifier;
    VTParameterHandler& m_parent;

    std::vector<VTParameterListener*> m_listeners;

};


// ==============================================================================================

template<typename T>
class AtomicVTParameter : public VTParameterBase<T> {
public:

    AtomicVTParameter(T initial_value, const std::string& id, VTParameterHandler& parent)
            : VTParameterBase<T>(initial_value, id, parent), m_value(initial_value) {
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
class LockingVTParameter : public VTParameterBase<T> {
public:

    LockingVTParameter(T initial_value, const std::string& id, VTParameterHandler& parent)
            : VTParameterBase<T>(initial_value, id, parent), m_value(initial_value) {
        static_assert(std::is_copy_constructible_v<T>, "DataType must be copyable");
    }


private:
    T get_internal_value() override {
        std::lock_guard lock{m_mutex};
        return m_value;

    }


    void set_internal_value(T new_value) override {
        std::lock_guard lock{m_mutex};

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

//template<typename T>
//class VTSequenceParameter : private juce::ValueTree::Listener {
//public:
//    VTSequenceParameter(const std::vector<T>& initial, const std::string& id, VTParameterHandler& parent)
//    :
//};


template<typename T>
class ValueTreeList {
public:
    ValueTreeList(const std::vector<T>& initial
                  , const std::string& id
                  , juce::ValueTree& parent_tree
                  , juce::UndoManager& undo_manager)
            : m_value_tree({id})
              , m_parent_tree(parent_tree)
              , m_undo_manager(undo_manager) {

        m_parent_tree.addChild(m_value_tree, -1, &m_undo_manager);
        reset(initial);
    }


    virtual ~ValueTreeList() {
        m_parent_tree.removeChild(m_value_tree, &m_undo_manager);
    }


    ValueTreeList(const ValueTreeList&) = delete;
    ValueTreeList& operator=(const ValueTreeList&) = delete;
    ValueTreeList(ValueTreeList&&) noexcept = default;
    ValueTreeList& operator=(ValueTreeList&&) noexcept = default;


    void reset(const std::vector<T>& new_values = {}) {
        m_next_vt_name = 0;
        for (const auto& v: new_values) {
            auto name = "v" + std::to_string(m_next_vt_name);
            ++m_next_vt_name;
            m_value_tree.setProperty({name}, VTSerialization::serialize<T>(v), &m_undo_manager);
        }
    }


private:
    juce::ValueTree m_value_tree;
    juce::ValueTree& m_parent_tree;
    juce::UndoManager& m_undo_manager;

    unsigned long m_next_vt_name = 0;
};


// ==============================================================================================

template<typename OutputType, typename StoredType = OutputType>
class VTSequenceParameter : private juce::ValueTree::Listener {
public:

    VTSequenceParameter(const std::string& id
                        , VTParameterHandler& parent
                        , const std::vector<std::vector<StoredType>>& initial)
            : m_value_tree({id})
              , m_parent(parent) {
        auto& parent_tree = m_parent.get_value_tree();
        if (!parent_tree.isValid())
            throw ParameterError("Cannot register VTSequenceParameter for invalid tree");

        parent_tree.addChild(m_value_tree, -1, &m_parent.get_undo_manager());

        set(initial);

        parent_tree.addListener(this);
    }


//    VTSequenceParameter(const std::string& id
//                        , VTParameterHandler& parent
//                        , const Voices<OutputType>& initial)
//            : VTSequenceParameter(id, parent, initial.vectors_as()) {}


    ~VTSequenceParameter() override {
        m_parent.get_value_tree().removeListener(this);
        m_parent.get_value_tree().removeChild(m_value_tree, &m_parent.get_undo_manager());
    }


    VTSequenceParameter(const VTSequenceParameter&) = delete;
    VTSequenceParameter& operator=(const VTSequenceParameter&) = delete;
    VTSequenceParameter(VTSequenceParameter&&) noexcept = default;
    VTSequenceParameter& operator=(VTSequenceParameter&&) noexcept = default;


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


//    void set(const Voices<OutputType>& v) {
//        std::lock_guard lock{m_values_mutex};
//        m_voices = v;
//        reset_value_tree(v.vectors_as());
//    }


    void set(const std::vector<std::vector<StoredType>>& v) {
        std::lock_guard lock{m_values_mutex};
        m_voices = Voices<OutputType>(v);
        reset_value_tree(v);
    }

    void set_transposed(const std::vector<StoredType>& v) {
        std::lock_guard lock{m_value_tree};
        auto v_transposed = VoiceUtils::transpose(v);
        m_voices = Voices<OutputType>(v_transposed);
        reset_value_tree(v_transposed);
    }


    const Voices<OutputType>& get_voices() {
        std::lock_guard lock{m_values_mutex};
        return m_voices;
    }


    const std::vector<std::vector<StoredType>>& get_values() {
        std::lock_guard lock{m_values_mutex};
    }


//    void reset_values(Voice<T> new_values = {}) {
//        std::lock_guard lock{m_values_mutex};
//        m_value_tree.removeAllChildren(&m_parent.get_undo_manager());
//        std::cout << "warning: missing CLEAR of vector / REMOVE\n";
//        for (auto& value: new_values) {
//            internal_insert(value, -1);
//        }
//    }
//
//
//    std::vector<T> interpolate(double position, const InterpolationStrategy& strategy) {
//        std::lock_guard lock{m_values_mutex};
//        return Interpolator<T>::interpolate(position, strategy, m_voices);
//    }
//
//
//    void insert(T value, int index) {
//        std::lock_guard lock{m_values_mutex};
//        internal_insert(value, index);
//
//    }
//
//
//    void move(int index_from, int index_to) {
//        std::lock_guard lock{m_values_mutex};
//
//        index_from = adjust_index_range(index_from, false);
//        index_to = adjust_index_range(index_to, true);
//
//        std::rotate(m_voices.begin() + index_from, m_voices.begin() + index_from + 1, m_voices.begin() + index_to);
//
//        m_value_tree.moveChild(index_from, index_to, &m_parent.get_undo_manager());
//    }
//
//
//    void remove(int index) {
//        std::lock_guard lock{m_values_mutex};
//
//        index = adjust_index_range(index, false);
//
//        m_voices.erase(m_voices.begin() + index);
//
//        m_value_tree.removeChild(index, &m_parent.get_undo_manager());
//    }
//
//
//    const std::size_t& size() {
//        std::lock_guard lock{m_values_mutex};
//        return m_voices.size();
//    }
//
//
//    bool empty() {
//        std::lock_guard lock{m_values_mutex};
//        return m_voices.empty();
//    }


private:
    void reset_value_tree(const std::vector<std::vector<StoredType>>& values) {
        m_vt_list.clear();
        m_next_vt_name = 0;

        m_vt_list.reserve(values.size());
        for (const auto& v: values) {
            auto name = "c" + std::to_string(m_next_vt_name);
            ++m_next_vt_name;
            m_vt_list.emplace_back(v, name, m_value_tree, m_parent.get_undo_manager());
        }
    }


//    int adjust_index_range(int index, bool for_insertion) {
//
//        // negative indices: insert/access from back
//        if (index < 0)
//            index += static_cast<int>(m_voices.size()) + static_cast<int>(for_insertion);
//
//        return std::clamp(index, 0, static_cast<int>(m_voices.size()) - static_cast<int>(!for_insertion));
//    }
//
//
//    void internal_insert(const T& value, int index) {
//        index = adjust_index_range(index, true);
//
//        m_voices.insert(m_voices.begin() + index, value);
//
//        juce::ValueTree child({"c" + std::to_string(used_vt_names++)});
//        child.setProperty("v1", value, nullptr); // TODO: Generalize for trees with multiple properties
//        m_value_tree.addChild(child, index, &m_parent.get_undo_manager());
//    }


    std::mutex m_values_mutex;

    Voices<OutputType> m_voices = Voices<OutputType>::create_empty_like();
    juce::ValueTree m_value_tree;

    std::vector<ValueTreeList<StoredType>> m_vt_list;

    unsigned long m_next_vt_name = 0;

    VTParameterHandler& m_parent;

    std::vector<VTParameterListener*> m_listeners;

};

#endif //SERIALISTPLAYGROUND_VT_PARAMETER_H
