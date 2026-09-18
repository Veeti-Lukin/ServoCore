# Copies the compiler runtime libraries an executable needs next to it.
#
# Run in script mode - file(GET_RUNTIME_DEPENDENCIES) is not available at configure time:
#
#   cmake -DEXECUTABLE=<exe> -DDESTINATION=<dir> -DSEARCH_DIRS=<dirs> -P deploy_runtime_dependencies.cmake
#
# The dependencies are read out of the binary itself rather than listed by name, so this works for
# whichever compiler produced it. It also picks up transitive dependencies that never appear in the
# executable's own import table (IE. on GCC libwinpthread, is pulled in by libstdc++ and not by the exe)
#
# Only SEARCH_DIRS is searched, which is normally the compiler's bin directory. Anything else the
# binary needs (IE. Qt dlls) stays unresolved on purpose and is left to its own deployment step.

if (NOT EXECUTABLE OR NOT DESTINATION)
    message(FATAL_ERROR "EXECUTABLE and DESTINATION must both be set")
endif ()

file(GET_RUNTIME_DEPENDENCIES
        EXECUTABLES "${EXECUTABLE}"
        RESOLVED_DEPENDENCIES_VAR _resolved
        UNRESOLVED_DEPENDENCIES_VAR _unresolved
        DIRECTORIES ${SEARCH_DIRS}
        PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
        POST_EXCLUDE_REGEXES ".*[Ss]ystem32.*"
)

if (_resolved)
    file(COPY ${_resolved} DESTINATION "${DESTINATION}")
    foreach (_dll IN LISTS _resolved)
        get_filename_component(_name "${_dll}" NAME)
        message(STATUS "Deployed runtime library ${_name}")
    endforeach ()
endif ()
