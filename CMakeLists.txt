cmake_minimum_required(VERSION 4.1)
project(esf)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall -Wextra -Werror -Wno-format-security)

option(DEBUG "Compile with debug symbols" OFF)
if (DEBUG)
  add_compile_options(-O0 -gdwarf-4)
  add_link_options(-O0 -gdwarf-4)
endif(DEBUG)

option(ADDR_SAN "Compile with address sanitizer" OFF)
if (ADDR_SAN)
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
endif(ADDR_SAN)

option(THREAD_SAN "Compile with thread sanitizer" OFF)
if (THREAD_SAN)
  add_compile_options(-fsanitize=thread)
  add_link_options(-fsanitize=thread)
endif(THREAD_SAN)

find_library(ENDPOINT_SECURITY_FRAMEWORK EndpointSecurity)
find_library(FOUNDATION_FRAMEWORK Foundation)

if(NOT ENDPOINT_SECURITY_FRAMEWORK)
  message(FATAL_ERROR "EndpointSecurity framework not found")
endif()

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR} /events.proto)

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h)


# set_target_properties(${PROJECT_NAME} PROPERTIES
    # XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Developer ID Application: Ivan Gorichenko"
    # XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_SOURCE_DIR}/entitlements.plist"
# )


#####################################
############PROTOBUF#################
#####################################

find_package(protobuf CONFIG REQUIRED)

file(GLOB PROTO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.proto)
set(PROTO_TARGET generate_proto_sources)
add_custom_target(${PROTO_TARGET}
    DEPENDS ${PROTOBUF_GENERATED_SOURCES} ${PROTOBUF_GENERATED_HEADERS}
    COMMENT "Generating all protobuf sources"
)
protobuf_generate(
  TARGET ${PROTO_TARGET}
  LANGUAGE cpp
  PROTOS ${PROTO_SOURCES}
  OUT_VAR PROTOBUF_GENERATED_SOURCES
)


#####################################
##############GTEST##################
#####################################

option(TEST "Make tests" OFF)
if (TEST)
  add_compile_definitions(__TEST__)

  set(TEST_TARGET tests)
  file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tst/*.cpp)
  file(GLOB TEST_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/tst/*.h)

  enable_testing()
  find_package(GTest CONFIG REQUIRED)
  add_executable(${TEST_TARGET}
    ${TEST_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Storage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ESClient.cpp
    ${PROTOBUF_GENERATED_SOURCES}
  )
  target_link_libraries(${TEST_TARGET} PRIVATE
    ${ENDPOINT_SECURITY_FRAMEWORK}
    ${FOUNDATION_FRAMEWORK}
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
    protobuf::libprotobuf
  )
  target_include_directories(${TEST_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/tst
    ${CMAKE_CURRENT_BINARY_DIR}
  )
  add_custom_command(TARGET ${TEST_TARGET} POST_BUILD
    COMMAND codesign --force --sign - --entitlements ${CMAKE_SOURCE_DIR}/entitlements.plist ${CMAKE_BINARY_DIR}/${TEST_TARGET}
    COMMENT "Signing the executable with ad-hoc signature"
  )

  include(GoogleTest)
  gtest_discover_tests(${TEST_TARGET})
endif(TEST)

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES} ${PROTOBUF_GENERATED_SOURCES})

add_dependencies(${PROJECT_NAME} ${PROTO_TARGET})

target_include_directories(${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${ENDPOINT_SECURITY_FRAMEWORK}
  ${FOUNDATION_FRAMEWORK}
  nlohmann_json
  protobuf::libprotobuf
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND codesign --force --sign - --entitlements ${CMAKE_SOURCE_DIR}/entitlements.plist ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
  COMMENT "Signing the executable with ad-hoc signature"
)
