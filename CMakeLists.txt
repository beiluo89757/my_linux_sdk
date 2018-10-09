cmake_minimum_required (VERSION 2.8)

project (test)


set(SOURCE_FILES
	src/GateWay_Entrance.c
	src/device/UartRecV.c
	src/device/Device_Protocol.c
	src/device/Device_Json.c
	src/ota/gateway_ota.c
	platform/peripherals/platform_uart.c
	Utils/utils.c
	Utils/parson/parson.c
	Utils/md5/md5.c
)
include_directories(
	include
	src
	src/device
	src/ota
	platform
	platform/include
	platform/peripherals
	Utils/parson
	Utils/md5
	Utils
)
# aux_source_directory(. DIR_SRCS)

add_executable(test ${SOURCE_FILES})

target_link_libraries(test curl)
# target_link_libraries(test lcrypto)

