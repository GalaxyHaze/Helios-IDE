# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/Zith_Studio_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/Zith_Studio_autogen.dir/ParseCache.txt"
  "Zith_Studio_autogen"
  )
endif()
