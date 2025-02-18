mkdir -p build && cd build
cmake -G Xcode -DLABFONT_ENABLE_METAL=ON -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_WGPU=OFF ..
make
./tests/labfont_tests
cd ..
