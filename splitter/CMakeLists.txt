cmake_minimum_required (VERSION 3.10)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("splitter")
add_executable (splitter "splitter.cpp" "splitter.h")

# add_subdirectory(tests)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET splitter PROPERTY CXX_STANDARD 14)
endif()
