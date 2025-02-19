#ifndef LABFONT_BACKEND_TYPES_H
#define LABFONT_BACKEND_TYPES_H

#include "labfont/labfont_types.h"
#include <string>

namespace labfont {

// Forward declarations
class Context;
class Backend;
class CommandBuffer;
class Resource;
class Texture;
class Buffer;
class RenderTarget;

// Result type for internal operations
struct lab_result {
    lab_error error;
    std::string message;

    lab_result() : error(LAB_ERROR_NONE) {}
    lab_result(lab_error e) : error(e) {}
    lab_result(lab_error e, const char* msg) : error(e), message(msg ? msg : "") {}
    lab_result(lab_error e, const std::string& msg) : error(e), message(msg) {}

    operator lab_operation_result() const {
        return lab_operation_result{
            error,
            message.empty() ? nullptr : message.c_str()
        };
    }
};

} // namespace labfont

#endif // LABFONT_BACKEND_TYPES_H
