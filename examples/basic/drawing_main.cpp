// Simple example of drawing a red triangle using LabFont abstraction
// Based on the original WebGPU example at https://github.com/beaufortfrancois/webgpu-cross-platform-app

#include <GLFW/glfw3.h>
#include <labfont/labfont.h>
#include <iostream>
#include <vector>

// Window dimensions
const uint32_t kWidth = 512;
const uint32_t kHeight = 512;

// Global variables
lab_context context = nullptr;
GLFWwindow* window = nullptr;

// Initialize GLFW and create a window
bool InitWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(kWidth, kHeight, "LabFont Triangle Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    return true;
}

// Global render target
lab_render_target render_target = nullptr;

// Initialize LabFont context
bool InitLabFont() {
    // Create a backend descriptor
    lab_backend_desc backend_desc = {
        .type = LAB_BACKEND_CPU, // Use CPU backend for portability
        .width = kWidth,
        .height = kHeight,
        .native_window = window
    };

    // Create the context
    lab_result result = lab_create_context(&backend_desc, &context);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to create LabFont context: " << lab_get_error_string(result);
        return false;
    }

    // Create a render target
    lab_render_target_desc rt_desc = {
        .width = kWidth,
        .height = kHeight,
        .format = LAB_FORMAT_RGBA8_UNORM,
        .hasDepth = true
    };
    
    result = lab_create_render_target(context, &rt_desc, &render_target);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to create render target: " << lab_get_error_string(result);
        return false;
    }
    
    // Set the render target
    result = lab_set_render_target(context, render_target);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to set render target: " << lab_get_error_string(result);
        return false;
    }

    return true;
}

// Create a triangle using LabFont vertices
std::vector<lab_vertex_2TC> CreateTriangleVertices() {
    // Define a red triangle
    std::vector<lab_vertex_2TC> vertices = {
        // Top vertex (red)
        {
            .position = {0.0f, 0.5f},
            .texcoord = {0.5f, 0.0f},
            .color = {1.0f, 0.0f, 0.0f, 1.0f}
        },
        // Bottom left vertex (red)
        {
            .position = {-0.5f, -0.5f},
            .texcoord = {0.0f, 1.0f},
            .color = {1.0f, 0.0f, 0.0f, 1.0f}
        },
        // Bottom right vertex (red)
        {
            .position = {0.5f, -0.5f},
            .texcoord = {1.0f, 1.0f},
            .color = {1.0f, 0.0f, 0.0f, 1.0f}
        }
    };

    return vertices;
}

// Render a frame
lab_result Render() {
    // Begin the frame
    lab_result result = lab_begin_frame(context);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to begin frame: " << lab_get_error_string(result);
        return result;
    }

    // Create clear command (black background)
    lab_draw_command clear_cmd = {
        .type = LAB_DRAW_COMMAND_CLEAR,
        .clear = {
            .color = {0.0f, 0.0f, 0.0f, 1.0f}
        }
    };

    // Get triangle vertices
    std::vector<lab_vertex_2TC> vertices = CreateTriangleVertices();

    // Create triangle draw command
    lab_draw_command triangle_cmd = {
        .type = LAB_DRAW_COMMAND_TRIANGLES,
        .triangles = {
            .vertices = vertices.data(),
            .vertexCount = static_cast<uint32_t>(vertices.size())
        }
    };

    // Submit commands
    lab_draw_command commands[] = {clear_cmd, triangle_cmd};
    result = lab_submit_commands(context, commands, 2);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to submit commands: " << lab_get_error_string(result);
        return result;
    }

    // End the frame
    result = lab_end_frame(context);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to end frame: " << lab_get_error_string(result);
        return result;
    }
    
    return LAB_RESULT_OK;
}

// Cleanup resources
void Cleanup() {
    if (render_target && context) {
        lab_destroy_render_target(context, render_target);
        render_target = nullptr;
    }
    
    if (context) {
        lab_destroy_context(context);
        context = nullptr;
    }

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

int main() {
    // Initialize window
    if (!InitWindow()) {
        return 1;
    }

    // Initialize LabFont
    if (!InitLabFont()) {
        Cleanup();
        return 1;
    }

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (Render() != LAB_RESULT_OK) {
            break;
        }
    }

    // Cleanup
    Cleanup();
    return 0;
}
