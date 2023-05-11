

#ifndef SERIALISTPLAYGROUND_VT_PARAMETER_H
#define SERIALISTPLAYGROUND_VT_PARAMETER_H

#include <iostream>
#include <juce_data_structures/juce_data_structures.h>
#include "exceptions.h"

class VTParameterHandler {
public:

    // Public ctor, same template as NopParameterHandler
    explicit VTParameterHandler(const std::string& identifier, VTParameterHandler& parent)
            : m_identifier(identifier)
              , m_value_tree(m_identifier)
              , m_undo_manager(parent.get_undo_manager())
              , m_parent(&parent) {
        m_parent->add_child(*this);
    }


    ~VTParameterHandler() {
        if (m_parent)
            m_parent->m_value_tree.removeChild(m_value_tree, &m_undo_manager);
    }


    static VTParameterHandler create_root(juce::Identifier id, juce::ValueTree& vt, juce::UndoManager& um) {
        return {std::move(id), vt, um, nullptr};
    }


    juce::ValueTree& get_value_tree() {
        if (!m_value_tree.isValid())
            throw ParameterError("VTParameterHandler is not registered");

        return m_value_tree;
    }


    juce::UndoManager& get_undo_manager() {
        return m_undo_manager;
    }


private:
    VTParameterHandler(juce::Identifier id, juce::ValueTree& vt, juce::UndoManager& um, VTParameterHandler* parent)
            : m_identifier(std::move(id)), m_value_tree(vt), m_undo_manager(um), m_parent(parent) {}


    void add_child(VTParameterHandler& child) {
        if (!m_value_tree.isValid())
            throw ParameterError("VTParameterHandler needs to have a valid root");

        m_value_tree.addChild(child.m_value_tree, -1, &m_undo_manager);
    }


    juce::Identifier m_identifier;
    juce::ValueTree m_value_tree;
    juce::UndoManager& m_undo_manager;
    VTParameterHandler* m_parent;
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
        m_parent.get_value_tree().removeProperty(m_identifier, &m_parent.get_undo_manager());
    }


    VTParameter(const VTParameter&) = delete;
    VTParameter& operator=(const VTParameter&) = delete;
    VTParameter(VTParameter&&) noexcept = default;
    VTParameter& operator=(VTParameter&&) noexcept = default;


    virtual T get() const = 0;
    virtual void set(const T& new_value) = 0;

    void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged
                                  , const juce::Identifier &property) override {
        if (treeWhosePropertyHasChanged == m_parent.get_value_tree() && property == m_identifier) {
            set_internal_value(treeWhosePropertyHasChanged.getProperty(property));
        }
    }


protected:
    virtual T get_internal_value() = 0;
    virtual void set_internal_value(T new_value) = 0;


    void update_value_tree(T new_value) {
        m_parent.get_value_tree().setProperty(m_identifier, new_value, &m_parent.get_undo_manager());
    }


private:
    juce::Identifier m_identifier;
    VTParameterHandler& m_parent;

};


// ==============================================================================================

template<typename T>
class AtomicVTParameter : public VTParameter<T> {
public:
    AtomicVTParameter(T initial_value, const std::string& id, VTParameterHandler& parent)
            : VTParameter<T>(initial_value, id, parent), m_value(initial_value) {
        static_assert(std::atomic<T>::is_always_lock_free, "T must be lock-free");
    }


    T get() const override {
        return m_value;
    }


    void set(const T& new_value) override {
        m_value = new_value;                            // atomic
        VTParameter<T>::update_value_tree(new_value);   // not atomic
    }


private:
    T get_internal_value() override {
        return get();
    }


    void set_internal_value(T new_value) override {
        m_value = new_value;
    }


    std::atomic<T> m_value;
};

// ==============================================================================================

template<typename T>
class LockingVTParameter {
};// TODO

#endif //SERIALISTPLAYGROUND_VT_PARAMETER_H
