cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("upp_sp1")

add_executable(upp_sp1
        main.cpp
        core/AppRunner.h
        core/AppRunner.cpp
        io/CsvParser.h
        io/CsvParser.cpp
        io/CsvWriter.h
        io/CsvWriter.cpp
        processing/Aggregator.h
        processing/Aggregator.cpp
        processing/AnomalyDetector.h
        processing/AnomalyDetector.cpp
        processing/Filter.h
        processing/Filter.cpp
        output/ColorMapper.h
        output/ColorMapper.cpp
        output/CoordMapper.h
        output/CoordMapper.cpp
        output/SvgWriter.h
        output/SvgWriter.cpp
        data/Station.h
        data/Measurement.h
        data/Anomaly.h
        utils/Config.h
        utils/Timer.h
)

target_include_directories(upp_sp1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
