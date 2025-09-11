cmake_minimum_required(VERSION 4.1)
project(esf)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 20)

option(DEBUG "Compile with debug symbols" OFF)
if (DEBUG)
  add_compile_options(-O0 -g)
  add_link_options(-O0 -g)
endif(DEBUG)

option(ADDRESS_SANITIZER "Compile with address sanitizer" OFF)
if (ADDRESS_SANITIZER)
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
endif(ADDRESS_SANITIZER)

option("Compile with thread sanitizer" OFF)
if (THREAD_SANITIZER)
  add_compile_options(-fsanitize=thread)
  add_link_options(-fsanitize=thread)
endif(THREAD_SANITIZER)

find_library(ENDPOINT_SECURITY_FRAMEWORK EndpointSecurity)
find_library(FOUNDATION_FRAMEWORK Foundation)

if(NOT ENDPOINT_SECURITY_FRAMEWORK)
  message(FATAL_ERROR "EndpointSecurity framework not found")
endif()

set(SOURCES ./main.cpp ./events.proto)

set(HEADERS ./ESClient.h)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ./${CMAKE_CURRENT_SOURCE_DIR})

# target_link_libraries(${PROJECT_NAME} 
#   ${ENDPOINT_SECURITY_FRAMEWORK}
#   ${FOUNDATION_FRAMEWORK}
# )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND codesign --force --sign - --entitlements ${CMAKE_SOURCE_DIR}/entitlements.plist ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    COMMENT "Signing the executable with ad-hoc signature"
)

# set_target_properties(${PROJECT_NAME} PROPERTIES
    # XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Developer ID Application: Ivan Gorichenko"
    # XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_SOURCE_DIR}/entitlements.plist"
# )


#####################################
############PROTOBUF#################
#####################################

find_package(protobuf CONFIG REQUIRED)

protobuf_generate(
  TARGET ${PROJECT_NAME}
  LANGUAGE cpp
  PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/events.proto
)
# add_library(events_proto ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(${PROJECT_NAME} PRIVATE
  ${ENDPOINT_SECURITY_FRAMEWORK}
  ${FOUNDATION_FRAMEWORK}
  protobuf::libprotobuf)
# target_include_directories(events_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
# target_link_libraries(${PROJECT_NAME
#   events_proto
#   ${Protobuf_LIBRARIES}
# )
target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
)
