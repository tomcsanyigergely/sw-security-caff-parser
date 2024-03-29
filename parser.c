#include "parser.h"

#include <cstring>
#include <fstream>

namespace parser {
    bool datacopy(void *to, const std::vector<char> &from, uint64_t &pos, uint64_t count) {
        if (count > 0) {
            uint64_t startingPos = pos;

            if (count > SIZE_MAX) {
                return false;
            }

            uint64_t length = from.size();

            if (count > UINT64_MAX - pos || pos + count > length) {
                return false;
            }

            if (to != nullptr) {
                std::memcpy(to, from.data() + pos, count);
            }

            pos += count;

            if (pos < startingPos) {
                return false;
            }
        }

        return true;
    }

    bool parseCiff(const std::vector<char> &buffer, uint64_t &pos, CIFF &ciff) {
        uint64_t startingPos = pos;

        if (!datacopy(ciff.magic, buffer, pos, sizeof(ciff.magic))) {
            printf("Failed to read CIFF magic.\n");
            return false;
        }

        if (ciff.magic[0] != 'C' || ciff.magic[1] != 'I' || ciff.magic[2] != 'F' || ciff.magic[3] != 'F') {
            printf("Invalid CIFF magic.\n");
            return false;
        }

        if (!datacopy(&ciff.header_size, buffer, pos, sizeof(ciff.header_size))) {
            printf("Failed to read CIFF header_size.\n");
            return false;
        }

        if (ciff.header_size <
            sizeof(ciff.magic) + sizeof(ciff.header_size) + sizeof(ciff.content_size) + sizeof(ciff.width) +
            sizeof(ciff.height)) {
            printf("Invalid CIFF header_size.\n");
            return false;
        }

        if (!datacopy(&ciff.content_size, buffer, pos, sizeof(ciff.content_size))) {
            printf("Failed to read CIFF content_size.\n");
            return false;
        }

        if (!datacopy(&ciff.width, buffer, pos, sizeof(ciff.width))) {
            printf("Failed to read CIFF width.\n");
            return false;
        }

        if (!datacopy(&ciff.height, buffer, pos, sizeof(ciff.height))) {
            printf("Failed to read CIFF height.\n");
            return false;
        }

        if (ciff.width * ciff.height * 3 != ciff.content_size ||
            ciff.width > ciff.content_size ||
            ciff.height > ciff.content_size ||
            ciff.width * ciff.height > UINT64_MAX / 3 ||
            ciff.width > UINT64_MAX / ciff.height / 3 ||
            ciff.height > UINT64_MAX / ciff.width / 3) {
            printf("Invalid CIFF content_size.\n");
            return false;
        }

        pos = startingPos;

        if (!datacopy(nullptr, buffer, pos, ciff.header_size)) {
            printf("Unexpected error while parsing CIFF.\n");
            return false;
        }

        if (ciff.content_size > SIZE_MAX) {
            printf("CIFF content_size is too large.\n");
            return false;
        }

        ciff.pixels.resize(ciff.content_size);

        if (!datacopy(ciff.pixels.data(), buffer, pos, ciff.content_size)) {
            printf("Error while parsing CIFF pixels.\n");
            return false;
        }

        return true;
    }

    bool parseCaffHeader(const std::vector<char> &buffer, uint64_t blockLength, uint64_t &pos, CAFF_HEADER &caffHeader) {
        uint64_t startingPos = pos;

        if (!datacopy(caffHeader.magic, buffer, pos, sizeof(caffHeader.magic))) {
            printf("Failed to read magic in CAFF header.\n");
            return false;
        }

        if (caffHeader.magic[0] != 'C' || caffHeader.magic[1] != 'A' || caffHeader.magic[2] != 'F' ||
            caffHeader.magic[3] != 'F') {
            printf("Invalid magic in CAFF header.\n");
            return false;
        }

        if (!datacopy(&caffHeader.header_size, buffer, pos, sizeof(caffHeader.header_size))) {
            printf("Failed to read header_size in CAFF header.\n");
            return false;
        }

        if (caffHeader.header_size !=
            sizeof(caffHeader.magic) + sizeof(caffHeader.header_size) + sizeof(caffHeader.num_anim)) {
            printf("Invalid header_size in CAFF header.\n");
            return false;
        }

        if (blockLength != caffHeader.header_size) {
            printf("CAFF header block length differs from header_size.\n");
            return false;
        }

        if (!datacopy(&caffHeader.num_anim, buffer, pos, sizeof(caffHeader.num_anim))) {
            printf("Failed to read num_anim in CAFF header.\n");
            return false;
        }

        pos = startingPos;

        if (!datacopy(nullptr, buffer, pos, blockLength)) {
            printf("Unexpected error while parsing CAFF header.\n");
            return false;
        }

        return true;
    }

    bool parseCaffCredits(const std::vector<char> &buffer, uint64_t blockLength, uint64_t &pos, CAFF_CREDITS &caffCredits) {
        uint64_t startingPos = pos;

        if (!datacopy(&caffCredits.year, buffer, pos, sizeof(caffCredits.year))) {
            printf("Failed to read year in CAFF credits.\n");
            return false;
        }

        if (!datacopy(&caffCredits.month, buffer, pos, sizeof(caffCredits.month))) {
            printf("Failed to read month in CAFF credits.\n");
            return false;
        }

        if (!datacopy(&caffCredits.day, buffer, pos, sizeof(caffCredits.day))) {
            printf("Failed to read day in CAFF credits.\n");
            return false;
        }

        if (!datacopy(&caffCredits.hour, buffer, pos, sizeof(caffCredits.hour))) {
            printf("Failed to read hour in CAFF credits.\n");
            return false;
        }

        if (!datacopy(&caffCredits.minute, buffer, pos, sizeof(caffCredits.minute))) {
            printf("Failed to read minute in CAFF credits.\n");
            return false;
        }

        uint64_t creator_len;

        if (!datacopy(&creator_len, buffer, pos, sizeof(creator_len))) {
            printf("Failed to read creator_len in CAFF credits.\n");
            return false;
        }

        uint64_t caffCreditsFixedPartSize = sizeof(caffCredits.year) +
                                        sizeof(caffCredits.month) +
                                        sizeof(caffCredits.day) +
                                        sizeof(caffCredits.hour) +
                                        sizeof(caffCredits.minute) +
                                        sizeof(creator_len);

        if (creator_len > UINT64_MAX - caffCreditsFixedPartSize) {
            printf("Invalid creator_len in CAFF credits (too large).\n");
            return false;
        }

        if (caffCreditsFixedPartSize + creator_len != blockLength) {
            printf("CAFF credits block length differs from the size of its content.\n");
            return false;
        }

        if (creator_len > SIZE_MAX - 1) {
            printf("Invalid creator_len in CAFF credits (too large).\n");
            return false;
        }

        caffCredits.creator.resize(creator_len + 1);

        if (!datacopy((void *) caffCredits.creator.data(), buffer, pos, creator_len)) {
            printf("Failed to read creator in CAFF credits.\n");
            return false;
        }

        caffCredits.creator[creator_len] = '\0';

        pos = startingPos;

        if (!datacopy(nullptr, buffer, pos, blockLength)) {
            printf("Unexpected error while parsing CAFF credits.\n");
            return false;
        }

        return true;
    }

    bool parseCaffAnimation(const std::vector<char> &buffer, uint64_t blockLength, uint64_t &pos, CAFF_ANIMATION &caffAnimation) {
        uint64_t startingPos = pos;

        if (!datacopy(&caffAnimation.duration, buffer, pos, sizeof(caffAnimation.duration))) {
            printf("Failed to read duration in CAFF animation.\n");
            return false;
        }

        if (!parseCiff(buffer, pos, caffAnimation.ciff)) {
            printf("Failed to parse CIFF in CAFF animation.\n");
            return false;
        }

        pos = startingPos;

        if (!datacopy(nullptr, buffer, pos, blockLength)) {
            printf("Unexpected error while parsing CAFF animation.\n");
            return false;
        }

        return true;
    }

    bool parseCaffFile(std::string filePath, CAFF &caff) {
        std::ifstream file;
        file.open(filePath, std::ifstream::in | std::ifstream::binary);

        if (!file) {
            printf("Failed to open CAFF file.\n");
            return false;
        }

        std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

        file.close();

        uint64_t pos = 0;

        uint8_t id;
        uint64_t blockLength;

        if (!datacopy(&id, buffer, pos, sizeof(id))) {
            printf("Failed to read first block ID in CAFF file.\n");
            return false;
        }

        if (!datacopy(&blockLength, buffer, pos, sizeof(blockLength))) {
            printf("Failed to read first block length in CAFF file.\n");
            return false;
        }

        if (id != 0x1) {
            printf("Invalid first block ID in CAFF file (must be 0x1).\n");
            return false;
        }

        if (!parseCaffHeader(buffer, blockLength, pos, caff.header)) {
            printf("Failed to parse CAFF header in CAFF file.\n");
            return false;
        }

        if (!datacopy(&id, buffer, pos, sizeof(id))) {
            printf("Failed to read second block ID in CAFF file.\n");
            return false;
        }

        if (!datacopy(&blockLength, buffer, pos, sizeof(blockLength))) {
            printf("Failed to read second block length in CAFF file.\n");
            return false;
        }

        if (id == 0x2) {
            if (!parseCaffCredits(buffer, blockLength, pos, caff.credits)) {
                printf("Failed to parse CAFF credits in CAFF file.\n");
                return false;
            }
        } else {
            pos -= sizeof(id) + sizeof(blockLength);
        }

        for (uint64_t i = 0; i < caff.header.num_anim; i++) {
            if (!datacopy(&id, buffer, pos, sizeof(id))) {
                printf("Failed to read animation block ID in CAFF file.\n");
                return false;
            }

            if (!datacopy(&blockLength, buffer, pos, sizeof(blockLength))) {
                printf("Failed to read animation block length in CAFF file.\n");
                return false;
            }

            if (id != 0x3) {
                printf("Invalid animation block ID in CAFF file (must be 0x3).\n");
                return false;
            }

            CAFF_ANIMATION caffAnimation;

            if (!parseCaffAnimation(buffer, blockLength, pos, caffAnimation)) {
                printf("Failed to parse CAFF animation in CAFF file.\n");
                return false;
            }

            caff.animations.push_back(caffAnimation);
        }

        return true;
    }

    bool parseCiffFile(std::string filePath, CIFF &ciff) {
        std::ifstream file;
        file.open(filePath, std::ifstream::in | std::ifstream::binary);

        if (!file) {
            printf("Failed to open CIFF file.\n");
            return false;
        }

        std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

        file.close();

        uint64_t pos = 0;

        if (!parseCiff(buffer, pos, ciff)) {
            printf("Failed to parse CIFF file content.\n");
            return false;
        }

        return true;
    }
}
