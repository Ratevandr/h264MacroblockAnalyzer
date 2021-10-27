cmake_minimum_required(VERSION 3.14)

project(MacroblockAnalyzer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package(...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick LinguistTools REQUIRED)

find_package(OpenCV REQUIRED core highgui imgproc)
include_directories( ${OpenCV_INCLUDE_DIRS} )

## FFMPEG

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h

    /home/landgrafpc/programming/ffmpeg/FFmpeg/install/build/incdir/)
find_library(AVCODEC_LIBRARY
    NAMES avcodec
    NO_DEFAULT_PATH
    PATHS
    /home/landgrafpc/programming/ffmpeg/FFmpeg/install/build/shlibdir/
    )

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h
     /home/landgrafpc/programming/ffmpeg/FFmpeg/install/build/incdir/)
find_library(AVFORMAT_LIBRARY
    NAMES avformat
    NO_DEFAULT_PATH
    PATHS
    /home/landgrafpc/programming/ffmpeg/FFmpeg/install/build/shlibdir/)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h
    /home/landgrafpc/programming/ffmpeg/FFmpeg/install/build/incdir/)
find_library(AVUTIL_LIBRARY
    NAMES avutil
    NO_DEFAULT_PATH
    PATHS
    /home/landgrafpc/programming/ffmpeg/FFmpeg/install/build/shlibdir/)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h
    /home/landgrafpc/programming/ffmpeg/FFmpeg/install/build/incdir/)
find_library(AVDEVICE_LIBRARY
    NAMES avdevice
    NO_DEFAULT_PATH
    PATHS
    /home/landgrafpc/programming/ffmpeg/FFmpeg/install/build/shlibdir/)

find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h
    /home/landgrafpc/programming/ffmpeg/FFmpeg/install/build/incdir/libswresample/)
find_library(SWRESAMPLE_LIBRARY
    NAMES swresample
    NO_DEFAULT_PATH
    PATHS
    /home/landgrafpc/programming/ffmpeg/FFmpeg/install/build/shlibdir/)

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h
    /home/landgrafpc/programming/ffmpeg/FFmpeg/install/build/incdir/libswscale/)
find_library(SWSCALE_LIBRARY
    NAMES swscale
    NO_DEFAULT_PATH
    PATHS
    /home/landgrafpc/programming/ffmpeg/FFmpeg/install/build/shlibdir/)


#find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
#find_library(AVCODEC_LIBRARY
#    NAMES avcodec
#    )

#find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
#find_library(AVFORMAT_LIBRARY
#    NAMES avformat)

#find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
#find_library(AVUTIL_LIBRARY
#    NAMES avutil)

#find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
#find_library(AVDEVICE_LIBRARY
#    NAMES avdevice)



set(TS_FILES MacroblockAnalyzer_en_US.ts)

set (PROJECT_HEADERS
    main.hpp
    framestream.hpp
    opencvimageprovider.hpp
    ffmpegMacroblockdDef.h
    qp_table.h
    video_enc_params.h
   # framenumbermodel.hpp
    )
set(PROJECT_SOURCES
        main.cpp
        framestream.cpp
        opencvimageprovider.cpp
 #       framenumbermodel.cpp
        qp_table.cpp
        qml.qrc

        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MacroblockAnalyzer
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
    )

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(MacroblockAnalyzer SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(MacroblockAnalyzer
          ${PROJECT_SOURCES}
          ${PROJECT_HEADERS}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${AVCODEC_INCLUDE_DIR}
    ${AVFORMAT_INCLUDE_DIR}
    ${AVUTIL_INCLUDE_DIR}
    ${AVDEVICE_INCLUDE_DIR}
    ${SWRESAMPLE_INCLUDE_DIR}
    ${SWSCALE_INCLUDE_DIR})

target_compile_definitions(MacroblockAnalyzer
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(MacroblockAnalyzer
  PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Quick
  ${OpenCV_LIBS}
  ${AVCODEC_LIBRARY}
  ${AVFORMAT_LIBRARY}
  ${AVUTIL_LIBRARY}
  ${AVDEVICE_LIBRARY}
  ${SWRESAMPLE_LIBRARY}
  ${SWSCALE_LIBRARY}
  )
