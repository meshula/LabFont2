#ifndef LABFONT_ERROR_H
#define LABFONT_ERROR_H

#include "backend_types.h"
#include <string>
#include <memory>

namespace labfont {

// Error context to track error details
class ErrorContext {
public:
    static ErrorContext& Instance() {
        static ErrorContext instance;
        return instance;
    }

    void SetError(lab_error code, const char* message) {
        m_lastError = code;
        m_lastMessage = message ? message : "";
    }

    void ClearError() {
        m_lastError = LAB_ERROR_NONE;
        m_lastMessage.clear();
    }

    lab_result GetLastError() const {
        return lab_result(m_lastError, m_lastMessage.empty() ? nullptr : m_lastMessage.c_str());
    }

private:
    ErrorContext() = default;
    ~ErrorContext() = default;

    lab_error m_lastError = LAB_ERROR_NONE;
    std::string m_lastMessage;
};

// RAII error scope guard
class ErrorGuard {
public:
    ErrorGuard() {
        ErrorContext::Instance().ClearError();
    }

    ~ErrorGuard() = default;

    void SetError(lab_error code, const char* message) {
        ErrorContext::Instance().SetError(code, message);
    }

    lab_result GetError() const {
        return ErrorContext::Instance().GetLastError();
    }
};

// Helper macros for error handling
#define LAB_ERROR_GUARD() ErrorGuard errorGuard
#define LAB_SET_ERROR(code, message) errorGuard.SetError(code, message)
#define LAB_GET_ERROR() errorGuard.GetError()
#define LAB_RETURN_ERROR(code, message) do { LAB_SET_ERROR(code, message); return LAB_GET_ERROR(); } while(0)
#define LAB_RETURN_IF_ERROR(result) do { if (result.error != LAB_ERROR_NONE) return result; } while(0)

// Error message utilities
const char* GetErrorString(lab_error error);

} // namespace labfont

#endif // LABFONT_ERROR_H
