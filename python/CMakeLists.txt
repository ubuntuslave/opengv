
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG QUIET)

if(NOT pybind11_FOUND)
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -m pybind11 --cmakedir
        OUTPUT_VARIABLE PYBIND11_CMAKEDIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE PYBIND11_CMAKEDIR_RESULT
    )

    if(PYBIND11_CMAKEDIR_RESULT EQUAL 0 AND EXISTS "${PYBIND11_CMAKEDIR}")
        list(APPEND CMAKE_PREFIX_PATH "${PYBIND11_CMAKEDIR}")
        find_package(pybind11 CONFIG REQUIRED)
    else()
        message(FATAL_ERROR
            "pybind11 not found. Install it in the selected Python environment "
            "(e.g. '${Python3_EXECUTABLE} -m pip install pybind11') or provide pybind11_DIR.")
    endif()
endif()

message(STATUS "pyopengv Python executable: ${Python3_EXECUTABLE}")
message(STATUS "pyopengv Python include dirs: ${Python3_INCLUDE_DIRS}")

add_library(pyopengv MODULE pyopengv.cpp)
target_link_libraries(pyopengv PRIVATE opengv pybind11::module)

set_target_properties(pyopengv PROPERTIES
    PREFIX ""
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

if(APPLE)
    set_target_properties(pyopengv PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()

# Find install path using the configured interpreter.
execute_process(
    COMMAND "${Python3_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_path('platlib'))"
    OUTPUT_VARIABLE PYTHON_PLATLIB
    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(PYTHON_INSTALL_DIR
    "${PYTHON_PLATLIB}"
    CACHE PATH "Path where to install pyopengv")

install(TARGETS pyopengv DESTINATION "${PYTHON_INSTALL_DIR}")
