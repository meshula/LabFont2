#include "error.h"

namespace labfont {

const char* GetErrorString(lab_error error) {
    switch (error) {
        case LAB_ERROR_NONE:
            return "No error";
        case LAB_ERROR_INVALID_PARAMETER:
            return "Invalid parameter";
        case LAB_ERROR_OUT_OF_MEMORY:
            return "Out of memory";
        case LAB_ERROR_BACKEND_ERROR:
            return "Backend error";
        case LAB_ERROR_FONT_LOAD_FAILED:
            return "Font load failed";
        case LAB_ERROR_NOT_INITIALIZED:
            return "Not initialized";
        case LAB_ERROR_INVALID_STATE:
            return "Invalid state";
        default:
            return "Unknown error";
    }
}

} // namespace labfont

// C interface implementation
extern "C" {

const char* lab_get_error_string(lab_error error) {
    return labfont::GetErrorString(error);
}

} // extern "C"
