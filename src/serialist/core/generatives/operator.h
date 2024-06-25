
#ifndef SERIALISTLOOPER_OPERATOR_H
#define SERIALISTLOOPER_OPERATOR_H

#include "core/utility/math.h"
#include "core/utility/optionals.h"
#include "core/algo/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/algo/temporal/trigger.h"
#include "sequence.h"
#include "variable.h"

class Operator {
public:
    enum class Type {
        add
        , sub
        , mul
        , div
        , mod
        , pow
        , and_op
        , or_op
        , eq
        , ne
        , lt
        , le
        , gt
        , ge
        , min
        , max
        // END OF BINARY OPERATORS: Note that LAST_BINARY_OPERATOR must be updated, should this change
        , not_op
        , abs
        , ceil
        , floor
        , round
        , sqrt
        , sin
        , cos
        , tan
        , exp
        , log
        , nop
    };

    static const Type LAST_BINARY_OPERATOR = Type::max;

    // ================

    static Voice<Facet> process(Voice<Facet>& lhs, Voice<Facet>& rhs, std::optional<Type> type) {
        if (!type.has_value())
            return lhs;

        if (rhs.empty()) {
            if (is_binary(*type))
                return rhs;
            else
                return lhs.map([&type](const Facet& f) {
                    return Facet(Operator::process(static_cast<double>(f), std::nullopt, *type));
                });
        }


        if (lhs.empty())
            return lhs;

        if (lhs.size() < rhs.size()) {
            lhs.resize_fold(rhs.size());
        } else if (lhs.size() > rhs.size()) {
            rhs.resize_fold(lhs.size());
        }

        auto output = Voice<Facet>::allocated(lhs.size());
        for (std::size_t i = 0; i < lhs.size(); ++i) {
            output.append(Facet(Operator::process(static_cast<double>(lhs[i]), static_cast<double>(rhs[i]), *type)));
        }
        return output;


    }

    static double process(double lhs, std::optional<double> rhs, Type type) {
        switch (type) {
            case Type::add:
                return lhs + rhs.value_or(0.0);
            case Type::sub:
                return lhs - rhs.value_or(0.0);
            case Type::mul:
                return lhs * rhs.value_or(1.0);
            case Type::div:
                return divide(lhs, rhs);
            case Type::mod:
                return modulo(lhs, rhs);
            case Type::pow:
                return pow(lhs, rhs.value_or(1.0));
            case Type::and_op:
                return static_cast<bool>(lhs) && static_cast<bool>(rhs);
            case Type::or_op:
                return static_cast<bool>(lhs) || static_cast<bool>(rhs);
            case Type::not_op:
                return !static_cast<bool>(lhs);
            case Type::eq:
                return rhs.has_value() && utils::equals(lhs, *rhs);
            case Type::ne:
                return !rhs.has_value() || !utils::equals(lhs, *rhs);
            case Type::lt:
                return rhs.has_value() && lhs < rhs;
            case Type::le:
                return rhs.has_value() && lhs <= rhs;
            case Type::gt:
                return rhs.has_value() && lhs > rhs;
            case Type::ge:
                return rhs.has_value() && lhs >= rhs;
            case Type::min:
                return rhs.has_value() ? std::fmin(lhs, *rhs) : lhs;
            case Type::max:
                return rhs.has_value() ? std::fmax(lhs, *rhs) : lhs;
            case Type::abs:
                return std::fabs(lhs);
            case Type::ceil:
                return std::ceil(lhs);
            case Type::floor:
                return std::floor(lhs);
            case Type::round:
                return std::round(lhs);
            case Type::sqrt:
                return square_root(lhs);
            case Type::sin:
                return std::sin(lhs);
            case Type::cos:
                return std::cos(lhs);
            case Type::tan:
                return std::tan(lhs);
            case Type::exp:
                return std::exp(lhs);
            case Type::log:
                return logarithm(lhs);
            case Type::nop:
                return lhs;
            default:
                throw std::invalid_argument("Unknown operator type");
        }
    }

    /**
     * @throw std::domain_error if str cannot be parsed
     */
    static Type from_string(const std::string& str) {
        if (str == "add" || str == "+")
            return Type::add;
        if (str == "sub" || str == "-")
            return Type::sub;
        if (str == "mul" || str == "*")
            return Type::mul;
        if (str == "div" || str == "/")
            return Type::div;
        if (str == "mod" || str == "%")
            return Type::mod;
        if (str == "pow" || str == "**")
            return Type::pow;
        if (str == "and" || str == "&&")
            return Type::and_op;
        if (str == "or" || str == "||")
            return Type::or_op;
        if (str == "eq" || str == "==")
            return Type::eq;
        if (str == "ne" || str == "!=")
            return Type::ne;
        if (str == "lt" || str == "<")
            return Type::lt;
        if (str == "le" || str == "<=")
            return Type::le;
        if (str == "gt" || str == ">")
            return Type::gt;
        if (str == "ge" || str == ">=")
            return Type::ge;
        if (str == "min")
            return Type::min;
        if (str == "max")
            return Type::max;
        if (str == "abs")
            return Type::abs;
        if (str == "ceil")
            return Type::ceil;
        if (str == "floor")
            return Type::floor;
        if (str == "round")
            return Type::round;
        if (str == "sqrt")
            return Type::sqrt;
        if (str == "sin")
            return Type::sin;
        if (str == "cos")
            return Type::cos;
        if (str == "tan")
            return Type::tan;
        if (str == "exp")
            return Type::exp;
        if (str == "log")
            return Type::log;
        if (str == "nop")
            return Type::nop;
        throw std::domain_error("Unknown operator type");
    }


private:

    static bool is_binary(Type type) {
        return type <= LAST_BINARY_OPERATOR;
    }

    static double divide(double lhs, std::optional<double> rhs) {
        if (!rhs.has_value())
            return lhs;
        if (utils::equals(*rhs, 0.0))
            return 0.0;
        return lhs / *rhs;
    }

    static double modulo(double lhs, std::optional<double> rhs) {
        if (!rhs.has_value())
            return lhs;
        if (utils::equals(*rhs, 0.0))
            return 0.0;
        return utils::modulo(lhs, *rhs);
    }

    static double square_root(double lhs) {
        if (lhs < 0.0)
            return 0.0;
        return std::sqrt(lhs);
    }

    static double logarithm(double lhs) {
        if (lhs <= 0.0)
            return 0.0;
        return std::log(lhs);
    }

};


// ==============================================================================================

class OperatorNode : public NodeBase<Facet> {
public:
    struct Keys {
        static const inline std::string TYPE = "type";
        static const inline std::string LHS = "lhs";
        static const inline std::string RHS = "rhs";

        static const inline std::string CLASS_NAME = "operator";
    };

    // ================

    OperatorNode(const std::string& identifier
                 , ParameterHandler& parent
                 , Node<Trigger>* trigger = nullptr
                 , Node<Facet>* type = nullptr
                 , Node<Facet>* lhs = nullptr
                 , Node<Facet>* rhs = nullptr
                 , Node<Facet>* enabled = nullptr
                 , Node<Facet>* num_voices = nullptr)
            : NodeBase<Facet>(identifier, parent, enabled, num_voices, Keys::CLASS_NAME)
              , m_trigger(add_socket(ParameterKeys::TRIGGER, trigger))
              , m_type(add_socket(Keys::TYPE, type))
              , m_lhs(add_socket(Keys::LHS, lhs))
              , m_rhs(add_socket(Keys::RHS, rhs)) {}

    Voices<Facet> process() override {
        if (!pop_time()) return m_current_value;

        if (!is_enabled() || !m_trigger.is_connected() || !m_type.is_connected() || !m_lhs.is_connected()) {
            m_current_value = Voices<Facet>::singular(Facet(0.0));
            return m_current_value;
        }

        if (m_trigger.process().is_empty_like())
            return m_current_value;

        auto type = m_type.process();
        auto lhs = m_lhs.process();
        auto rhs = m_rhs.process();

        auto num_voices = voice_count(type.size(), lhs.size(), rhs.size());

        auto types = type.adapted_to(num_voices).firsts();
        auto lhses = lhs.adapted_to(num_voices);
        auto rhses = rhs.adapted_to(num_voices);

        auto output = Voices<Facet>::zeros(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            output[i] = Operator::process(lhses[i], rhses[i], utils::optional_cast<Operator::Type>(types[i]));
        }

        m_current_value = std::move(output);
        return m_current_value;

    }

    void set_trigger(Node<Trigger>* trigger) { m_trigger = trigger; }

    void set_type(Node<Facet>* type) { m_type = type; }

    void set_lhs(Node<Facet>* lhs) { m_lhs = lhs; }

    void set_rhs(Node<Facet>* rhs) { m_rhs = rhs; }

    Socket<Trigger>& get_trigger() { return m_trigger; }

    Socket<Facet>& get_type() { return m_type; }

    Socket<Facet>& get_lhs() { return m_lhs; }

    Socket<Facet>& get_rhs() { return m_rhs; }

private:
    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_type;
    Socket<Facet>& m_lhs;
    Socket<Facet>& m_rhs;

    Voices<Facet> m_current_value = Voices<Facet>::singular(Facet(0.0));
};


// ==============================================================================================

template<typename FloatType = float>
struct OperatorWrapper {
    using Keys = OperatorNode::Keys;

    ParameterHandler parameter_handler;

    Sequence<Trigger> trigger{ParameterKeys::TRIGGER, parameter_handler, Trigger::pulse_on()};
    Sequence<Facet, Operator::Type> type{Keys::TYPE, parameter_handler
                                         , Voices<Operator::Type>::singular(Operator::Type::add)};
    Sequence<Facet, FloatType> lhs{Keys::LHS, parameter_handler};
    Sequence<Facet, FloatType> rhs{Keys::RHS, parameter_handler};

    Sequence<Facet, bool> enabled{ParameterKeys::ENABLED, parameter_handler, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{ParameterKeys::NUM_VOICES, parameter_handler, 0};

    OperatorNode operator_node{Keys::CLASS_NAME
                               , parameter_handler
                               , &trigger
                               , &type
                               , &lhs
                               , &rhs
                               , &enabled
                               , &num_voices
    };
};


#endif //SERIALISTLOOPER_OPERATOR_H
