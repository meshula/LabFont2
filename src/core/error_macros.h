#ifndef LABFONT_ERROR_MACROS_H
#define LABFONT_ERROR_MACROS_H

#include "error.h"

namespace labfont {

// Error handling macros
#define LAB_ERROR_GUARD() \
    ErrorState errorState; \
    lab_result result = {LAB_ERROR_NONE, nullptr}

#define LAB_RETURN_ERROR(error, message) \
    do { \
        errorState.SetError(error, message); \
        return errorState.GetLastError(); \
    } while (0)

#define LAB_RETURN_IF_ERROR(expr) \
    do { \
        auto result = (expr); \
        if (result.error != LAB_ERROR_NONE) { \
            return result; \
        } \
    } while (0)

} // namespace labfont

#endif // LABFONT_ERROR_MACROS_H
