glslc.exe ../shaders/linear.comp -O -o ../shaders/linear.spv
glslc.exe ../shaders/quadratic.comp -O -o ../shaders/quadratic.spv
glslc.exe ../shaders/generateImage.comp -O -o ../shaders/tmp.spv
glslc.exe ../shaders/bezier.comp -O -o ../shaders/bezier.spv

pause
