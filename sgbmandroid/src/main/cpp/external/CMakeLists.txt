add_subdirectory(opencv)

if (DEFINED SCAPIX_BRIDGE)
    add_subdirectory(scapix)
endif ()

if (ANDROID)
    add_subdirectory(log-lib)
endif ()
