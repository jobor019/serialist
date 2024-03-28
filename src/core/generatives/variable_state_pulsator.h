
#ifndef SERIALISTLOOPER_THREE_STATE_PULSATOR_H
#define SERIALISTLOOPER_THREE_STATE_PULSATOR_H

#include "core/generatives/stereotypes/pulsator_stereotypes.h"
#include "auto_pulsator.h"
#include "triggered_pulsator.h"

class VariableStatePulsator : public Pulsator {
    enum class Mode {
        auto_pulsator
        , triggered_pulsator
        , trigger_thru
    };

    explicit VariableStatePulsator(Mode mode = Mode::auto_pulsator
                                   , double duration = 1.0
                                   , double legato_amount = 1.0
                                   , bool sample_and_hold = true)
            : m_mode(mode)
              , m_auto_pulsator(duration, legato_amount, sample_and_hold)
              , m_triggered_pulsator(duration * legato_amount) {
    }


  void set_mode(Mode mode) {
        // TODO: We shouldn't update this until process is called! 
        if (mode != m_mode) {



            Voice<Trigger> flushed;

            if (m_active_pulsator) {
                flushed = m_active_pulsator->flush();
            }

            set_active_pulsator(mode);

            if (m_active_pulsator) {
                m_active_pulsator.
            }


            m_mode = mode;
        }

  }

private:
    void set_active_pulsator(Mode mode) {
        switch (m_mode) {
                case Mode::auto_pulsator:
                    m_active_pulsator = &m_auto_pulsator;
                    break;
                case Mode::triggered_pulsator:
                    m_active_pulsator = &m_triggered_pulsator;
                    break;
                case Mode::trigger_thru:
                    m_active_pulsator = nullptr;
            }
    }


    Mode m_mode;

    AutoPulsator m_auto_pulsator;
    TriggeredPulsator m_triggered_pulsator;

    Pulsator* m_active_pulsator;

    Vec<Trigger> m_flushed;



};


//class VariableStatePulsatorNode : public PulsatorBase {
//private:
//    MultiVoiced
//
//};

#endif //SERIALISTLOOPER_THREE_STATE_PULSATOR_H
