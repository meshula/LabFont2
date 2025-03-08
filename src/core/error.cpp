#include "error.h"

namespace labfont {

const char* GetResultString(lab_result error) {
    switch (error) {
        case LAB_RESULT_OK:
            return "No error";
        case LAB_RESULT_INVALID_PARAMETER:
            return "Invalid parameter";
        case LAB_RESULT_OUT_OF_MEMORY:
            return "Out of memory";
        case LAB_RESULT_BACKEND_ERROR:
            return "Backend error";
        case LAB_RESULT_FONT_LOAD_FAILED:
            return "Font load failed";
        case LAB_RESULT_NOT_INITIALIZED:
            return "Not initialized";
        //case LAB_RESULT_INVALID_STATE:
        //    return "Invalid state";
        case LAB_RESULT_INITIALIZATION_FAILED:
            return "Initialization failed";
        case LAB_RESULT_INVALID_OPERATION:
            return "Invalid operation";
        case LAB_RESULT_COMMAND_BUFFER:
            return "Command buffer error";
        case LAB_RESULT_UNSUPPORTED_FORMAT:
            return "Unsupported format";
        case LAB_RESULT_DEVICE_LOST:
            return "Device lost";
        case LAB_RESULT_STATE_NO_RENDER_TARGET_SET:
            return "Invalid state: No render target set";
        case LAB_RESULT_INVALID_DIMENSION:
            return "Invalid Parameter: Dimension";
        case LAB_RESULT_INVALID_TEXTURE:
            return "Invalid texture";
        case LAB_RESULT_READBACK_NOT_SUPPORTED:
            return "Readback not supported";
        case LAB_RESULT_INVALID_RENDER_TARGET:
            return "Invalid render target";
        case LAB_RESULT_UNSUPPORTED_BACKEND:
            return "Unsupported back end";
        case LAB_RESULT_INVALID_CONTEXT:
            return "Invalid context";
        case LAB_RESULT_INVALID_RESOURCE_NAME:
            return "Invalid resource name";
        case LAB_RESULT_DUPLICATE_RESOURCE_NAME:
            return "Duplicate resource name";
        case LAB_RESULT_INVALID_BUFFER_SIZE:
            return "Invalid buffer size";
        case LAB_RESULT_INVALID_BUFFER:
            return "Invalid buffer";
        case LAB_RESULT_TEXTURE_CREATION_FAILED:
            return "Texture creation failed";
        case LAB_RESULT_SHADER_LIBRARY_INITIALIZATION_FAILED:
            return "Shader library initialization failed";
        case LAB_RESULT_DEVICE_INITIALIZATION_FAILED:
            return "Device initialization failed";
        case LAB_RESULT_RENDERTARGET_INITIALIZATION_FAILED:
            return "Render target initialization failed";
        case LAB_RESULT_COMMAND_BUFFER_INITIALIZATION_FAILED:
            return "Command buffer initialization failed";
        case LAB_RESULT_COMMAND_ENCODER_INITIALIZATION_FAILED:
            return "Command encoder initialization failed";
        case LAB_RESULT_INVALID_COMMAND_BUFFER:
            return "Invalid command buffer";
        default:
            return "Unknown error";
    }
}

} // namespace labfont

// C interface implementation
extern "C" {

const char* lab_get_error_string(lab_result error) {
    return labfont::GetResultString(error);
}

} // extern "C"
