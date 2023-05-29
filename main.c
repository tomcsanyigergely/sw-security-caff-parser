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
        printf("Usage: parser [-caff | -ciff] path-to-file \n");
        return -1;
    }

    std::string fileType = argv[1];
    std::string filePath = argv[2];

    if (fileType == "-caff" && endsWith(filePath, ".caff"))
    {
        parser::CAFF caff;
        if (!parser::parseCaffFile(filePath, caff)) {
            printf("Failed to parse CAFF file.\n");
            return -1;
        }

        filePath.erase(filePath.length()-5);
        filePath = filePath + ".jpg";

        if (caff.animations[0].ciff.width > INT_MAX || caff.animations[0].ciff.height > INT_MAX) {
            printf("Error while saving JPG: First CIFF image size too large.\n");
            return -1;
        }

        if (!jpge::compress_image_to_jpeg_file(filePath.c_str(), (int)caff.animations[0].ciff.width, (int)caff.animations[0].ciff.height, 3, (jpge::uint8*)(caff.animations[0].ciff.pixels.data()))) {
            printf("Unexpected error while saving first CIFF image as JPG.\n");
            return -1;
		}
    }
    else if (fileType == "-ciff" && endsWith(filePath, ".ciff"))
    {
        parser::CIFF ciff;
        if (!parser::parseCiffFile(filePath, ciff)) {
            printf("Failed to parse CIFF file.\n");
            return -1;
        }

        filePath.erase(filePath.length()-5);
        filePath = filePath + ".jpg";

        if (ciff.width > INT_MAX || ciff.height > INT_MAX) {
            printf("Error while saving JPG: CIFF image size too large.\n");
            return -1;
        }

        if (!jpge::compress_image_to_jpeg_file(filePath.c_str(), (int)ciff.width, (int)ciff.height, 3, (jpge::uint8*)(ciff.pixels.data()))) {
            printf("Unexpected error while saving CIFF image as JPG.\n");
            return -1;
		}
    }
    else
    {
        printf("Usage: parser [-caff | -ciff] path-to-file \n");
        return -1;
    }

    return 0;
}
