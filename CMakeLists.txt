cmake_minimum_required(VERSION 3.11)
project(gimp-plugin-blood-splatter-angle-estimator)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(PkgConfig)

pkg_check_modules(GIMP REQUIRED gimp-2.0>=2.9.0)
pkg_check_modules(GIMPUI REQUIRED gimpui-2.0>=2.9.0)
pkg_check_modules(OPENCV REQUIRED opencv>=3.4.0)

# exclude OpenCV libraries that are linked against GTK3 and thus interfere with GIMP's GTK2
list(REMOVE_ITEM OPENCV_LIBRARIES opencv_highgui opencv_viz opencv_ccalib)

include_directories(
        ${GIMP_INCLUDE_DIRS}
        ${GIMPUI_INCLUDE_DIRS}
        ${OPENCV_INCLUDE_DIRS}
)

link_directories(
        ${GIMP_LIBRARY_DIRS}
        ${GIMPUI_LIBRARY_DIRS}
        ${OPENCV_LIBRARY_DIRS}
)

set(SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin_ui.cpp
)

set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/main.h
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin_ui.h
)

#message(${GIMP_LIBRARIES})
#message(${GIMPUI_LIBRARIES})
#message(${OPENCV_LIBRARIES})

add_subdirectory(BloodSplatterAngleEstimator)

add_executable(plug-in-blood-splash-analyzer ${SOURCE} ${HEADERS})

target_link_libraries(plug-in-blood-splash-analyzer
        ${GIMP_LIBRARIES}
        ${GIMPUI_LIBRARIES}
        ${OPENCV_LIBRARIES}
)

INSTALL(TARGETS plug-in-blood-splash-analyzer RUNTIME DESTINATION ${GIMP_PREFIX}/lib/gimp/2.0/plug-ins)