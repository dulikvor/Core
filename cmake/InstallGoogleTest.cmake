if (NOT GOOGLE_TEST_FOUND)

    ExternalProject_Add(GOOGLE_TEST
            DOWNLOAD_NAME       googletest-release-1.8.0.tar.gz
            URL                 https://github.com/google/googletest/archive/release-1.8.0.tar.gz
            CONFIGURE_COMMAND   mkdir <SOURCE_DIR>/build && cd <SOURCE_DIR>/build && cmake -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> ..
            BUILD_COMMAND       cd <SOURCE_DIR>/build && make
            INSTALL_COMMAND     cd <SOURCE_DIR>/build && make install
            TEST_COMMAND        ""
            )

    ExternalProject_Get_Property(GOOGLE_TEST INSTALL_DIR)

    set (GOOGLE_TEST_ROOT_DIR          ${INSTALL_DIR})
    set (GOOGLE_TEST_INCLUDE_DIR       ${GOOGLE_TEST_ROOT_DIR}/include)
    set (GOOGLE_TEST_LIBRARY_DIR       ${GOOGLE_TEST_ROOT_DIR}/lib)
    set (GOOGLE_TEST_FOUND             YES)

endif ()
