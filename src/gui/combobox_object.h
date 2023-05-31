

#ifndef SERIALISTLOOPER_COMBOBOX_OBJECT_H
#define SERIALISTLOOPER_COMBOBOX_OBJECT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "variable.h"
#include "node_component.h"

template<typename T>
class ComboBoxObject : public GenerativeComponent
                       , private juce::ValueTree::Listener
                       , private juce::ComboBox::Listener {
public:

    struct Entry {
        juce::String display_name;
        T value;
    };

    enum LabelPosition {
        left = 0
        , bottom = 1
    };


    ComboBoxObject(const std::string& identifier
                   , ParameterHandler& parent
                   , std::vector<Entry> values
                   , T initial
                   , const juce::String& label = ""
                   , int label_width = DimensionConstants::DEFAULT_LABEL_WIDTH)
            : m_variable(initial, identifier, parent)
              , m_label({}, label)
              , m_label_width(label_width) {

        for (auto& item: values) {
            add_entry(item);
        }

        m_combo_box.setSelectedId(id_from_value(initial), juce::dontSendNotification);
        m_combo_box.addListener(this);
        addAndMakeVisible(m_combo_box);


        if (m_label_position == LabelPosition::bottom)
            m_label.setJustificationType(juce::Justification::centred);
        else
            m_label.setJustificationType(juce::Justification::left);

        addAndMakeVisible(m_label);

        m_variable.get_parameter_obj().add_value_tree_listener(*this);
    }


    ~ComboBoxObject() override {
        m_variable.get_parameter_obj().remove_value_tree_listener(*this);
    }


    ComboBoxObject(const ComboBoxObject&) = delete;
    ComboBoxObject& operator=(const ComboBoxObject&) = delete;
    ComboBoxObject(ComboBoxObject&&) noexcept = default;
    ComboBoxObject& operator=(ComboBoxObject&&) noexcept = default;


    std::pair<int, int> dimensions() override {
        return {default_width(), default_height()};
    }

    int default_width() {
        if (m_label.getText().isEmpty())
            return DimensionConstants::SLIDER_DEFAULT_WIDTH;

        if (m_label_position == LabelPosition::bottom) {
            return DimensionConstants::SLIDER_DEFAULT_WIDTH;
        } else {
            return DimensionConstants::SLIDER_DEFAULT_WIDTH + m_label_width;
        }
    }


    int default_height() const {
        if (m_label.getText().isEmpty())
            return DimensionConstants::SLIDER_DEFAULT_HEIGHT;

        if (m_label_position == LabelPosition::bottom) {
            return DimensionConstants::SLIDER_DEFAULT_HEIGHT
                   + DimensionConstants::FONT_HEIGHT
                   + DimensionConstants::LABEL_BELOW_MARGINS;
        } else {
            return DimensionConstants::SLIDER_DEFAULT_HEIGHT;
        }
    }



    void set_label_position(LabelPosition label_position) {
        m_label_position = label_position;
        resized();
    }


    Generative& get_generative() override { return m_variable; }


    void paint(juce::Graphics&) override {}


    void resized() override {
        auto bounds = getLocalBounds();

        if (m_label.getText().isNotEmpty()) {

            if (m_label_position == LabelPosition::bottom) {
                m_label.setJustificationType(juce::Justification::centred);
                m_label.setBounds(bounds.removeFromBottom(DimensionConstants::FONT_HEIGHT));
                bounds.removeFromBottom(DimensionConstants::LABEL_BELOW_MARGINS);

            } else {
                m_label.setJustificationType(juce::Justification::left);
                m_label.setBounds(bounds.removeFromLeft(m_label_width));
            }
        }
        m_combo_box.setBounds(bounds);
    }


    void add_entry(const Entry& entry) {
        int id = ++last_id;
        m_item_map.insert(std::pair<int, Entry>(id, entry));
        m_combo_box.addItem(entry.display_name, id);
    }


private:

    void comboBoxChanged(juce::ComboBox*) override {
        m_variable.set_value(value_from_id(m_combo_box.getSelectedId()));
    }


    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged
                                  , const juce::Identifier& property) override {
        if (m_variable.get_parameter_obj().equals_property(treeWhosePropertyHasChanged, property)) {
            m_combo_box.setSelectedId(id_from_value(m_variable.get_value()), juce::dontSendNotification);
        }
    }


    int id_from_value(const T& value) const {
        for (const auto& entry: m_item_map) {
            if (entry.second.value == value)
                return entry.first;
        }

        throw std::runtime_error("Could not find value in map");
    }


    const T& value_from_id(int id) const {
        return m_item_map.at(id).value;
    }


    Variable<T> m_variable;

    juce::Label m_label;
    LabelPosition m_label_position = LabelPosition::bottom;
    int m_label_width;

    juce::ComboBox m_combo_box;

    std::map<int, Entry> m_item_map;
    int last_id = 0;
};

#endif //SERIALISTLOOPER_COMBOBOX_OBJECT_H
