

#ifndef SERIALISTLOOPER_COMBOBOX_COMPONENT_H
#define SERIALISTLOOPER_COMBOBOX_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "variable.h"
#include "node_component.h"

template<typename T>
class ComboBoxComponent : public NodeComponent
                          , private juce::ValueTree::Listener
                          , private juce::ComboBox::Listener{
public:

    static const int DEFAULT_BOX_HEIGHT_PX = 14;

    struct Entry {
        juce::String display_name;
        T value;
    };

    enum LabelPosition {
        left = 0
        , bottom = 1
    };


    ComboBoxComponent(const std::string& identifier
                      , ParameterHandler& parent
                      , std::vector<Entry> values
                      , T initial
                      , const juce::String& label = ""
                      , LabelPosition label_position = LabelPosition::bottom)
            : m_variable(initial, identifier, parent)
              , m_label({}, label)
              , m_label_position(label_position) {

        for (auto& item: values) {
            add_entry(item);
        }

        m_combo_box.setSelectedId(find_index_by_value(initial), juce::dontSendNotification);
        m_combo_box.addListener(this);
        addAndMakeVisible(m_combo_box);


        if (m_label_position == LabelPosition::bottom)
            m_label.setJustificationType(juce::Justification::centred);
        else
            m_label.setJustificationType(juce::Justification::left);

        addAndMakeVisible(m_label);

        m_variable.get_parameter_obj().add_value_tree_listener(*this);
    }


    ~ComboBoxComponent() override {
        m_variable.get_parameter_obj().remove_value_tree_listener(*this);
    }


    ComboBoxComponent(const ComboBoxComponent&) = delete;
    ComboBoxComponent& operator=(const ComboBoxComponent&) = delete;
    ComboBoxComponent(ComboBoxComponent&&) noexcept = default;
    ComboBoxComponent& operator=(ComboBoxComponent&&) noexcept = default;


    int default_height() {
        if (m_label.getText().isEmpty())
            return DEFAULT_BOX_HEIGHT_PX;

        if (m_label_position == LabelPosition::bottom) {
            return static_cast<int>(DEFAULT_BOX_HEIGHT_PX
                                    + getLookAndFeel().getLabelFont(m_label).getHeight() + 2.0);
        } else {
            return DEFAULT_BOX_HEIGHT_PX;
        }
    }


    void set_label_position(LabelPosition label_position) {
        m_label_position = label_position;
        resized();
    }


    Generative& get_generative() override { return m_variable; }

    void paint(juce::Graphics &) override {}


    void resized() override {
        auto bounds = getLocalBounds();

        if (m_label.getText().isNotEmpty()) {

            if (m_label_position == LabelPosition::bottom) {
                m_label.setBounds(bounds.removeFromBottom(static_cast<int>(getLookAndFeel()
                                                                                   .getLabelFont(m_label)
                                                                                   .getHeight() + 2.0f)));
            } else {
                m_label.setBounds(bounds.removeFromLeft(
                        static_cast<int>(getLookAndFeel()
                                                 .getLabelFont(m_label)
                                                 .getStringWidth(m_label.getText()) + 2)));
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
        std::cout << "combo box changed\n";
    }

    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged
                                  , const juce::Identifier& property) override {
        if (m_variable.get_parameter_obj().equals_property(treeWhosePropertyHasChanged, property)) {
            m_combo_box.setSelectedId(find_index_by_value(m_variable.get_value()), juce::dontSendNotification);
        }
    }


    int find_index_by_value(const T& value) {
        for (const auto& entry: m_item_map) {
            if (entry.second.value == value)
                return entry.first;
        }

        throw std::runtime_error("Could not find value in map");
    }


    Variable<T> m_variable;

    juce::Label m_label;
    LabelPosition m_label_position;

    juce::ComboBox m_combo_box;

    std::map<int, Entry> m_item_map;
    int last_id = 0;
};

#endif //SERIALISTLOOPER_COMBOBOX_COMPONENT_H
