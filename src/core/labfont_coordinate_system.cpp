#include "labfont/labfont.h"
#include "coordinate_system.h"
#include "context_internal.h"

extern "C" {

lab_result lab_init_coordinate_system(lab_coordinate_system* coord_system, const lab_coordinate_desc* desc) {
    return labfont::InitializeCoordinateSystem(coord_system, desc);
}

lab_result lab_set_coordinate_system(lab_context ctx, const lab_coordinate_system* coord_system) {
    auto context = reinterpret_cast<labfont::Context*>(ctx);
    if (!context || !coord_system) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    context->SetCoordinateSystem(*coord_system);
    return LAB_RESULT_OK;
}

lab_result lab_get_coordinate_system(lab_context ctx, lab_coordinate_system* out_coord_system) {
    auto context = reinterpret_cast<labfont::Context*>(ctx);
    if (!context || !out_coord_system) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    *out_coord_system = context->GetCoordinateSystem();
    return LAB_RESULT_OK;
}

lab_result lab_transform_point(const lab_coordinate_system* coord_system,
                              lab_coordinate_space from_space, lab_coordinate_space to_space,
                              const float input[2], float output[2]) {
    return labfont::TransformPoint(coord_system, from_space, to_space, input, output);
}

lab_result lab_transform_vertex(const lab_coordinate_system* coord_system,
                               lab_coordinate_space from_space, lab_coordinate_space to_space,
                               const lab_vertex_2TC* input, lab_vertex_2TC* output) {
    return labfont::TransformVertex(coord_system, from_space, to_space, input, output);
}

} // extern "C"
