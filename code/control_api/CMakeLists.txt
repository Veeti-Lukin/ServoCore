add_subdirectory(template)

# The Windows backend uses the Win32 serial API, so it only builds when the target is Windows
if (WIN32)
    add_subdirectory(windows)
endif ()