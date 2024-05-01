# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\SmartSlateCompanion_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\SmartSlateCompanion_autogen.dir\\ParseCache.txt"
  "SmartSlateCompanion_autogen"
  )
endif()
