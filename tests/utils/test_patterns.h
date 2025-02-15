#ifndef LABFONT_TEST_PATTERNS_H
#define LABFONT_TEST_PATTERNS_H

#include <labfont/labfont_types.h>
#include <core/backend_types.h>
#include <vector>
#include <cstdint>
#include <cmath>

namespace labfont {
namespace test {

// Pattern generation utilities
class PatternGenerator {
public:
    // Generate a checkerboard pattern with specified colors
    template<typename T>
    static std::vector<T> GenerateCheckerboard(
        uint32_t width,
        uint32_t height,
        uint32_t blockSize,
        const T* color1,
        const T* color2,
        size_t channelCount)
    {
        std::vector<T> data(width * height * channelCount);
        
        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                bool isColor1 = ((x / blockSize) + (y / blockSize)) % 2 == 0;
                const T* color = isColor1 ? color1 : color2;
                
                for (size_t c = 0; c < channelCount; ++c) {
                    data[(y * width + x) * channelCount + c] = color[c];
                }
            }
        }
        
        return data;
    }
    
    // Generate a gradient pattern
    template<typename T>
    static std::vector<T> GenerateGradient(
        uint32_t width,
        uint32_t height,
        const T* startColor,
        const T* endColor,
        size_t channelCount)
    {
        std::vector<T> data(width * height * channelCount);
        
        for (uint32_t y = 0; y < height; ++y) {
            float t = static_cast<float>(y) / (height - 1);
            
            for (uint32_t x = 0; x < width; ++x) {
                for (size_t c = 0; c < channelCount; ++c) {
                    T value = static_cast<T>(
                        startColor[c] * (1.0f - t) + endColor[c] * t
                    );
                    data[(y * width + x) * channelCount + c] = value;
                }
            }
        }
        
        return data;
    }
};

// Pixel buffer comparison utilities
class PixelComparator {
public:
    // Compare two pixel buffers with tolerance
    template<typename T>
    static bool CompareBuffers(
        const T* buffer1,
        const T* buffer2,
        size_t pixelCount,
        size_t channelCount,
        T tolerance)
    {
        for (size_t i = 0; i < pixelCount * channelCount; ++i) {
            T diff = std::abs(buffer1[i] - buffer2[i]);
            if (diff > tolerance) {
                return false;
            }
        }
        return true;
    }
    
    // Compare floating point buffers with relative tolerance
    static bool CompareBuffersFloat(
        const float* buffer1,
        const float* buffer2,
        size_t pixelCount,
        size_t channelCount,
        float relativeTolerance = 1e-5f)
    {
        for (size_t i = 0; i < pixelCount * channelCount; ++i) {
            float a = buffer1[i];
            float b = buffer2[i];
            float maxAbs = std::max(std::abs(a), std::abs(b));
            
            if (maxAbs > 0) {
                float relDiff = std::abs(a - b) / maxAbs;
                if (relDiff > relativeTolerance) {
                    return false;
                }
            } else if (a != b) {
                return false;
            }
        }
        return true;
    }
    
    // Get maximum difference between two buffers
    template<typename T>
    static T GetMaxDifference(
        const T* buffer1,
        const T* buffer2,
        size_t pixelCount,
        size_t channelCount)
    {
        T maxDiff = 0;
        for (size_t i = 0; i < pixelCount * channelCount; ++i) {
            T diff = std::abs(buffer1[i] - buffer2[i]);
            maxDiff = std::max(maxDiff, diff);
        }
        return maxDiff;
    }
    
    // Calculate mean squared error
    template<typename T>
    static double CalculateMSE(
        const T* buffer1,
        const T* buffer2,
        size_t pixelCount,
        size_t channelCount)
    {
        double sumSquaredDiff = 0.0;
        for (size_t i = 0; i < pixelCount * channelCount; ++i) {
            double diff = static_cast<double>(buffer1[i]) - static_cast<double>(buffer2[i]);
            sumSquaredDiff += diff * diff;
        }
        return sumSquaredDiff / (pixelCount * channelCount);
    }
};

} // namespace test
} // namespace labfont

#endif // LABFONT_TEST_PATTERNS_H
