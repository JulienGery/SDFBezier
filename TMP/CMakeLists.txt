set(CMAKE_CXX_FLAGS "-O2")

add_library(msdfgen ./Bitmap.h ./Bitmap.hpp
    ./Vector2.h ./Vector2.cpp ./render-sdf.h
    ./render-sdf.cpp ./BitmapRef.hpp ./arithmetics.hpp
    ./pixel-conversion.hpp ./bitmap-interpolation.hpp
    )

