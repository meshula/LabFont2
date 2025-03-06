#ifndef LABFONT_ERROR_H
#define LABFONT_ERROR_H

#include "backend_types.h"
#include <string>

namespace labfont {

// Get a string representation of an error code
const char* GetErrorString(lab_error error);

class ErrorState {
public:
    ErrorState() : m_lastError(LAB_ERROR_NONE) {}

    void SetError(lab_error error, const char* message = nullptr) {
        m_lastError = error;
        if (message) {
            m_lastMessage = std::string(GetErrorString(error)) + ": " + message;
        } else {
            m_lastMessage = GetErrorString(error);
        }
    }

    void SetError(lab_error error, const std::string& message) {
        m_lastError = error;
        if (!message.empty()) {
            m_lastMessage = std::string(GetErrorString(error)) + ": " + message;
        } else {
            m_lastMessage = GetErrorString(error);
        }
    }

    void SetErrorWithContext(lab_error error, const std::string& context, const std::string& message) {
        m_lastError = error;
        m_lastMessage = std::string(GetErrorString(error)) + " in " + context;
        if (!message.empty()) {
            m_lastMessage += ": " + message;
        }
    }

    void ClearError() {
        m_lastError = LAB_ERROR_NONE;
        m_lastMessage.clear();
    }

    lab_result GetLastError() const {
        return lab_result(m_lastError, m_lastMessage.c_str());
    }

protected:
    lab_error m_lastError;
    std::string m_lastMessage;
};

} // namespace labfont

#endif // LABFONT_ERROR_H
