cmake_minimum_required(VERSION 3.11)
project(test01)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(PkgConfig)

#find_package(GTK2 REQUIRED)

pkg_check_modules(GIMP REQUIRED gimp-2.0>=2.9.0)
pkg_check_modules(GIMPUI REQUIRED gimpui-2.0>=2.9.0)
pkg_check_modules(OPENCV REQUIRED opencv>=3.4.0)

include_directories(
        ${GIMP_INCLUDE_DIRS}
        ${GIMPUI_INCLUDE_DIRS}
        ${OPENCV_INCLUDE_DIRS}
)

add_executable(plug-in-blood-splash-analyzer main.cpp BloodSplatterAngleEstimator/BloodSplatterAngleEstimator.cpp BloodSplatterAngleEstimator/OptimaFinder.cpp BloodSplatterAngleEstimator/Helpers.cpp BloodSplatterAngleEstimator/Parameters.cpp)

target_link_libraries(plug-in-blood-splash-analyzer
        ${GIMP_LIBRARIES}
        ${GIMPUI_LIBRARIES}
        ${OPENCV_LIBRARIES}
)

INSTALL(TARGETS plug-in-blood-splash-analyzer RUNTIME DESTINATION ${GIMP_PREFIX}/lib/gimp/2.0/plug-ins)