#ifndef LABFONT_COORDINATE_SYSTEM_H
#define LABFONT_COORDINATE_SYSTEM_H

#include "labfont/labfont_types.h"

namespace labfont {

// Core coordinate system functions
lab_result InitializeCoordinateSystem(lab_coordinate_system* coord_system, const lab_coordinate_desc* desc);

lab_result TransformPoint(const lab_coordinate_system* coord_system,
                         lab_coordinate_space from_space, lab_coordinate_space to_space,
                         const float input[2], float output[2]);

lab_result TransformVertex(const lab_coordinate_system* coord_system,
                          lab_coordinate_space from_space, lab_coordinate_space to_space,
                          const lab_vertex_2TC* input, lab_vertex_2TC* output);

// Helper function to create backend-appropriate coordinate systems
lab_result CreateStandardCoordinateSystem(lab_coordinate_system* coord_system,
                                         lab_backend_type backend_type,
                                         uint32_t width, uint32_t height);

} // namespace labfont

#endif // LABFONT_COORDINATE_SYSTEM_H
