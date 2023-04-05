

#ifndef SERIALIST_LOOPER_INTERPOLATOR_H
#define SERIALIST_LOOPER_INTERPOLATOR_H

#include <vector>
#include <optional>

template<typename T>
class Interpolator {
public:
    virtual ~Interpolator() = default;

    virtual std::optional<T> interpolate(double x, const std::vector<T>& v) = 0;
};


// ==============================================================================================

template<typename T>
class ContinueInterpolator : public Interpolator<T> {
public:
    explicit ContinueInterpolator(T focal_point) : m_focal_point(focal_point) {}


    std::optional<T> interpolate(double x, const std::vector<T>& v) override {
        if (v.empty()) {
            return std::nullopt;
        }

        auto index = static_cast<std::size_t>(std::floor(static_cast<double>(v.size()) * std::fmod(x, 1.0)));
        return {std::floor(x) * m_focal_point + v.at(index)};
    }


    T get_focal_point() const {
        return m_focal_point;
    }

private:
    T m_focal_point;

};


// ==============================================================================================

template<typename T>
class ModuloInterpolator : public Interpolator<T> {
public:
    std::optional<T> interpolate(double x, const std::vector<T>& v) override {
        if (v.empty()) {
            return std::nullopt;
        }

        return v.at(static_cast<std::size_t>(std::floor(std::fmod(x, 1.0) * static_cast<double>(v.size()))));
    }
};


// ==============================================================================================

template<typename T>
class ClipInterpolator : public Interpolator<T> {
public:
    std::optional<T> interpolate(double x, const std::vector<T>& v) override {
        if (v.empty()) {
            return std::nullopt;
        }

        return v.at(std::min(v.size() - 1, static_cast<std::size_t>(std::floor(x * static_cast<double>(v.size())))));
    }
};


// ==============================================================================================

template<typename T>
class PassInterpolator : public Interpolator<T> {
public:
    std::optional<T> interpolate(double x, const std::vector<T>& v) override {
        if (v.empty()) {
            return std::nullopt;
        }

        auto i = static_cast<std::size_t>(std::floor(x * static_cast<double>(v.size())));

        if (i >= v.size()) {
            return std::nullopt;
        }

        return v.at(i);
    }

};

#endif //SERIALIST_LOOPER_INTERPOLATOR_H
