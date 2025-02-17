mkdir -p build && cd build
VULKAN_SDK=/Users/nporcino/bin/VulkanSDK/1.4.304.1/macOS cmake -DLABFONT_ENABLE_VULKAN=ON -DLABFONT_ENABLE_WGPU=OFF ..
make
./tests/labfont_tests
cd ..

