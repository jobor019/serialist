

#ifndef SERIALISTLOOPER_MODULAR_GENERATOR_H
#define SERIALISTLOOPER_MODULAR_GENERATOR_H

#include <mutex>
#include <regex>
#include "source.h"

class ModularGenerator {
public:
    static const int N_DIGITS_ID = 6;


    explicit ModularGenerator(ParameterHandler&& handler)
            : m_parameter_handler(std::move(handler)) {}


    void process(const TimePoint& time) {
        std::lock_guard<std::mutex> lock(process_mutex);
        for (auto& source: m_sources) {
            source->process(time);
        }
    }

    ParameterHandler & get_parameter_handler(){
        return m_parameter_handler;
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


    void remove(Generative& generative) {
        std::lock_guard<std::mutex> lock(process_mutex);
        remove_internal(generative);

    }


    void remove(const std::vector<Generative*>& generatives) {
        std::lock_guard<std::mutex> lock(process_mutex);
        remove_internal(generatives);

    }


    void remove_generative_and_children(Generative& generative) {
        std::lock_guard<std::mutex> lock{process_mutex};

        auto generative_and_children = find_generatives_matching(
                generative.get_parameter_handler().get_identifier_as_string());
        remove_internal(generative_and_children);
    }


    Generative* find(const std::string& generative_id) {
        auto it = std::find_if(m_generatives.begin()
                               , m_generatives.end()
                               , [&generative_id](const std::unique_ptr<Generative>& g) {
                    return g->get_parameter_handler().get_identifier_as_string() == generative_id;
                });

        if (it != m_generatives.end())
            return it->get();
        else
            return nullptr;
    }


    void print_names() {
        std::cout << "names: ";
        for (const auto& g: m_generatives) {
            std::cout << g->get_parameter_handler().get_identifier_as_string() << ", ";
        }
        std::cout << "\n";
    }


    std::vector<Generative*> find_generatives_matching(const std::string& base_name) {
        std::vector<Generative*> matching_generatives;

        // match exact base name as well as any children on format <base_name>::.*
        std::regex regex("^" + base_name + "(:{2}.*)?$");

        for (const auto& generative: m_generatives) {
            if (generative->get_parameter_handler().identifier_matches(regex)) {
                matching_generatives.push_back(generative.get());
            }
        }

        return matching_generatives;
    }


    std::size_t size() {
        return m_generatives.size();
    }


    std::string next_id() {
        std::lock_guard<std::mutex> lock{process_mutex};
        std::string str = std::to_string(++m_next_id);
        str = std::string(N_DIGITS_ID - str.length(), '0') + str;
        return str;
    }


    std::string next_free_name(const std::string& suggested_name) {
        std::lock_guard<std::mutex> lock{process_mutex};


        if (std::find_if(m_generatives.begin()
                         , m_generatives.end()
                         , [&suggested_name](const auto& g) {
                    return g->get_parameter_handler().identifier_equals(suggested_name);
                }) == m_generatives.end()) {
            return suggested_name;
        }

        std::vector<std::string> conflicting_names;
        for (const auto& generative: m_generatives) {
            if (generative->get_parameter_handler().identifier_begins_with(suggested_name))
                conflicting_names.emplace_back(generative->get_parameter_handler().get_identifier_as_string());
        }

        // TODO: Naive approach, might need optimization for large patches
        int i = 2;
        std::string new_name = suggested_name + std::to_string(i);
        while (std::find(conflicting_names.begin(), conflicting_names.end(), new_name) != conflicting_names.end()) {
            i += 1;
            new_name = suggested_name + std::to_string(i);
        }
        return new_name;
    }


private:

    void remove_internal(Generative& generative) {
        if (auto source = dynamic_cast<Source*>(&generative)) {
            m_sources.erase(std::remove(m_sources.begin(), m_sources.end(), source), m_sources.end());
        }

        m_generatives.erase(
                std::remove_if(
                        m_generatives.begin()
                        , m_generatives.end()
                        , [&generative](const auto& e) { return e.get() == &generative; }
                ), m_generatives.end());
    }


    void remove_internal(const std::vector<Generative*>& generatives) {
        // TODO: Optimize with erase-remove if slow
        for (auto* generative: generatives) {
            if (generative)
                remove_internal(*generative);
        }
    }


    ParameterHandler m_parameter_handler;

    std::mutex process_mutex;

    std::vector<std::unique_ptr<Generative>> m_generatives;
    std::vector<Source*> m_sources;

    int m_next_id = 1;


};

#endif //SERIALISTLOOPER_MODULAR_GENERATOR_H
