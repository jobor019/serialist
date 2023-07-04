
#ifndef SERIALISTLOOPER_BASE_MODULE_H
#define SERIALISTLOOPER_BASE_MODULE_H

#include "generative_component.h"
#include "editable.h"
#include "keyboard_shortcuts.h"

class BaseModule : public GenerativeComponent
        , public Editable {
public:
    enum DefaultEditStates {
        move_module = 1
        , remove_module = 2
    };

    class RemoveHighlight : public EditHighlight {
    public:
        void paint(juce::Graphics& g) override {
            g.setColour(juce::Colours::salmon.withAlpha(0.5f));
            g.fillRect(getLocalBounds());
        }
    };

    class MoveHighlight : public EditHighlight {
    public:
        void paint(juce::Graphics& g) override {
            g.setColour(juce::Colours::springgreen);
            g.drawRect(getLocalBounds(), 3);
        }
    };

    explicit BaseModule() : m_highlight_manager(*this, this, default_module_highlights()) {
        addAndMakeVisible(m_highlight_manager);
    }


    static std::map<int, std::unique_ptr<EditHighlight>> default_module_highlights() {
        std::map<int, std::unique_ptr<EditHighlight>> highlights;
        highlights.emplace(DefaultEditStates::move_module, std::make_unique<MoveHighlight>());
        highlights.emplace(DefaultEditStates::remove_module, std::make_unique<RemoveHighlight>());
        return highlights;
    }

    void resized() override {
        m_highlight_manager.toFront(false);
        m_highlight_manager.setBounds(getLocalBounds());
    }



protected:

    int compute_edit_state() override {
        if (GlobalKeyState::is_down_exclusive(ConfigurationLayerKeyboardShortcuts::MOVE_KEY)) {
            return static_cast<int>(DefaultEditStates::move_module);
        } else if (GlobalKeyState::is_down_exclusive(ConfigurationLayerKeyboardShortcuts::DELETE_KEY)) {
            return static_cast<int>(DefaultEditStates::remove_module);
        } else {
            return static_cast<int>(Editable::DefaultEditStates::disabled);
        }
    }


private:

    EditHighlightManager m_highlight_manager;
};

#endif //SERIALISTLOOPER_BASE_MODULE_H
