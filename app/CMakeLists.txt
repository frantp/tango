cmake_minimum_required(VERSION 3.4.1)

add_library( tangoc SHARED
             src/main/jni/jni.cpp
             src/main/jni/TangoApp.cpp
             src/main/jni/tango-gl/video_overlay.cc
             src/main/jni/tango-gl/drawable_object.cc
             src/main/jni/tango-gl/shaders.cc
             src/main/jni/tango-gl/transform.cc
             src/main/jni/tango-gl/util.cc )

add_library( tango_client_api SHARED IMPORTED )
set_target_properties( tango_client_api PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libtango_client_api.so )

add_library( tango_support_api SHARED IMPORTED )
set_target_properties( tango_support_api PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libtango_support_api.so )

find_library( GLESv2-lib GLESv2 )

find_library( log-lib log )

include_directories( src/main/include
                     ../external/glm )

target_link_libraries( tangoc
                       tango_client_api
                       tango_support_api
                       ${GLESv2-lib}
                       ${log-lib} )