cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread" )
project( video-to-terminal )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable( transcoder transcoder/main.cpp transcoder/videotranscoder.cpp libs/binaryutils.cpp libs/variousutils.cpp )
#target_compile_options( transcoder PRIVATE -pg )
#target_link_options( transcoder PRIVATE -pg )
target_link_libraries( transcoder stdc++fs ${OpenCV_LIBS} )

add_executable( decoder decoder/main.cpp decoder/vtdidecoder.cpp libs/binaryutils.cpp libs/variousutils.cpp libs/termutils.cpp )
#target_compile_options( decoder PRIVATE -pg -fsanitize=address )
#target_link_options( decoder PRIVATE -pg -fsanitize=address )
target_link_libraries( decoder stdc++fs )

add_executable( img-viewer img-viewer/main.cpp img-viewer/imgviewer.cpp libs/variousutils.cpp )
target_link_libraries( img-viewer stdc++fs ${OpenCV_LIBS} )

add_executable (
    media-viewer media-viewer/main.cpp media-viewer/mediaviewer.cpp
    decoder/vtdidecoder.cpp
    libs/termutils.cpp libs/variousutils.cpp libs/binaryutils.cpp libs/_kbhit.cpp
)
target_link_libraries( media-viewer stdc++fs ${OpenCV_LIBS} )
