project(engine VERSION 1.0.0)

if(NOT WIN32)
    pkg_check_modules(FFTW3 IMPORTED_TARGET fftw3)
endif()

add_subdirectory(audio)
add_subdirectory(src)
if(NOT ANDROID AND NOT IOS)
    add_subdirectory(test)
endif()
