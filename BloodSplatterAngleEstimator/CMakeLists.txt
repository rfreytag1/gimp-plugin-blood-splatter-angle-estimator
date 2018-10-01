set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/BloodSplatterAngleEstimator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OptimaFinder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Parameters.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Helpers.cpp
        PARENT_SCOPE
)

set(HEADERS
        ${HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/BloodSplatterAngleEstimator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/OptimaFinder.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Parameters.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Helpers.h
        PARENT_SCOPE
)