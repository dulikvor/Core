include(ExternalProject) #In order to load the ExternalProject module and to add the ExternalProject_Add command
set (CURRENT_PROJECT_3RD_LOC ${PROJECT_SOURCE_DIR}/Third_Party CACHE PATH "External projects location") #Sets a variable with the installation location of all third party
set_directory_properties(PROPERTIES EP_PREFIX ${CURRENT_PROJECT_3RD_LOC}) #Sets the prefix(dir) for all installations commands
if(CORE_SPDLOG_SUPPORT)
    include(InstallSpdLog)
endif()
if((CORE_WITH_TESTS) AND (UNIX AND NOT APPLE))
    include(InstallGoogleTest)
endif()

