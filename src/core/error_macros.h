#ifndef LABFONT_ERROR_MACROS_H
#define LABFONT_ERROR_MACROS_H

#include "error.h"

namespace labfont {

// Error handling macros
#define LAB_RESULT_GUARD() \
    ErrorState errorState; \
    lab_result result = LAB_RESULT_OK

#define LAB_RETURN_IF_ERROR(expr) \
    do { \
        auto result = (expr); \
        if (result != LAB_RESULT_OK) { \
            return result; \
        } \
    } while (0)

} // namespace labfont

#endif // LABFONT_ERROR_MACROS_H
