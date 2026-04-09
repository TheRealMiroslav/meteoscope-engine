cmake_minimum_required(VERSION 3.20)
project(UPP_Semestralka_1)

set(CMAKE_CXX_STANDARD 20)

# Ruční vypsání souborů - máš nad tím 100% kontrolu
add_executable(UPP_Semestralka_1
        main.cpp
        io/CsvParser.cpp
        io/CsvWriter.cpp
        output/ColorMapper.cpp
        output/CoordMapper.cpp
        output/SvgWriter.cpp
        processing/Aggregator.cpp
        processing/AnomalyDetector.cpp
        processing/Filter.cpp
        data/Station.h
        data/Measurement.h
        io/CsvWriter.h
        processing/Filter.h
        processing/Aggregator.h
        data/Anomaly.h
        processing/AnomalyDetector.h
        output/SvgWriter.h
        utils/Config.h
        output/CoordMapper.h
        output/ColorMapper.h
        output/SvgWriter.cpp
        core/AppRunner.h
        core/AppRunner.cpp
        utils/Timer.h
)

find_package(OpenMP REQUIRED)
target_link_libraries(UPP_Semestralka_1 PUBLIC OpenMP::OpenMP_CXX)