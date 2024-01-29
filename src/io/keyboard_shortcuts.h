//
// Created by Joakim Borg on 2023-06-21.
//

#ifndef SERIALISTLOOPER_KEYBOARD_SHORTCUTS_H
#define SERIALISTLOOPER_KEYBOARD_SHORTCUTS_H

struct GlobalKeyboardShortcuts {

};



// ==============================================================================================

// TODO LEGACY LEGACY LEGACY LEGACY LEGACY LEGACY LEGACY LEGACY LEGACY
// TODO
// TODO: Not sure if this file should be used at all given how InputHandler
// TODO  solves most of the problems that this file was intended to solve
// TODO
// TODO LEGACY LEGACY LEGACY LEGACY LEGACY LEGACY LEGACY LEGACY LEGACY

struct ConfigurationLayerKeyboardShortcuts {
    static const int NEW_GENERATOR_KEY = static_cast<int>('Q');
    static const int NEW_OSCILLATOR_KEY = static_cast<int>('W');
    static const int NEW_MIDI_SOURCE_KEY = static_cast<int>('E');
    static const int NEW_PULSATOR_KEY = static_cast<int>('A');

    static const int CONNECTOR_KEY = static_cast<int>('R');
    static const int DISCONNECT_KEY = static_cast<int>('T');
    static const int DELETE_KEY = static_cast<int>('X');
    static const int MOVE_KEY = static_cast<int>('M');
    static const int MODULATION_KEY = static_cast<int>('Y');
};


// ==============================================================================================

struct MultiSliderKeyboardShortcuts {
    static const int DELETE_SLIDER = static_cast<int>('D');
};




#endif //SERIALISTLOOPER_KEYBOARD_SHORTCUTS_H
