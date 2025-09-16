@mainpage Project Documentation

# ESF

## building

    cmake -S . -B ./build -DCMAKE_TOOLCHAIN_FILE=<path to vcpkg>/scripts/buildsystems/vcpkg.cmake
    cmake --build build

## running

For running app You should specify path where to store protobuf data.
Also You need root for that and disable System Integrity Protection (SIP).

Example:

        sudo ./build/esf storage.data

To read stored date You can run:

        ./build/esf -p storage.data > storage.json && less storage.json

## build docs

    cmake -S . -B ./build -DCMAKE_TOOLCHAIN_FILE=<path to vcpkg>/scripts/buildsystems/vcpkg.cmake -DDOCS=1
    cmake --build build

    /Applications/Google\ Chrome.app/Contents/MacOS/Google\ Chrome ./build/docs/html/index.html

## build tests

    cmake -S . -B ./build -DCMAKE_TOOLCHAIN_FILE=<path to vcpkg>/scripts/buildsystems/vcpkg.cmake -DDOCS=1 -DTEST=1
    cmake --build build

    sudo ./build/tests
