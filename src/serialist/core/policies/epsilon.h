
#ifndef SERIALIST_EPSILON_H
#define SERIALIST_EPSILON_H

namespace serialist {

#ifndef SERIALIST_EPSILON
#define SERIALIST_EPSILON 1e-8
#endif


inline double EPSILON = SERIALIST_EPSILON;


/**
 * This function is intended for testing only, and should never be used at runtime.
 * For build-related settings, use the SERIALIST_EPSILON macro instead.
 */
inline void override_epsilon(double epsilon) {
    EPSILON = epsilon;
}

}

#endif //SERIALIST_EPSILON_H
