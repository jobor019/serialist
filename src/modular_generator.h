

#ifndef SERIALISTLOOPER_MODULAR_GENERATOR_H
#define SERIALISTLOOPER_MODULAR_GENERATOR_H

#include <mutex>
#include "source.h"

class ModularGenerator : public ParameterHandler {
public:
    explicit ModularGenerator(ParameterHandler&& handler)
            : ParameterHandler(std::move(handler)) {}


    void process(const TimePoint& time) {
        std::lock_guard<std::mutex> lock(process_mutex);
        for (auto& source: m_sources) {
            source->process(time);
        }
    }


    void add(std::unique_ptr<Generative> generative) {
        std::lock_guard<std::mutex> lock(process_mutex);

        if (std::find(m_generatives.begin(), m_generatives.end(), generative) != m_generatives.end())
            throw std::runtime_error("Cannot add a generative twice");

        if (auto* source = dynamic_cast<Source*>(generative.get())) {
            m_sources.emplace_back(source);
        }

        m_generatives.emplace_back(std::move(generative));
    }


    void add(std::vector<std::unique_ptr<Generative>> generatives) {
        for (auto& generative: generatives) {
            add(std::move(generative));
        }
    }


    void remove(Generative* generative) {
        (void) generative;
        std::lock_guard<std::mutex> lock(process_mutex);
//        std::cout << "DUMMY REMOVE (NOT IMPLEMENTED): THIS WILL LEAK!!!!\n";
        throw std::runtime_error("Don't forget to implement this to fix this leak"); // TODO
    }


    Generative* find(const std::string& generative_id) {
        auto it = std::find_if(m_generatives.begin()
                     , m_generatives.end()
                     , [&generative_id](const std::unique_ptr<Generative>& g) {
                    return g->get_identifier_as_string() == generative_id;
                });

        if (it != m_generatives.end())
            return it->get();
        else
            return nullptr;
    }

    void reposition() { throw std::runtime_error("not implemented"); /* TODO */ }


private:

    std::mutex process_mutex;

    std::vector<std::unique_ptr<Generative>> m_generatives;
    std::vector<Source*> m_sources;


};

#endif //SERIALISTLOOPER_MODULAR_GENERATOR_H
