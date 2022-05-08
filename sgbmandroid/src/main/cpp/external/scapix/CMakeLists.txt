message(STATUS "Configuring Scapix for ${SCAPIX_BRIDGE}")

set(HEADER_FILES ${CMAKE_SOURCE_DIR}/include/DepthEstimator.h)

include(FetchContent)
FetchContent_Declare(
        cmodule
        URL "https://github.com/scapix-com/cmodule/archive/refs/tags/v1.0.34.tar.gz"
        URL_HASH SHA256=b9292c8022d0f2bc37a1e6acb9c2fb8626219e926fd7254d931f373b8a2c1ec2
)
FetchContent_MakeAvailable(cmodule)

find_package(Scapix REQUIRED)

scapix_bridge_headers(${PROJECT_NAME} me.timpushkin.sgbmandroid ${HEADER_FILES})

message(STATUS "Configuring Scapix - done")
