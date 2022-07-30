##################################################
##     __  _             __          ____       
##    / /_(_)___  __  __/ /   ____  / __ \______
##   / __/ / __ \/ / / / /   / __ \/ /_/ / __  /
##  / /_/ / / / / /_/ / /___/ /_/ / _, _/ /_/ / 
##  \__/_/_/ /_/\__, /_____/\____/_/ |_|\__,_/  
##             /____/                 by HSLU                      
##
## Author: Julian Staffelbach (HSLU)

# minimum required cmake version
cmake_minimum_required(VERSION 3.12)

# set(CMAKE_VERBOSE_MAKEFILE ON)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# name of project
project(tinyLoRa-loramac-node)

# init sdk
pico_sdk_init()

# add subdirectories
add_subdirectory(src)
