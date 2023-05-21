#include "parser.h"

#include <cstring>
#include <fstream>

namespace parser {
    bool datacopy(void *to, const std::vector<char> &from, uint64_t &pos, uint64_t count) {
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

        return true;
    }

    bool parseCiff(const std::vector<char> &buffer, uint64_t &pos, CIFF &ciff) {
        uint64_t startingPos = pos;

        if (!datacopy(ciff.magic, buffer, pos, sizeof(ciff.magic))) {
            return false;
        }

        if (ciff.magic[0] != 'C' || ciff.magic[1] != 'I' || ciff.magic[2] != 'F' || ciff.magic[3] != 'F') {
            return false;
        }

        if (!datacopy(&ciff.header_size, buffer, pos, sizeof(ciff.header_size))) {
            return false;
        }

        if (ciff.header_size <
            sizeof(ciff.magic) + sizeof(ciff.header_size) + sizeof(ciff.content_size) + sizeof(ciff.width) +
            sizeof(ciff.height)) {
            return false;
        }

        if (!datacopy(&ciff.content_size, buffer, pos, sizeof(ciff.content_size))) {
            return false;
        }

        if (!datacopy(&ciff.width, buffer, pos, sizeof(ciff.width))) {
            return false;
        }

        if (!datacopy(&ciff.height, buffer, pos, sizeof(ciff.height))) {
            return false;
        }

        if (ciff.width * ciff.height * 3 != ciff.content_size ||
            ciff.width > ciff.content_size ||
            ciff.height > ciff.content_size ||
            ciff.width * ciff.height > UINT64_MAX / 3 ||
            ciff.width > UINT64_MAX / ciff.height / 3 ||
            ciff.height > UINT64_MAX / ciff.width / 3) {
            return false;
        }

        pos = startingPos;

        if (!datacopy(nullptr, buffer, pos, ciff.header_size)) {
            return false;
        }

        if (ciff.content_size > SIZE_MAX) {
            return false;
        }

        ciff.pixels.resize(ciff.content_size);

        if (!datacopy(ciff.pixels.data(), buffer, pos, ciff.content_size)) {
            return false;
        }

        return true;
    }

    bool parseCaffHeader(const std::vector<char> &buffer, uint64_t blockLength, uint64_t &pos, CAFF_HEADER &caffHeader) {
        uint64_t startingPos = pos;

        if (!datacopy(caffHeader.magic, buffer, pos, sizeof(caffHeader.magic))) {
            return false;
        }

        if (caffHeader.magic[0] != 'C' || caffHeader.magic[1] != 'A' || caffHeader.magic[2] != 'F' ||
            caffHeader.magic[3] != 'F') {
            return false;
        }

        if (!datacopy(&caffHeader.header_size, buffer, pos, sizeof(caffHeader.header_size))) {
            return false;
        }

        if (caffHeader.header_size !=
            sizeof(caffHeader.magic) + sizeof(caffHeader.header_size) + sizeof(caffHeader.num_anim)) {
            return false;
        }

        if (!datacopy(&caffHeader.num_anim, buffer, pos, sizeof(caffHeader.num_anim))) {
            return false;
        }

        pos = startingPos;

        if (!datacopy(nullptr, buffer, pos, blockLength)) {
            return false;
        }

        return true;
    }

    bool parseCaffCredits(const std::vector<char> &buffer, uint64_t blockLength, uint64_t &pos, CAFF_CREDITS &caffCredits) {
        uint64_t startingPos = pos;

        if (!datacopy(&caffCredits.year, buffer, pos, sizeof(caffCredits.year))) {
            return false;
        }

        if (!datacopy(&caffCredits.month, buffer, pos, sizeof(caffCredits.month))) {
            return false;
        }

        if (!datacopy(&caffCredits.day, buffer, pos, sizeof(caffCredits.day))) {
            return false;
        }

        if (!datacopy(&caffCredits.hour, buffer, pos, sizeof(caffCredits.hour))) {
            return false;
        }

        if (!datacopy(&caffCredits.minute, buffer, pos, sizeof(caffCredits.minute))) {
            return false;
        }

        uint64_t creator_len;

        if (!datacopy(&creator_len, buffer, pos, sizeof(creator_len))) {
            return false;
        }

        if (creator_len > SIZE_MAX - 1) {
            return false;
        }

        caffCredits.creator.resize(creator_len + 1);

        if (!datacopy((void*)caffCredits.creator.data(), buffer, pos, creator_len)) {
            return false;
        }

        caffCredits.creator[creator_len] = '\0';

        pos = startingPos;

        if (!datacopy(nullptr, buffer, pos, blockLength)) {
            return false;
        }

        return true;
    }

    bool parseCaffAnimation(const std::vector<char> &buffer, uint64_t blockLength, uint64_t &pos, CAFF_ANIMATION &caffAnimation) {
        uint64_t startingPos = pos;

        if (!datacopy(&caffAnimation.duration, buffer, pos, sizeof(caffAnimation.duration))) {
            return false;
        }

        if (!parseCiff(buffer, pos, caffAnimation.ciff)) {
            return false;
        }

        pos = startingPos;

        if (!datacopy(nullptr, buffer, pos, blockLength)) {
            return false;
        }

        return true;
    }

    bool parseCaffFile(std::string filePath, CAFF &caff) {
        std::ifstream file;
        file.open(filePath, std::ifstream::in | std::ifstream::binary);

        if (!file) {
            return false;
        }

        std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

        uint64_t pos = 0;

        uint8_t id;
        uint64_t blockLength;

        if (!datacopy(&id, buffer, pos, sizeof(id))) {
            return false;
        }

        if (!datacopy(&blockLength, buffer, pos, sizeof(blockLength))) {
            return false;
        }

        if (id != 0x1) {
            return false;
        }

        if (!parseCaffHeader(buffer, blockLength, pos, caff.header)) {
            return false;
        }

        if (!datacopy(&id, buffer, pos, sizeof(id))) {
            return false;
        }

        if (!datacopy(&blockLength, buffer, pos, sizeof(blockLength))) {
            return false;
        }

        if (id == 0x2) {
            if (!parseCaffCredits(buffer, blockLength, pos, caff.credits)) {
                return false;
            }
        } else {
            pos -= sizeof(id) + sizeof(blockLength);
        }

        for (uint64_t i = 0; i < caff.header.num_anim; i++) {
            if (!datacopy(&id, buffer, pos, sizeof(id))) {
                return false;
            }

            if (!datacopy(&blockLength, buffer, pos, sizeof(blockLength))) {
                return false;
            }

            if (id != 0x3) {
                return false;
            }

            CAFF_ANIMATION caffAnimation;

            if (!parseCaffAnimation(buffer, blockLength, pos, caffAnimation)) {
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
            return false;
        }

        std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});

        uint64_t pos = 0;

        if (!parseCiff(buffer, pos, ciff)) {
            return false;
        }

        return true;
    }
}
