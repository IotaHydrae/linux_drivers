cmake_minimum_required(VERSION 3.10)

project(usbmouse)

add_executable(usbmouse usbmouse.c)

target_link_libraries(usbmouse usb-1.0)
