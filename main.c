#include "jpge.h"
#include "parser.h"

#include <iterator>
#include <climits>

bool endsWith(std::string const &str, std::string const &suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        return -1;
    }

    std::string fileType = argv[1];
    std::string filePath = argv[2];

    if (fileType == "-caff" && endsWith(filePath, ".caff"))
    {
        parser::CAFF caff;
        if (!parser::parseCaffFile(filePath, caff)) {
            return -1;
        }

        filePath.erase(filePath.length()-5);
        filePath = filePath + ".jpg";

        if (caff.animations[0].ciff.width > INT_MAX || caff.animations[0].ciff.height > INT_MAX) {
            return -1;
        }

        jpge::compress_image_to_jpeg_file(filePath.c_str(), (int)caff.animations[0].ciff.width, (int)caff.animations[0].ciff.height, 3, (jpge::uint8*)(caff.animations[0].ciff.pixels.data()));
    }
    else if (fileType == "-ciff" && endsWith(filePath, ".ciff"))
    {
        parser::CIFF ciff;
        if (!parser::parseCiffFile(filePath, ciff)) {
            return -1;
        }

        filePath.erase(filePath.length()-5);
        filePath = filePath + ".jpg";

        if (ciff.width > INT_MAX || ciff.height > INT_MAX) {
            return -1;
        }

        jpge::compress_image_to_jpeg_file(filePath.c_str(), (int)ciff.width, (int)ciff.height, 3, (jpge::uint8*)(ciff.pixels.data()));
    }
    else
    {
        return -1;
    }

    return 0;
}
