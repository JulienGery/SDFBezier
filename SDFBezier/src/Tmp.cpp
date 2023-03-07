//#include "Tmp.h"
//
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <iostream>
//
//#define FT_CURVE_TAG_ON 0x01
//
//
//template<class T>
//std::vector<T> readFile(const std::string& path) {
//    std::ifstream file{ path, std::ios::binary | std::ios::ate };
//
//    const size_t size = file.tellg();
//    std::vector<T> buffer(size / sizeof(T));
//    file.seekg(0);
//    file.read((char*)buffer.data(), size);
//    file.close();
//    return buffer;
//}
//
//std::vector<uint16_t> GetIndexes(const uint16_t& start, const uint16_t& end, const std::vector<uint8_t>& flags)
//{
//    std::vector<uint16_t> indexes{ start };
//    uint16_t i = start;
//
//    for (size_t n = start; n < end + 1; n++)
//    {
//        i++;
//        if (i > end)
//            i = start;
//        indexes.push_back(i);
//
//        if (flags[i] & FT_CURVE_TAG_ON)
//            break;
//    }
//
//    return indexes;
//}
//
//std::vector<Bezier> buildCurves()
//{
//    const std::vector<float> points = readFile<float>("/Users/pincable/source/repos/tmp/points.bin");
//    const std::vector<uint8_t> flags = readFile<uint8_t>("/Users/pincable/source/repos/tmp/on.bin");
//    const std::vector<uint16_t> contour = readFile<uint16_t>("/Users/pincable/source/repos/tmp/contour.bin");
//    const size_t pointsCount = flags.size();
//
//    
//    std::vector<Bezier> curves;
//    
//
//    for (const auto& i : curves)
//        std::cout << i.getPointCount() << ' ';
//    std::cout << '\n';
//
//    return curves;
//}
