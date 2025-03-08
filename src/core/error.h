#ifndef LABFONT_ERROR_H
#define LABFONT_ERROR_H

#include "backend_types.h"

namespace labfont {

// Get a string representation of an error code
const char* GetResultString(lab_result error);

class ErrorState {
public:
    ErrorState() : m_lastResult(LAB_RESULT_OK) {}

    void SetError(lab_result error) {
        m_lastResult = error;
    }

    void ClearError() {
        m_lastResult = LAB_RESULT_OK;
    }

    lab_result GetLastError() const {
        return m_lastResult;
    }

protected:
    lab_result m_lastResult;
};

} // namespace labfont

#endif // LABFONT_ERROR_H
