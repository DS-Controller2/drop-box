cmake_minimum_required(VERSION 3.14)
project(drop-box CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Enable testing for CTest integration
enable_testing()

# --- Fetch GoogleTest using FetchContent ---
# This downloads GoogleTest during CMake configuration.
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.12.1.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
# This is often recommended for Windows builds with FetchContent.
if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)
# --- End FetchContent for GoogleTest ---

# --- Fetch cppcrypto using FetchContent ---
include(FetchContent)

FetchContent_Declare(
    cppcrypto
    URL https://sourceforge.net/projects/cppcrypto/files/cppcrypto-0.20-src.zip/download # Corrected URL for 0.20-src.zip
)

FetchContent_MakeAvailable(cppcrypto)
# --- End FetchContent for cppcrypto ---

# Find the ZLIB package
find_package(ZLIB REQUIRED)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql) # Specify all necessary components

# Include directories for headers
include_directories(include)
include_directories(${cppcrypto_SOURCE_DIR})


# Define the core library
add_library(drop-box-core STATIC
    src/FileSystemManager.cpp
    src/EncryptionManager.cpp
    src/CompressionManager.cpp
    src/Utils.cpp
    src/ExclusionManager.cpp
    src/FileMonitor.cpp
    src/MetadataManager.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/argon2.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/cpuinfo.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/pbkdf2.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/random_bytes.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/sha256.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/sha512.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/hmac.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/crypto_hash.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/crypto_kdf.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/crypto_mac.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/blake2.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/thread_pool.cpp
    ${cppcrypto_SOURCE_DIR}/cppcrypto/3rdparty/blake2b.c
    ${cppcrypto_SOURCE_DIR}/cppcrypto/3rdparty/blake2s.c
)

target_link_libraries(drop-box-core PRIVATE sodium ZLIB::ZLIB Qt6::Core Qt6::Sql)

# Add source files for the main executable
qt6_wrap_cpp(drop-box_MOC_SRCS include/FileMonitor.h)
add_executable(drop-box
    src/main.cpp
    src/LoadingScreen.cpp
    src/SettingsDialog.cpp
    include/LoadingScreen.h
    include/SettingsDialog.h
    include/ExclusionManager.h
    include/MetadataManager.h
    ${drop-box_MOC_SRCS}
)

# Link libraries to your executable
target_link_libraries(drop-box PRIVATE drop-box-core Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql)

# Add your test code subdirectory
add_subdirectory(tests)

# No need for qt_standard_project_setup if we manually set AUTOUIC, AUTOMOC, AUTORCC and link components
