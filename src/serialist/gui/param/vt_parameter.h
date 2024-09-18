

#ifndef SERIALIST_VT_PARAMETER_H
#define SERIALIST_VT_PARAMETER_H

#include <iostream>
#include <regex>
#include <juce_data_structures/juce_data_structures.h>
#include "core/exceptions.h"
#include "core/utility/traits.h"
#include "core/param/parameter_keys.h"
#include "core/algo/facet.h"
#include "core/collections/voices.h"
#include "gui/param/vt_serialization.h"
#include "deserialization.h"

namespace serialist {


class VTParameterHandler {
public:
    class Listener {
    public:
        Listener() = default;
        virtual ~Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;

        virtual void on_parameter_changed(VTParameterHandler& handler, const std::string& id) = 0;
    };


    // Public ctor, same template as NopParameterHandler
    VTParameterHandler(const Specification& specification, VTParameterHandler& parent)
            : m_value_tree{{specification.type()}}
              , m_undo_manager{parent.get_undo_manager()}
              , m_parent{&parent} {
        for (const auto& [property_name, property_value]: specification.static_properties()) {
            m_value_tree.setProperty({property_name}, {property_value}, &m_undo_manager);
        }

        m_parent->add_child(*this);

    }

    // Public ctor, same template as NopParameterHandler
    [[deprecated]] VTParameterHandler(const std::string& id_property
                                      , VTParameterHandler& parent
                                      , const std::string& value_tree_type)
            : m_value_tree({value_tree_type})
              , m_undo_manager(parent.get_undo_manager())
              , m_parent(&parent) {
        if (!id_property.empty()) {
            m_value_tree.setProperty({ParameterTypes::ID_PROPERTY}, {id_property}, &m_undo_manager);
        }

        m_parent->add_child(*this);
    }


    // create root
    explicit VTParameterHandler(juce::UndoManager& um)
            : m_value_tree({ParameterTypes::ROOT_TREE}), m_undo_manager(um), m_parent(nullptr) {}


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
        return m_value_tree.getProperty({ParameterTypes::ID_PROPERTY}).toString().toStdString();
    }


    /**
     * @throws: RuntimeError if property_value already exists
     */
    // TODO: Implement add_specification for subclasses, e.g. SliderMapping
    template<typename T>
    [[deprecated("Use Specification instead")]]
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

    void set(const T& new_value) {
        // Updates value tree,
        // which internally calls this->valueTreePropertyChanged
        // which updates the internal value and notifies all ParameterHandler::Listeners
        //
        // Note: the value tree is only updated if the new value is different from the current value
        update_value_tree(new_value);
    }

    void add_listener(VTParameterHandler::Listener& listener) {
        m_listeners.append(listener);
    }


    void remove_listener(VTParameterHandler::Listener& listener) {
        m_listeners.remove([&listener](const auto& l) { return &l.get() == &listener; });
    }


    bool equals_property(const juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) {
        return treeWhosePropertyHasChanged == m_parent.get_value_tree() && property == m_identifier;
    }

    void load_state(const VTDeserializationData& dd) {
        set(dd.get_property<T>(m_identifier));
    }

protected:
    virtual void set_internal_value(const T& new_value) = 0;

    void notify() {
        for (const auto& listener: m_listeners) {
            listener.get().on_parameter_changed(m_parent, m_identifier.toString().toStdString());
        }
    }

private:
    /**
     * Called on UndoManager actions only
     */
    void valueTreePropertyChanged(juce::ValueTree& t, const juce::Identifier& property) override {
        if (equals_property(t, property)) {
            set_internal_value(GenericSerializer::deserialize<T>(t.getProperty(property)));
            notify();
        }
    }


    void update_value_tree(const T& new_value) {
        m_parent.get_value_tree().setProperty(m_identifier
                                              , GenericSerializer::serialize(new_value)
                                              , &m_parent.get_undo_manager());
    }


    juce::Identifier m_identifier;
    VTParameterHandler& m_parent;

    Vec<std::reference_wrapper<VTParameterHandler::Listener>> m_listeners;
};


// ==============================================================================================

template<typename T>
class ThreadedVTParameterBase : public VTParameterBase<T> {
public:
    ThreadedVTParameterBase(T initial_value, const std::string& id, VTParameterHandler& parent)
    : VTParameterBase<T>(initial_value, id, parent) {}

    /**
     * @return a copy of the internal value (thread safety)
     */
    T get() {
        return get_internal_value();
    }


protected:
    virtual T get_internal_value() = 0;
};


// ==============================================================================================

template<typename T>
class AtomicVTParameter : public ThreadedVTParameterBase<T> {
public:

    AtomicVTParameter(T initial_value, const std::string& id, VTParameterHandler& parent)
            : ThreadedVTParameterBase<T>(initial_value, id, parent), m_value(initial_value) {
        static_assert(std::atomic<T>::is_always_lock_free, "DataType must be lock-free");
    }


private:
    T get_internal_value() override {
        return m_value;
    }


    void set_internal_value(const T& new_value) override {
        m_value = new_value;
    }


    std::atomic<T> m_value;
};


// ==============================================================================================

template<typename T>
class SingleThreadedVTParameter : public VTParameterBase<T> {
public:
    // TODO: Assert that this is on the correct thread
    SingleThreadedVTParameter(T initial_value
                              , const std::string& id
                              , VTParameterHandler& parent
                              , VTParameterHandler::Listener* default_listener = nullptr)
            : VTParameterBase<T>(initial_value, id, parent)
            , m_value(initial_value) {
        if (default_listener) {
            this->add_listener(*default_listener);
        }
    }

    SingleThreadedVTParameter& operator=(const T& new_value) {
        this->set(new_value);
        return *this;
    }

    const T& operator*() const { return get(); }

    const T* operator->() const { return &get(); }

    /**
     * Note: in a single-threaded context, there's no need to return a copy of the object as we can be sure that it's
     *       never modified elsewhere. We can stil however not return a non-const reference to the object, as we
     *       want to explicitly disallow any modification of the object without informing the internal value tree
     *       of such a change.
     */
    const T& get() const { return m_value; }

private:

    void set_internal_value(const T& new_value) override {
        m_value = new_value;
    }

    T m_value;
};

// ==============================================================================================

template<typename T>
class LockingVTParameter : public ThreadedVTParameterBase<T> {
public:

    LockingVTParameter(T initial_value, const std::string& id, VTParameterHandler& parent)
            : ThreadedVTParameterBase<T>(initial_value, id, parent), m_value(initial_value) {
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

template<typename T>
class ValueTreeVector {
public:
    ValueTreeVector(const Vec<T>& initial
                    , const std::string& id
                    , juce::ValueTree& parent_tree
                    , juce::UndoManager& undo_manager)
            : m_value_tree({id})
              , m_parent_tree(parent_tree)
              , m_undo_manager(undo_manager) {

        m_parent_tree.addChild(m_value_tree, -1, &m_undo_manager);
        reset(initial);
    }


    virtual ~ValueTreeVector() {
        m_parent_tree.removeChild(m_value_tree, &m_undo_manager);
    }


    ValueTreeVector(const ValueTreeVector&) = delete;

    ValueTreeVector& operator=(const ValueTreeVector&) = delete;

    ValueTreeVector(ValueTreeVector&&) noexcept = default;

    ValueTreeVector& operator=(ValueTreeVector&&) noexcept = default;


    void reset(const Vec<T>& new_values = {}) {
        m_next_vt_name = 0;
        for (const auto& v: new_values) {
            auto name = "v" + std::to_string(m_next_vt_name);
            ++m_next_vt_name;
            m_value_tree.setProperty({name}, VTSerializer<T>::to_var(v), &m_undo_manager);
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
                        , const Voices<StoredType>& initial)
            : m_value_tree({id})
              , m_parent(parent) {
        auto& parent_tree = m_parent.get_value_tree();
        if (!parent_tree.isValid())
            throw ParameterError("Cannot register VTSequenceParameter for invalid tree");

        parent_tree.addChild(m_value_tree, -1, &m_parent.get_undo_manager());

        set(initial);

        parent_tree.addListener(this);
    }


    ~VTSequenceParameter() override {
        m_parent.get_value_tree().removeListener(this);
        m_parent.get_value_tree().removeChild(m_value_tree, &m_parent.get_undo_manager());
    }


    VTSequenceParameter(const VTSequenceParameter&) = delete;

    VTSequenceParameter& operator=(const VTSequenceParameter&) = delete;

    VTSequenceParameter(VTSequenceParameter&&) noexcept = default;

    VTSequenceParameter& operator=(VTSequenceParameter&&) noexcept = default;


    void add_parameter_listener(VTParameterHandler::Listener& listener) {
        m_listeners.append(&listener);
    }


    void remove_parameter_listener(VTParameterHandler::Listener& listener) {
        m_listeners.remove(&listener);
    }


    void set(const Voices<StoredType>& v) {
        std::lock_guard lock{m_values_mutex};
        m_voices = v.template as_type<OutputType>();
        reset_value_tree(v);
    }


    void set_transposed(const Voice<StoredType>& voice) {
        auto voices = Voices<StoredType>::transposed(voice);

        std::lock_guard lock{m_value_tree};
        m_voices = voice.template as_type<OutputType>();
        reset_value_tree(voices);
    }


    void set_at(std::size_t index, const Voice<StoredType>& value) {
        // TODO: This function is critical for efficient manipulation of huge value trees
        std::cout << "warning: ValueTree is NOT updated in this function\n";
        std::lock_guard lock{m_values_mutex};
        m_voices[index] = value.template as_type<OutputType>();
    }


    const Voices<OutputType>& get_voices() {
        std::lock_guard lock{m_values_mutex};
        return m_voices;
    }


    Voices<StoredType> get_voices_raw() {
        std::lock_guard lock{m_values_mutex};
        return m_voices.template as_type<StoredType>();
    }


private:
    void reset_value_tree(const Voices<StoredType>& values) {
        m_next_vt_name = 0;
        m_vt_list = Vec<ValueTreeVector<StoredType>>::allocated(values.size());
        for (const auto& v: values) {
            auto name = "c" + std::to_string(m_next_vt_name);
            ++m_next_vt_name;
            m_vt_list.append({v, name, m_value_tree, m_parent.get_undo_manager()});
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

    Voices<OutputType> m_voices = Voices<OutputType>::empty_like();
    juce::ValueTree m_value_tree;

    Vec<ValueTreeVector<StoredType>> m_vt_list;

    unsigned long m_next_vt_name = 0;

    VTParameterHandler& m_parent;

    Vec<VTParameterHandler::Listener*> m_listeners;

};

} // namespace serialist

#endif //SERIALIST_VT_PARAMETER_H
