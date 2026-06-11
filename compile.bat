rmdir /s /q build

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/Users/Admin/Downloads/vcpkg-master/scripts/buildsystems/vcpkg.cmake

cmake --build build --config Debug