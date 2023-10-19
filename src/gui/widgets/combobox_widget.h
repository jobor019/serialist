

#ifndef SERIALISTLOOPER_COMBOBOX_WIDGET_H
#define SERIALISTLOOPER_COMBOBOX_WIDGET_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "core/generatives/variable.h"
#include "generative_component.h"

template<typename StoredType>
class ComboBoxWidget : public GenerativeComponent
                       , private juce::ValueTree::Listener
                       , private juce::ComboBox::Listener {
public:

    struct Entry {
        juce::String display_name;
        StoredType value;
    };

    enum class Layout : int {
        label_left = 0
        , label_below = 1
    };


    ComboBoxWidget(Variable<Facet, StoredType>& variable
                   , std::vector<Entry>&& values
                   , const juce::String& label = ""
                   , const Layout layout = Layout::label_left
                   , int label_width = DimensionConstants::DEFAULT_LABEL_WIDTH)
            : m_variable(variable)
              , m_label({}, label)
              , m_layout(layout)
              , m_label_width(label_width) {
        static_assert(std::is_enum_v<StoredType> || std::is_integral_v<StoredType>
                      , "ComboBoxWidget requires an integral value");

        setComponentID(variable.get_parameter_handler().get_id());

        initialize_combo_box(values);
        initialize_label();

        m_variable.get_parameter_obj().add_value_tree_listener(*this);
    }


    ~ComboBoxWidget() override {
        m_variable.get_parameter_obj().remove_value_tree_listener(*this);
    }


    ComboBoxWidget(const ComboBoxWidget&) = delete;
    ComboBoxWidget& operator=(const ComboBoxWidget&) = delete;
    ComboBoxWidget(ComboBoxWidget&&) noexcept = delete;
    ComboBoxWidget& operator=(ComboBoxWidget&&) noexcept = delete;


    static int height_of(Layout layout) {
        if (layout == Layout::label_below) {
            return DimensionConstants::SLIDER_DEFAULT_HEIGHT
                   + DimensionConstants::FONT_HEIGHT
                   + DimensionConstants::LABEL_BELOW_MARGINS;
        } else {
            return DimensionConstants::SLIDER_DEFAULT_HEIGHT;
        }
    }


    Generative& get_generative() override { return m_variable; }


    void set_layout(int layout_id) override {
        m_layout = static_cast<Layout>(layout_id);
        resized();
    }


    void paint(juce::Graphics&) override {}


    void resized() override {
        auto bounds = getLocalBounds();

        if (m_label.getText().isNotEmpty()) {

            if (m_layout == Layout::label_below) {
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

    void initialize_combo_box(const std::vector<Entry>& values) {
        for (auto& item: values) {
            add_entry(item);
        }

        m_combo_box.addListener(this);
        m_combo_box.setSelectedId(id_from_value(m_variable.get_value()));
        addAndMakeVisible(m_combo_box);
    }


    void initialize_label() {
        if (m_layout == Layout::label_left)
            m_label.setJustificationType(juce::Justification::left);
        else
            m_label.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(m_label);
    }


    void comboBoxChanged(juce::ComboBox*) override {
        m_variable.set_value(value_from_id(m_combo_box.getSelectedId()));
    }


    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged
                                  , const juce::Identifier& property) override {
        if (m_variable.get_parameter_obj().equals_property(treeWhosePropertyHasChanged, property)) {
            std::cout << "Variable ValueTree: " << m_variable.get_parameter_handler().get_value_tree().toXmlString()
                      << "\n";
            std::cout << "Changed ValueTree: " << treeWhosePropertyHasChanged.toXmlString() << "\n";

            m_combo_box.setSelectedId(id_from_value(m_variable.get_value()), juce::dontSendNotification);
        }
    }


    int id_from_value(const StoredType& value) const {
        for (const auto& entry: m_item_map) {
            if (entry.second.value == value)
                return entry.first;
        }

        throw std::runtime_error("Could not find value in map");
    }


    const StoredType& value_from_id(int id) const {
        return m_item_map.at(id).value;
    }


    Variable<Facet, StoredType>& m_variable;

    juce::Label m_label;
    Layout m_layout;
    int m_label_width;

    juce::ComboBox m_combo_box;

    std::map<int, Entry> m_item_map;
    int last_id = 0;
};

#endif //SERIALISTLOOPER_COMBOBOX_WIDGET_H
