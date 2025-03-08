#ifndef LABFONT_ERROR_H
#define LABFONT_ERROR_H

#include "backend_types.h"
#include <string>

namespace labfont {

// Get a string representation of an error code
const char* GetResultString(lab_result error);

class ErrorState {
public:
    ErrorState() : m_lastResult(LAB_RESULT_OK) {}

    void SetError(lab_result error, const char* message = nullptr) {
        m_lastResult = error;
        if (message) {
            m_lastMessage = std::string(GetResultString(error)) + ": " + message;
        } else {
            m_lastMessage = GetResultString(error);
        }
    }

    void SetError(lab_result error, const std::string& message) {
        m_lastResult = error;
        if (!message.empty()) {
            m_lastMessage = std::string(GetResultString(error)) + ": " + message;
        } else {
            m_lastMessage = GetResultString(error);
        }
    }

    void SetErrorWithContext(lab_result error, const std::string& context, const std::string& message) {
        m_lastResult = error;
        m_lastMessage = std::string(GetResultString(error)) + " in " + context;
        if (!message.empty()) {
            m_lastMessage += ": " + message;
        }
    }

    void ClearError() {
        m_lastResult = LAB_RESULT_OK;
        m_lastMessage.clear();
    }

    lab_result GetLastError() const {
        return m_lastResult;
    }

protected:
    lab_result m_lastResult;
    std::string m_lastMessage;
};

} // namespace labfont

#endif // LABFONT_ERROR_H
