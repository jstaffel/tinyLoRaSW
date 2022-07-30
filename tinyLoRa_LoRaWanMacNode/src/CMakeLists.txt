##################################################
##     __  _             __          ____       
##    / /_(_)___  __  __/ /   ____  / __ \____ _
##   / __/ / __ \/ / / / /   / __ \/ /_/ / __ `/
##  / /_/ / / / / /_/ / /___/ /_/ / _, _/ /_/ / 
##  \__/_/_/ /_/\__, /_____/\____/_/ |_|\__,_/  
##             /____/                 by HSLU                      
##
## Author: Julian Staffelbach (HSLU)

# minimum required cmake version
project(lora-mac)
cmake_minimum_required(VERSION 3.12)

# Allow switching of secure-elements
set(SECURE_ELEMENT_LIST SOFT_SE LR1110_SE)
set(SECURE_ELEMENT SOFT_SE CACHE STRING "Default secure element is SOFT_SE")
set_property(CACHE SECURE_ELEMENT PROPERTY STRINGS ${SECURE_ELEMENT_LIST})

# Allow switching of Applications
set(APPLICATION_LIST LoRaMac ping-pong rx-sensi tx-cw )
set(APPLICATION LoRaMac CACHE STRING "Default Application is LoRaMac")
set_property(CACHE APPLICATION PROPERTY STRINGS ${APPLICATION_LIST})

# Switch for Class B support of LoRaMac.
option(CLASSB_ENABLED "Class B support of LoRaMac" OFF)

# Configure radio
set(RADIO sx126x CACHE INTERNAL "Radio sx126x selected")

# add subdirectories
add_subdirectory(boards)
add_subdirectory(boards/tinyLoRa)
add_subdirectory(mac)
add_subdirectory(peripherals)
add_subdirectory(radio)
add_subdirectory(system)

if(APPLICATION STREQUAL LoRaMac)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps/LoRaMac)

elseif(APPLICATION STREQUAL ping-pong)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps/ping-pong)

endif()