#include "memory.h"
#include "error_macros.h"
#include <cstdlib>

namespace labfont {

// Conversion functions
inline MemoryCategory ToInternalCategory(lab_memory_category category) {
    switch (category) {
        case LAB_MEMORY_GENERAL: return MemoryCategory::General;
        case LAB_MEMORY_GRAPHICS: return MemoryCategory::Graphics;
        case LAB_MEMORY_TEXT: return MemoryCategory::Text;
        case LAB_MEMORY_RESOURCES: return MemoryCategory::Resources;
        case LAB_MEMORY_TEMPORARY: return MemoryCategory::Temporary;
        default: return MemoryCategory::General;
    }
}

inline lab_memory_stats ToPublicStats(const MemoryStats& stats) {
    lab_memory_stats result = {
        .totalAllocated = stats.totalAllocated,
        .totalFreed = stats.totalFreed,
        .currentUsage = stats.currentUsage,
        .peakUsage = stats.peakUsage,
        .categoryUsage = {0}
    };
    
    // Convert category usage
    result.categoryUsage[LAB_MEMORY_GENERAL] = stats.categoryUsage.at(MemoryCategory::General);
    result.categoryUsage[LAB_MEMORY_GRAPHICS] = stats.categoryUsage.at(MemoryCategory::Graphics);
    result.categoryUsage[LAB_MEMORY_TEXT] = stats.categoryUsage.at(MemoryCategory::Text);
    result.categoryUsage[LAB_MEMORY_RESOURCES] = stats.categoryUsage.at(MemoryCategory::Resources);
    result.categoryUsage[LAB_MEMORY_TEMPORARY] = stats.categoryUsage.at(MemoryCategory::Temporary);
    
    return result;
}

MemoryManager::MemoryManager() {
    m_categoryUsage[MemoryCategory::General] = 0;
    m_categoryUsage[MemoryCategory::Graphics] = 0;
    m_categoryUsage[MemoryCategory::Text] = 0;
    m_categoryUsage[MemoryCategory::Resources] = 0;
    m_categoryUsage[MemoryCategory::Temporary] = 0;
}

MemoryManager::~MemoryManager() {
    if (m_leakDetectionEnabled && m_currentUsage > 0) {
        DumpLeaks();
    }
}

MemoryManager& MemoryManager::Instance() {
    static MemoryManager instance;
    return instance;
}

void* MemoryManager::Allocate(size_t size, MemoryCategory category) {
    void* ptr = std::malloc(size);
    if (!ptr) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_totalAllocated += size;
    m_currentUsage += size;
    m_categoryUsage[category] += size;
    
    if (m_currentUsage > m_peakUsage) {
        m_peakUsage = m_currentUsage;
    }
    
    m_allocations[ptr] = {size, category, nullptr, 0};
    return ptr;
}

void MemoryManager::Free(void* ptr) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_allocations.find(ptr);
    if (it != m_allocations.end()) {
        const auto& info = it->second;
        m_totalFreed += info.size;
        m_currentUsage -= info.size;
        m_categoryUsage[info.category] -= info.size;
        m_allocations.erase(it);
    }
    
    std::free(ptr);
}

MemoryStats MemoryManager::GetStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return MemoryStats{
        .totalAllocated = m_totalAllocated,
        .totalFreed = m_totalFreed,
        .currentUsage = m_currentUsage,
        .peakUsage = m_peakUsage,
        .categoryUsage = m_categoryUsage
    };
}

void MemoryManager::ResetStats() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_totalAllocated = 0;
    m_totalFreed = 0;
    m_currentUsage = 0;
    m_peakUsage = 0;
    for (auto& [category, usage] : m_categoryUsage) {
        usage = 0;
    }
}

void MemoryManager::EnableLeakDetection(bool enable) {
    m_leakDetectionEnabled = enable;
}

void MemoryManager::DumpLeaks() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    // TODO: Implement leak reporting
}

} // namespace labfont

// C API implementations
extern "C" {

void* lab_alloc(size_t size, lab_memory_category category) {
    return labfont::MemoryManager::Instance().Allocate(size, labfont::ToInternalCategory(category));
}

void lab_free(void* ptr) {
    labfont::MemoryManager::Instance().Free(ptr);
}

lab_memory_stats lab_get_memory_stats(void) {
    return labfont::ToPublicStats(labfont::MemoryManager::Instance().GetStats());
}

} // extern "C"
