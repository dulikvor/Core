if (NOT HIREDIS_FOUND)

    ExternalProject_Add(HIRedis
            DOWNLOAD_NAME       hiredis-0.13.3.tar.gz
            URL                 https://github.com/redis/hiredis/archive/v0.13.3.tar.gz
            CONFIGURE_COMMAND   ""
            BUILD_COMMAND       make -C <SOURCE_DIR>
            INSTALL_COMMAND     make -C <SOURCE_DIR> PREFIX=<INSTALL_DIR> install && cp <SOURCE_DIR>/libhiredis.a <INSTALL_DIR>/lib
            TEST_COMMAND        ""
            )

    ExternalProject_Get_Property(HIRedis INSTALL_DIR)

    set (HIREDIS_ROOT_DIR          ${INSTALL_DIR})
    set (HIREDIS_INCLUDE_DIR       ${HIREDIS_ROOT_DIR}/include)
    set (HIREDIS_LIBRARY_DIR       ${HIREDIS_ROOT_DIR}/lib)
    set (HIREDIS_FOUND             YES)

    #    mark_as_advanced(GLOG_LIBRARY GLOG_INCLUDE_PATH)

endif ()
