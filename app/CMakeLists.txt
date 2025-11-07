#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
message("Here: ${CMAKE_SOURCE_DIR}/../drivers")
message("Here: $ENV{ZEPHYR_BASE}")
set(ZEPHYR_EXTRA_MODULES "${CMAKE_SOURCE_DIR}/../drivers/dac/tps55287q1")
# set(ZEPHYR_EXTRA_MODULES "${CMAKE_SOURCE_DIR}/../drivers/dac/tps55287q1")
# set(ZEPHYR_EXTRA_MODULES "../drivers")

project(app LANGUAGES C)

target_sources(app PRIVATE src/main.c)

# # Set C++ standard to what Zephyr supports
# set_property(TARGET app PROPERTY CXX_STANDARD 11)
# set_property(TARGET app PROPERTY CXX_STANDARD_REQUIRED ON)