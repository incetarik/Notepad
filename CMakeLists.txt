cmake_minimum_required(VERSION 3.6)
project(Notepad)

MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()

# Burası C++ derleyici bayrakları, ilk oluşturulduğunda bu vardı. C kullandığımız için pek işe yaramayacak.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static-libgcc -static-libstdc++")
set(CMAKE_C_FLAGS -mwindows)

# Kaynak dosyalarının adlarını bu dosyaya göre buraya yaz. Aynı klasördeyse direkt adı,
# Farklı klasördeyse ../ veya sonrakiKlasör/adı/ile/ilerle

# GTK'nin include adresi, iki klasör geride, bu sende farklıysa burayı değiştir
set(BOINC_INCLUDE_DIR ../../GTK/include)

# GTK'nin include içerisindeki bütün dosyaları alıyor
FILE(GLOB _ALL_FILES ${BOINC_INCLUDE_DIR} ${BOINC_INCLUDE_DIR}/*)


FOREACH(_FILE ${_ALL_FILES})
    IF(IS_DIRECTORY ${_FILE})
        INCLUDE_DIRECTORIES(${_FILE})
    ENDIF()
ENDFOREACH()

# GTK'nin içindeki lib yani dll dosyalarının hepsini alıyor, klasör farklıysa ../../ kısmını değiştir.
set(BOINC_INCLUDE_DIR ../../GTK/lib)

# Ekleme işlemi...
FILE(GLOB _ALL_FILES ${BOINC_INCLUDE_DIR} ${BOINC_INCLUDE_DIR}/*)
FOREACH(_FILE ${_ALL_FILES})
    IF(IS_DIRECTORY ${_FILE})
        INCLUDE_DIRECTORIES(${_FILE})
    ENDIF()
ENDFOREACH()

# Buradan aşağısı, include ediyor. Hepsini kendine göre düzenle ../../ kısımlarını
include_directories(../../GTK/lib/glib-2.0/include)
include_directories(../../GTK/lib/)

# Libraries yani lib klasörünü buraya göster. dll dosyalarını buradan alacak
link_directories(../../GTK/lib)

# Bunu tam adres olarak göster. Dosyamız lib içindeki gdk-win32-3.0.lib
link_libraries(Z:/GTK/lib/gdk-win32-3.0.lib)

# Bütün kalan lib'leri alıyor
file(GLOB libraries "../../GTK/lib/*.lib")
link_libraries(${libraries})

# Bütün kalan dll'leri alıyor
file(GLOB executables "../../GTK/bin/*.dll")
link_libraries(${executables})

# Çıktıları veriyor
set(SOURCE_FILES gtk-notepad.c gtk-notepad.h)
add_executable(Notepad ${SOURCE_FILES})