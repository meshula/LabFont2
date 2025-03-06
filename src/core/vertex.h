#ifndef LABFONT_VERTEX_H
#define LABFONT_VERTEX_H

#include "../../include/labfont/labfont_types.h"
#include <cstring>

namespace labfont {

// C++ wrapper for lab_vertex_2TC
struct Vertex {
    float position[2];  // x, y
    float texcoord[2];  // u, v
    float color[4];     // r, g, b, a

    Vertex() = default;

    Vertex(float px, float py, float u, float v, const float col[4]) {
        position[0] = px;
        position[1] = py;
        texcoord[0] = u;
        texcoord[1] = v;
        std::memcpy(color, col, sizeof(float) * 4);
    }

    // Conversion constructor from lab_vertex_2TC to Vertex
    Vertex(const lab_vertex_2TC& v) {
        std::memcpy(position, v.position, sizeof(position));
        std::memcpy(texcoord, v.texcoord, sizeof(texcoord));
        std::memcpy(color, v.color, sizeof(color));
    }

    // Conversion operator to lab_vertex_2TC
    operator lab_vertex_2TC() const {
        lab_vertex_2TC result;
        std::memcpy(result.position, position, sizeof(position));
        std::memcpy(result.texcoord, texcoord, sizeof(texcoord));
        std::memcpy(result.color, color, sizeof(color));
        return result;
    }
};

} // namespace labfont

#endif // LABFONT_VERTEX_H
