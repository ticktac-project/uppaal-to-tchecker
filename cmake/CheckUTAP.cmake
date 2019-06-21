find_package(LibXml2 REQUIRED)

function(find_utap)
    set(include_paths ${ARGV0})
    set(library_paths ${ARGV1})

    set(LIBUTAP_FOUND FALSE PARENT_SCOPE)

    find_path(UTAP_INCLUDE_DIR "utap/utap.h" PATHS "${include_paths}")
    if (NOT UTAP_INCLUDE_DIR)
        message(STATUS "utap/utap.h not found.")
        return()
    endif ()

    message(STATUS "Find utap/utap.h in ${UTAP_INCLUDE_DIR}")

    find_library(UTAP_LIBRARY_PATH utap PATHS "${library_paths}")
    if (NOT UTAP_LIBRARY_PATH)
        message(STATUS "UTAP library not found.")
        return()
    endif ()

    message(STATUS "Find UTAP library as ${UTAP_LIBRARY_PATH}")
    include(CheckCXXSymbolExists)
    set(CMAKE_REQUIRED_LIBRARIES ${UTAP_LIBRARY_PATH} ${LIBXML2_LIBRARIES})
    set(CMAKE_REQUIRED_INCLUDES ${UTAP_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR})
    check_cxx_symbol_exists(parseXMLFile "utap/utap.h" UTAP_USUABILITY)
    if (NOT UTAP_USUABILITY)
        message(STATUS "can not link with UTAP library.")
        return()
    endif ()

    set(LIBUTAP_FOUND TRUE PARENT_SCOPE)
    set(LIBUTAP_INCLUDES "${UTAP_INCLUDE_DIR};${LIBXML2_INCLUDE_DIR}" PARENT_SCOPE)
    set(LIBUTAP_LIBRARIES "${UTAP_LIBRARY_PATH};${LIBXML2_LIBRARIES}"
        PARENT_SCOPE)
endfunction(find_utap)

#
# Configure libutap located in ${srcdir} directory and compile it in
# ${bindir}. Then, the directory libutap is installed in stored in
# instdirvar.
# If the build process successes then resvar is assigned TRUE.
#
function(build_utap srcdir bindir instdirvar resvar)
    set(${resvar} FALSE PARENT_SCOPE)
    set(instdir "${bindir}/_inst")

    file(MAKE_DIRECTORY "${bindir}")

    execute_process(COMMAND autoreconf -i
                    RESULT_VARIABLE UTAP_CONF_OK
                    WORKING_DIRECTORY "${srcdir}")
    if (NOT UTAP_CONF_OK EQUAL 0)
        message(FATAL_ERROR "Fail to run 'autoreconf' in '${srcdir}' internal UTAP package.")
        return()
    endif ()

    set(ENV{CXX} "${CMAKE_CXX_COMPILER}")
    set(ENV{CXXFLAGS} "${CMAKE_CXX_FLAGS}")
    execute_process(COMMAND ${srcdir}/configure --prefix=${instdir}
                    RESULT_VARIABLE UTAP_CONF_OK
                    WORKING_DIRECTORY "${bindir}")
    if (NOT UTAP_CONF_OK EQUAL 0)
        file(STRINGS "${bindir}/config.log" cfglog)
        foreach(line ${cfglog})
            message("${cfglog}")
        endforeach()
        message(FATAL_ERROR "Fail to configure internal UTAP package.")
        return()
    endif ()
    execute_process(COMMAND make -j 4
                    RESULT_VARIABLE UTAP_BUILD_OK
                    WORKING_DIRECTORY "${bindir}")

    if (NOT UTAP_BUILD_OK EQUAL 0)
        message(FATAL_ERROR "An error occurs while compiling UTAP library.")
        return()
    endif ()
    execute_process(COMMAND make install
                    RESULT_VARIABLE UTAP_BUILD_INSTALLL
                    WORKING_DIRECTORY "${bindir}")
    if (${UTAP_BUILD_INSTALLL} EQUAL 0)
        message(STATUS "UTAP library has been installed successfully in ${instdir}")
        set(${resvar} TRUE PARENT_SCOPE)
        set(${instdirvar} ${instdir} PARENT_SCOPE)
    endif ()
endfunction(build_utap)

function(find_or_build_utap srcdir bindir)
    find_utap()

    if (NOT LIBUTAP_FOUND)
        message("*** UTAP library not found. Compile internal one.")
        build_utap(${srcdir} ${bindir} instdir UTAP_IS_BUILT)
        if(UTAP_IS_BUILT)
            message("insdir=${instdir}")
            find_utap("${instdir}/include" "${instdir}/lib")
            if (NOT LIBUTAP_FOUND)
                message(FATAL_ERROR "an error occurs with internal UTAP library installation")
            endif()
        endif()
    endif()

    if(LIBUTAP_FOUND)
        set(LIBUTAP_INCLUDES "${LIBUTAP_INCLUDES}" PARENT_SCOPE)
        set(LIBUTAP_LIBRARIES "${LIBUTAP_LIBRARIES}" PARENT_SCOPE)
    endif()
endfunction(find_or_build_utap)