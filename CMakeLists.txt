cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("upp_sp1")

add_executable(upp_sp1
        src/main.cpp
        src/core/AppRunner.h
        src/core/AppRunner.cpp
        src/io/CsvParser.h
        src/io/CsvParser.cpp
        src/io/CsvWriter.h
        src/io/CsvWriter.cpp
        src/processing/Aggregator.h
        src/processing/Aggregator.cpp
        src/processing/AnomalyDetector.h
        src/processing/AnomalyDetector.cpp
        src/processing/Filter.h
        src/processing/Filter.cpp
        src/output/ColorMapper.h
        src/output/ColorMapper.cpp
        src/output/CoordMapper.h
        src/output/CoordMapper.cpp
        src/output/SvgWriter.h
        src/output/SvgWriter.cpp
        src/data/Station.h
        src/data/Measurement.h
        src/data/Anomaly.h
        src/utils/Config.h
        src/utils/Timer.h
)

target_include_directories(upp_sp1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
