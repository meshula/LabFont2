#include "coordinate_system.h"
#include "error.h"
#include <cstring>
#include <cmath>

namespace labfont {

// 3x3 matrix utilities for 2D homogeneous coordinates
static void MatrixMultiply3x3(const float a[9], const float b[9], float result[9]) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result[i * 3 + j] = 0.0f;
            for (int k = 0; k < 3; ++k) {
                result[i * 3 + j] += a[i * 3 + k] * b[k * 3 + j];
            }
        }
    }
}

static float MatrixDeterminant3x3(const float m[9]) {
    return m[0] * (m[4] * m[8] - m[5] * m[7]) -
           m[1] * (m[3] * m[8] - m[5] * m[6]) +
           m[2] * (m[3] * m[7] - m[4] * m[6]);
}

static bool MatrixInverse3x3(const float m[9], float result[9]) {
    float det = MatrixDeterminant3x3(m);
    if (std::abs(det) < 1e-6f) {
        return false; // Matrix is not invertible
    }
    
    float invDet = 1.0f / det;
    
    result[0] = (m[4] * m[8] - m[5] * m[7]) * invDet;
    result[1] = (m[2] * m[7] - m[1] * m[8]) * invDet;
    result[2] = (m[1] * m[5] - m[2] * m[4]) * invDet;
    result[3] = (m[5] * m[6] - m[3] * m[8]) * invDet;
    result[4] = (m[0] * m[8] - m[2] * m[6]) * invDet;
    result[5] = (m[2] * m[3] - m[0] * m[5]) * invDet;
    result[6] = (m[3] * m[7] - m[4] * m[6]) * invDet;
    result[7] = (m[1] * m[6] - m[0] * m[7]) * invDet;
    result[8] = (m[0] * m[4] - m[1] * m[3]) * invDet;
    
    return true;
}

static void CreateTransformMatrix(const float from_origin[2], const float from_size[2],
                                const float to_origin[2], const float to_size[2],
                                float matrix[9]) {
    // Create transformation matrix from one coordinate space to another
    // T = Translate(to_origin) * Scale(to_size/from_size) * Translate(-from_origin)
    
    float scale_x = to_size[0] / from_size[0];
    float scale_y = to_size[1] / from_size[1];
    
    float translate_x = to_origin[0] - from_origin[0] * scale_x;
    float translate_y = to_origin[1] - from_origin[1] * scale_y;
    
    // Build the 3x3 transformation matrix
    matrix[0] = scale_x;  matrix[1] = 0.0f;     matrix[2] = translate_x;
    matrix[3] = 0.0f;     matrix[4] = scale_y;  matrix[5] = translate_y;
    matrix[6] = 0.0f;     matrix[7] = 0.0f;     matrix[8] = 1.0f;
}

static void TransformPoint2D(const float matrix[9], const float input[2], float output[2]) {
    // Apply 3x3 homogeneous transformation to 2D point
    float x = input[0];
    float y = input[1];
    
    output[0] = matrix[0] * x + matrix[1] * y + matrix[2];
    output[1] = matrix[3] * x + matrix[4] * y + matrix[5];
    // Note: We assume w=1 for 2D points, so we don't need to compute the third component
}

lab_result InitializeCoordinateSystem(lab_coordinate_system* coord_system, const lab_coordinate_desc* desc) {
    if (!coord_system || !desc) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    // Copy the description
    std::memcpy(&coord_system->desc, desc, sizeof(lab_coordinate_desc));
    
    // Create transformation matrices
    
    // Local to Normalized
    CreateTransformMatrix(desc->local_origin, desc->local_size,
                         desc->normalized_origin, desc->normalized_size,
                         coord_system->local_to_normalized);
    
    // Normalized to Device
    CreateTransformMatrix(desc->normalized_origin, desc->normalized_size,
                         desc->device_origin, desc->device_size,
                         coord_system->normalized_to_device);
    
    // Local to Device (composition of the above)
    MatrixMultiply3x3(coord_system->normalized_to_device, coord_system->local_to_normalized,
                      coord_system->local_to_device);
    
    // Compute inverse transformations
    if (!MatrixInverse3x3(coord_system->local_to_normalized, coord_system->normalized_to_local)) {
        return LAB_RESULT_INVALID_PARAMETER; // Singular matrix
    }
    
    if (!MatrixInverse3x3(coord_system->normalized_to_device, coord_system->device_to_normalized)) {
        return LAB_RESULT_INVALID_PARAMETER; // Singular matrix
    }
    
    if (!MatrixInverse3x3(coord_system->local_to_device, coord_system->device_to_local)) {
        return LAB_RESULT_INVALID_PARAMETER; // Singular matrix
    }
    
    // Cache determinants for performance
    coord_system->local_to_normalized_det = MatrixDeterminant3x3(coord_system->local_to_normalized);
    coord_system->normalized_to_device_det = MatrixDeterminant3x3(coord_system->normalized_to_device);
    
    return LAB_RESULT_OK;
}

lab_result TransformPoint(const lab_coordinate_system* coord_system,
                         lab_coordinate_space from_space, lab_coordinate_space to_space,
                         const float input[2], float output[2]) {
    if (!coord_system || !input || !output) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    if (from_space == to_space) {
        // No transformation needed
        output[0] = input[0];
        output[1] = input[1];
        return LAB_RESULT_OK;
    }
    
    const float* matrix = nullptr;
    
    // Select the appropriate transformation matrix
    if (from_space == LAB_COORD_LOCAL && to_space == LAB_COORD_NORMALIZED) {
        matrix = coord_system->local_to_normalized;
    } else if (from_space == LAB_COORD_LOCAL && to_space == LAB_COORD_DEVICE) {
        matrix = coord_system->local_to_device;
    } else if (from_space == LAB_COORD_NORMALIZED && to_space == LAB_COORD_DEVICE) {
        matrix = coord_system->normalized_to_device;
    } else if (from_space == LAB_COORD_NORMALIZED && to_space == LAB_COORD_LOCAL) {
        matrix = coord_system->normalized_to_local;
    } else if (from_space == LAB_COORD_DEVICE && to_space == LAB_COORD_NORMALIZED) {
        matrix = coord_system->device_to_normalized;
    } else if (from_space == LAB_COORD_DEVICE && to_space == LAB_COORD_LOCAL) {
        matrix = coord_system->device_to_local;
    } else {
        return LAB_RESULT_INVALID_PARAMETER; // Unsupported transformation
    }
    
    TransformPoint2D(matrix, input, output);
    return LAB_RESULT_OK;
}

lab_result TransformVertex(const lab_coordinate_system* coord_system,
                          lab_coordinate_space from_space, lab_coordinate_space to_space,
                          const lab_vertex_2TC* input, lab_vertex_2TC* output) {
    if (!coord_system || !input || !output) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    // Copy the input vertex
    *output = *input;
    
    // Transform the position
    lab_result result = TransformPoint(coord_system, from_space, to_space,
                                      input->position, output->position);
    if (result != LAB_RESULT_OK) {
        return result;
    }
    
    // Note: texture coordinates and colors are not transformed
    // They remain in their original coordinate space
    
    return LAB_RESULT_OK;
}

// Helper function to create standard coordinate systems

lab_result CreateStandardCoordinateSystem(lab_coordinate_system* coord_system,
                                         lab_backend_type backend_type,
                                         uint32_t width, uint32_t height) {
    lab_coordinate_desc desc = {};
    
    // Device coordinates depend on backend type
    if (backend_type == LAB_BACKEND_CPU) {
        // CPU backend uses pixel coordinates
        desc.device_origin[0] = 0.0f;
        desc.device_origin[1] = 0.0f;
        desc.device_size[0] = static_cast<float>(width);
        desc.device_size[1] = static_cast<float>(height);
    } else {
        // GPU backends (Metal, Vulkan, etc.) typically use NDC
        desc.device_origin[0] = -1.0f;
        desc.device_origin[1] = -1.0f;
        desc.device_size[0] = 2.0f;
        desc.device_size[1] = 2.0f;
    }
    
    // Normalized coordinates: (0,1) space
    desc.normalized_origin[0] = 0.0f;
    desc.normalized_origin[1] = 0.0f;
    desc.normalized_size[0] = 1.0f;
    desc.normalized_size[1] = 1.0f;
    
    // Local coordinates: (-1,1) space (standard graphics coordinates)
    desc.local_origin[0] = -1.0f;
    desc.local_origin[1] = -1.0f;
    desc.local_size[0] = 2.0f;
    desc.local_size[1] = 2.0f;
    
    return InitializeCoordinateSystem(coord_system, &desc);
}

} // namespace labfont
