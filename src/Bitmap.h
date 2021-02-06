//
// Created by alien on 2/3/21.
//

#ifndef IMAGEINVERTER_BITMAP_H
#define IMAGEINVERTER_BITMAP_H

#include <cstdint>

namespace imginv {

#pragma pack(push, 2)

    struct FileHeader {
        char bm[2];
        int32_t sizeFile;
        int32_t reserved;
        int32_t offset;
    };

    struct InfoHeader {
        int32_t sizeHeader;
        int32_t width;
        int32_t height;
        int16_t planes;
        int16_t bitsPerPixel;
        int32_t compression;
        int32_t sizeData;
        int32_t horizontalResolution;
        int32_t verticalResolution;
        int32_t colors;
        int32_t importantColors;
    };


#pragma pack(pop)

}


#endif //IMAGEINVERTER_BITMAP_H
