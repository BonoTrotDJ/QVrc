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

# NullSoft installer files
set(nsis_path "${INSTALLROOT}/${DATADIR}")
if(qmlui)
    set(nsis_files qlcplus5Qt6.nsi)
else()
    set(nsis_files qlcplus4Qt6.nsi)
endif()
install(FILES ${nsis_files} DESTINATION ${nsis_path})

# install(FILES ../Sample.qxw DESTINATION ${INSTALLROOT}/${DATADIR})
