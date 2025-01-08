

#ifndef SERIALISTLOOPER_GENERATION_GRAPH_H
#define SERIALISTLOOPER_GENERATION_GRAPH_H

#include <mutex>
#include <regex>
#include "core/generative.h"
#include "serialist/core/policies/policies.h"
#include "core/param/parameter_keys.h"
#include "core/temporal/time_point.h"

namespace serialist {

class GraphUtils {
public:
    using IndexGraph = std::unordered_map<std::size_t, std::vector<std::size_t>>;

    GraphUtils() = delete;


    static std::vector<std::vector<Generative*>> find_cycles(const std::vector<std::unique_ptr<Generative>>& generatives) {
        auto dependency_graph = compute_dependency_graph(generatives);

        auto cycles_as_indices = find_circular_dependencies(dependency_graph);

        std::vector<std::vector<Generative*>> cycles_as_generatives;
        for (auto& cycle: cycles_as_indices) {
            std::vector<Generative*> generative_cycle;
            for (auto& index: cycle) {
                generative_cycle.push_back(generatives.at(index).get());
            }
            cycles_as_generatives.push_back(generative_cycle);
        }

        return cycles_as_generatives;
    }


private:
    static IndexGraph compute_dependency_graph(const std::vector<std::unique_ptr<Generative>>& generatives) {
        IndexGraph dependency_graph;

        for (std::size_t i = 0; i < generatives.size(); ++i) {
            auto dependencies = indices_of(generatives.at(i)->get_connected(), generatives);
            dependency_graph.insert({i, dependencies});
        }

        return dependency_graph;
    }


    static std::vector<std::vector<std::size_t>> find_circular_dependencies(IndexGraph& graph) {
        std::unordered_map<std::size_t, bool> visited;
        std::vector<std::vector<std::size_t>> circular_dependencies;

        for (const auto& pair: graph) {
            std::size_t node = pair.first;
            if (!visited[node]) {
                std::vector<std::size_t> path;
                std::vector<std::vector<std::size_t>> cycle = cycles_from(node, graph, visited, path);
                circular_dependencies.insert(circular_dependencies.end(), cycle.begin(), cycle.end());
            }
        }

        return circular_dependencies;
    }


    /**
     *
     * @throw std::runtime_error if `generative` is not in `generatives`
     */
    static std::size_t index_of(Generative& generative, const std::vector<std::unique_ptr<Generative>>& generatives) {
        for (std::size_t i = 0; i < generatives.size(); ++i) {
            if (generatives.at(i).get() == &generative) {
                return i;
            }
        }
        throw std::runtime_error("unregistered generative encountered in index_of");
    }


    /**
     *
     * @throw std::runtime_error if `generative` is not in `generatives`
     */
    static std::vector<std::size_t> indices_of(const std::vector<Generative*> subset
                                               , const std::vector<std::unique_ptr<Generative>>& full_set) {
        std::vector<std::size_t> output;
        output.reserve(subset.size());

        for (auto* generative: subset) {
            output.emplace_back(index_of(*generative, full_set));
        }

        return output;
    }


    static std::vector<std::vector<std::size_t>> cycles_from(std::size_t node
                                                      , IndexGraph& graph
                                                      , std::unordered_map<std::size_t, bool>& visited
                                                      , std::vector<std::size_t>& path) {
        visited[node] = true;
        path.push_back(node);

        std::vector<std::vector<std::size_t>> cycles;
        for (std::size_t neighbor: graph[node]) {
            if (!visited[neighbor]) {
                std::vector<std::vector<std::size_t>> cycle = cycles_from(neighbor, graph, visited, path);
                cycles.insert(cycles.end(), cycle.begin(), cycle.end());
            } else if (std::find(path.begin(), path.end(), neighbor) != path.end()) {
                auto cycle_start = std::find(path.begin(), path.end(), neighbor) - path.begin();
                std::vector<std::size_t> cycle(path.begin() + cycle_start, path.end());
                cycles.push_back(cycle);
            }
        }

        path.pop_back();
        return cycles;
    }

};


// ==============================================================================================

class GenerationGraph {
public:
    static const int N_DIGITS_ID = 6;


    explicit GenerationGraph(ParameterHandler& root)
            : m_parameter_handler("", root, ParameterTypes::GENERATIVES_TREE) {}


    void process(const TimePoint& time) {
        std::lock_guard<std::mutex> lock(m_process_mutex);
        for (auto& generative: m_generatives) {
            generative->update_time(time);
        }

        for (auto* source: m_sources) {
            source->process();
        }
    }


    ParameterHandler& get_parameter_handler() {
        return m_parameter_handler;
    }


    void add(std::unique_ptr<Generative> generative) {
        std::lock_guard<std::mutex> lock(m_process_mutex);
        add_internal(std::move(generative));

        print_cycles();
    }


    void add(std::vector<std::unique_ptr<Generative>> generatives) {
        std::lock_guard<std::mutex> lock{m_process_mutex};
        for (auto& generative: generatives) {
            add_internal(std::move(generative));
        }
        print_cycles();
    }


    void remove(Generative& generative) {
        std::lock_guard<std::mutex> lock(m_process_mutex);
        remove_internal(generative);

    }


    void remove(const std::vector<Generative*>& generatives) {
        std::lock_guard<std::mutex> lock(m_process_mutex);
        remove_internal(generatives);
    }


    void remove_generative_and_children(Generative& generative) {
        std::lock_guard<std::mutex> lock{m_process_mutex};

        auto generative_and_children = find_generatives_matching(
                generative.get_parameter_handler().get_id());

        disconnect_if(generative_and_children);
        remove_internal(generative_and_children);
    }


    Generative* find(const std::string& generative_id) {
        auto it = std::find_if(m_generatives.begin()
                               , m_generatives.end()
                               , [&generative_id](const std::unique_ptr<Generative>& g) {
                    return g->get_parameter_handler().get_id() == generative_id;
                });

        if (it != m_generatives.end())
            return it->get();
        else
            return nullptr;
    }


    void print_names() const {
        std::cout << "names: ";
        for (const auto& g: m_generatives) {
            std::cout << g->get_parameter_handler().get_id() << ", ";
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
        std::lock_guard<std::mutex> lock{m_process_mutex};
        std::string str = std::to_string(++m_last_id);
        str = std::string(N_DIGITS_ID - str.length(), '0') + str;
        return str;
    }


//    std::string next_free_name(const std::string& suggested_name) {
//        std::lock_guard<std::mutex> lock{m_process_mutex};
//
//
//        if (std::find_if(m_generatives.begin()
//                         , m_generatives.end()
//                         , [&suggested_name](const auto& g) {
//                    return g->get_parameter_handler().identifier_equals(suggested_name);
//                }) == m_generatives.end()) {
//            return suggested_name;
//        }
//
//        std::vector<std::string> conflicting_names;
//        for (const auto& generative: m_generatives) {
//            if (generative->get_parameter_handler().identifier_begins_with(suggested_name))
//                conflicting_names.emplace_back(generative->get_parameter_handler().get_id());
//        }
//
//        // TODO: Naive approach, might need optimization for large patches
//        int i = 2;
//        std::string new_name = suggested_name + std::to_string(i);
//        while (std::find(conflicting_names.begin(), conflicting_names.end(), new_name) != conflicting_names.end()) {
//            i += 1;
//            new_name = suggested_name + std::to_string(i);
//        }
//        return new_name;
//    }


private:
    void print_cycles() const {
        for (auto& cycle : GraphUtils::find_cycles(m_generatives)) {
            // TODO: Temp: find better solution to indicate cycles
            std::cout << "Cycle detected: ";
            for (auto* node : cycle) {
                std::cout << node->get_parameter_handler().get_id() << " ";
            }
            std::cout << "\n";
        }
    }

    void add_internal(std::unique_ptr<Generative> generative) {
        if (std::find(m_generatives.begin(), m_generatives.end(), generative) != m_generatives.end())
            throw std::runtime_error("Cannot add a generative twice");

        if (auto* source = dynamic_cast<Root*>(generative.get())) {
            m_sources.emplace_back(source);
        }

        m_generatives.emplace_back(std::move(generative));
    }


    void disconnect_if(const std::vector<Generative*>& connected_to) {
        for (auto* connected: connected_to) {
            if (connected)
                disconnect_if(*connected);
        }
    }


    void disconnect_if(Generative& connected_to) {
        for (auto& generative: m_generatives) {
            generative->disconnect_if(connected_to);
        }
    }


    void remove_internal(Generative& generative) {
        if (auto* source = dynamic_cast<Root*>(&generative)) {
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

    std::mutex m_process_mutex;

    std::vector<std::unique_ptr<Generative>> m_generatives;
    std::vector<Root*> m_sources;

    int m_last_id = 0;


};

} // namespace serialist

#endif //SERIALISTLOOPER_GENERATION_GRAPH_H
