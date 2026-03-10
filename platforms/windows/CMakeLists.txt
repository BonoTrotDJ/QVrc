project(windows)

# Set paths
if (DEFINED $ENV{MSYSTEM_CARCH} AND $ENV{MSYSTEM_CARCH} STREQUAL "i686")
    set(SYS_LIBS_PATH $ENV{SystemDrive}/msys64/mingw32/bin)
else()
    set(SYS_LIBS_PATH $ENV{SystemDrive}/msys64/mingw64/bin)
endif()

function(copy_system_library target libname)
	if(EXISTS ${SYS_LIBS_PATH}/${libname})
		list(APPEND ${target} ${SYS_LIBS_PATH}/${libname})
		set(${target} "${${target}}" PARENT_SCOPE)
    else()
        message(WARNING "Missing optional system DLL: ${libname} (searched in ${SYS_LIBS_PATH})")
	endif()
endfunction()

# MSYS2 libraries
set(msys_path "${INSTALLROOT}/${LIBSDIR}")
copy_system_library(msys_files "libusb-1.0.dll")
if(msys_files)
    install(FILES ${msys_files} DESTINATION ${msys_path})
endif()

# audio libraries
set(mmedia_path "${INSTALLROOT}/${LIBSDIR}")
copy_system_library(mmedia_files "libogg-0.dll")
copy_system_library(mmedia_files "libopus-0.dll")
copy_system_library(mmedia_files "libmp3lame-0.dll")
copy_system_library(mmedia_files "libmpg123-0.dll")
copy_system_library(mmedia_files "libvorbis-0.dll")
copy_system_library(mmedia_files "libvorbisenc-2.dll")
copy_system_library(mmedia_files "libFLAC.dll")
copy_system_library(mmedia_files "libsndfile-1.dll")
copy_system_library(mmedia_files "libfftw3-3.dll")
if(mmedia_files)
    install(FILES ${mmedia_files} DESTINATION ${mmedia_path})
endif()

if(SNDFILE_RUNTIME_DLL AND EXISTS "${SNDFILE_RUNTIME_DLL}")
    install(FILES "${SNDFILE_RUNTIME_DLL}" DESTINATION ${INSTALLROOT}/${BINDIR})
endif()

# Qt Multimedia FFmpeg backend runtime for Windows video playback
if(QT_VERSION_MAJOR GREATER 5 AND CMAKE_PREFIX_PATH)
    set(qt_prefix_path "${CMAKE_PREFIX_PATH}")
    if(qt_prefix_path MATCHES ";")
        list(GET CMAKE_PREFIX_PATH 0 qt_prefix_path)
    endif()

    get_filename_component(qt_prefix_path "${qt_prefix_path}" ABSOLUTE)
    get_filename_component(qt_root_path "${qt_prefix_path}/../.." ABSOLUTE)
    set(qt_bin_path "${qt_root_path}/bin")

    set(qt_ffmpeg_runtime_files)
    foreach(ffmpeg_dll
            avcodec-60.dll
            avformat-60.dll
            avutil-58.dll
            swresample-4.dll
            swscale-7.dll)
        if(EXISTS "${qt_bin_path}/${ffmpeg_dll}")
            list(APPEND qt_ffmpeg_runtime_files "${qt_bin_path}/${ffmpeg_dll}")
        else()
            message(WARNING "Missing Qt FFmpeg runtime DLL: ${ffmpeg_dll} (searched in ${qt_bin_path})")
        endif()
    endforeach()

    if(qt_ffmpeg_runtime_files)
        install(FILES ${qt_ffmpeg_runtime_files} DESTINATION ${INSTALLROOT}/${BINDIR})
    endif()
endif()

# NullSoft installer files
set(nsis_path "${INSTALLROOT}/${DATADIR}")
if(qmlui)
    set(nsis_files qlcplus5Qt6.nsi)
else()
    set(nsis_files qlcplus4Qt6.nsi)
endif()
install(FILES ${nsis_files} DESTINATION ${nsis_path})

# install(FILES ../Sample.qxw DESTINATION ${INSTALLROOT}/${DATADIR})
