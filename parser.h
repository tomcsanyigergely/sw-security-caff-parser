#ifndef PARSER_PARSER_H
#define PARSER_PARSER_H

#include <vector>
#include <cstdint>
#include <string>

namespace parser {
    struct CIFF {
        char magic[4];
        uint64_t header_size;
        uint64_t content_size;
        uint64_t width;
        uint64_t height;
        std::vector<char> pixels;
    };

    struct CAFF_HEADER {
        char magic[4];
        uint64_t header_size;
        uint64_t num_anim;
    };

    struct CAFF_CREDITS {
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        std::string creator;
    };

    struct CAFF_ANIMATION {
        uint64_t duration;
        CIFF ciff;
    };

    struct CAFF {
        CAFF_HEADER header;
        CAFF_CREDITS credits;
        std::vector<CAFF_ANIMATION> animations;
    };

    bool datacopy(void *to, const std::vector<char> &from, uint64_t &pos, uint64_t count);

    bool parseCiff(const std::vector<char> &buffer, uint64_t &pos, CIFF &ciff);

    bool parseCaffHeader(const std::vector<char> &buffer, uint64_t blockLength, uint64_t &pos, CAFF_HEADER &caffHeader);

    bool parseCaffCredits(const std::vector<char> &buffer, uint64_t blockLength, uint64_t &pos, CAFF_CREDITS &caffCredits);

    bool parseCaffAnimation(const std::vector<char> &buffer, uint64_t blockLength, uint64_t &pos, CAFF_ANIMATION &caffAnimation);

    bool parseCiffFile(std::string filePath, CIFF &ciff);

    bool parseCaffFile(std::string filePath, CAFF &caff);
}

#endif //PARSER_PARSER_H
