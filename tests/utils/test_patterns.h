#ifndef LABFONT_TEST_PATTERNS_H
#define LABFONT_TEST_PATTERNS_H

#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>

namespace labfont {

class PatternGenerator {
public:
    template<typename T>
    static std::vector<T> GenerateCheckerboard(
        uint32_t width,
        uint32_t height,
        uint32_t tileSize,
        const T* color1,
        const T* color2,
        uint32_t channels
    ) {
        std::vector<T> pattern(width * height * channels);
        
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                bool isColor1 = ((x / tileSize) + (y / tileSize)) % 2 == 0;
                const T* color = isColor1 ? color1 : color2;
                
                uint32_t pixelIndex = (y * width + x) * channels;
                std::memcpy(&pattern[pixelIndex], color, channels * sizeof(T));
            }
        }
        
        return pattern;
    }
    
    template<typename T>
    static std::vector<T> GenerateGradient(
        uint32_t width,
        uint32_t height,
        const T* startColor,
        const T* endColor,
        uint32_t channels
    ) {
        std::vector<T> pattern(width * height * channels);
        
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                float t = static_cast<float>(x) / (width - 1);
                
                uint32_t pixelIndex = (y * width + x) * channels;
                for (uint32_t c = 0; c < channels; c++) {
                    pattern[pixelIndex + c] = static_cast<T>(
                        startColor[c] * (1.0f - t) + endColor[c] * t
                    );
                }
            }
        }
        
        return pattern;
    }
};

class PixelComparator {
public:
    template<typename T>
    static bool CompareBuffers(
        const T* buffer1,
        const T* buffer2,
        uint32_t pixelCount,
        uint32_t channels,
        T tolerance
    ) {
        for (uint32_t i = 0; i < pixelCount * channels; i++) {
            T diff = std::abs(buffer1[i] - buffer2[i]);
            if (diff > tolerance) {
                return false;
            }
        }
        return true;
    }
    
    template<typename T>
    static float CalculatePSNR(
        const T* buffer1,
        const T* buffer2,
        uint32_t pixelCount,
        uint32_t channels,
        T maxValue
    ) {
        double mse = 0.0;
        uint32_t totalSamples = pixelCount * channels;
        
        for (uint32_t i = 0; i < totalSamples; i++) {
            double diff = buffer1[i] - buffer2[i];
            mse += diff * diff;
        }
        
        mse /= totalSamples;
        if (mse == 0.0) {
            return INFINITY;
        }
        
        double maxValueSquared = static_cast<double>(maxValue) * maxValue;
        return 10.0 * log10(maxValueSquared / mse);
    }
};

} // namespace labfont

#endif // LABFONT_TEST_PATTERNS_H
