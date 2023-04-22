glslc.exe ../shaders/case2/solve.comp -O -o ../shaders/out/solve2.spv
glslc.exe ../shaders/case2/buildCoef.comp -O -o ../shaders/out/buildCoef2.spv
glslc.exe ../shaders/case2/final.comp -O -o ../shaders/out/final2.spv

glslc.exe ../shaders/case3/solve.comp -O -o ../shaders/out/solve3.spv
glslc.exe ../shaders/case3/init.comp -O -o ../shaders/out/init3.spv
glslc.exe ../shaders/case3/buildCoef.comp -O -o ../shaders/out/buildCoef3.spv
glslc.exe ../shaders/case3/final.comp -O -o ../shaders/out/final3.spv

glslc.exe ../shaders/case4/solve.comp -O -o ../shaders/out/solve4.spv
glslc.exe ../shaders/case4/init.comp -O -o ../shaders/out/init4.spv
glslc.exe ../shaders/case4/buildCoef.comp -O -o ../shaders/out/buildCoef4.spv
glslc.exe ../shaders/case4/final.comp -O -o ../shaders/out/final4.spv
pause
