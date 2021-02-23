#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <getopt.h>

#include "Bitmap.h"

#define no_argument 0
#define required_argument 1
#define optional_argument 2

using namespace std;
using namespace imginv;

bool verbose = false;

int invertBMP(string inputPath, string outputPath);

void
showHeaders(const shared_ptr<FileHeader> &ptrFileHeader, const shared_ptr<InfoHeader> &ptrInfoHeader);

int verifyHeaders(const string &inputPath, const shared_ptr<FileHeader> &ptrFileHeader,
                  shared_ptr<InfoHeader> &ptrInfoHeader, ifstream &is);

int verifyFileHeader(const shared_ptr<FileHeader> &ptrFileHeader, const ifstream &is);

int main(int argc, char **argv) {
    const std::string usage = "USAGE\n"
                              "  imginv [OPTIONS] FILE\n";
    const static struct option long_options[] = {
            {"output",  required_argument, 0, 'o'},
            {"verbose", no_argument,       0, 'v'},
            {"help",    no_argument,       0, 'h'},
    };

    const std::string help = "NAME\n"
                             "  imginv -- creates a negative of a photo from the input photo without changing the original photo. Supported photo format(s): Bitmap.\n"
                             "USAGE\n"
                             "  imginv [OPTIONS] FILE\n"
                             "OPTIONS\n"
                             "  -o, --output    the name of the output photo negative file\n"
                             "  -v, --verbose   displays additional information while running the program\n"
                             "  -h, --help      displays help and exit\n";
    bool flagExit = false;
    std::string inputPath, outputPath;

    while (true) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "vo:h", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'o':
                outputPath = optarg;
                break;

            case 'h':
                if (argc == 2)
                    cout << help;
                else
                    cout << usage;
                flagExit = true;
                break;

            case 'v':
                verbose = true;
                break;

            default:
                cout << usage;
                flagExit = true;
        }
    }

    if (flagExit)
        exit(EXIT_SUCCESS);

    inputPath = static_cast<string>(argv[optind++]);
    if (outputPath.empty())
        outputPath = inputPath + "_negative.bmp";

    auto ok = invertBMP(inputPath, outputPath);

    if (ok == -1) {
        cout << "Could not invert this photo" << endl;
    } else {
        cout << "Successfully inverted " << inputPath << " to " << outputPath << " !!!" << endl;
    }
    if (optind < argc)
        if (verbose) cout << "Discarded non-option ARGV-elements\n" << usage;

    return 0;
}

bool isOverflow(int a, int b) {
    int c = a * b;
    return a != c / b;
}

int invertBMP(string inputPath, string outputPath) {
    shared_ptr<FileHeader> ptrFileHeader{nullptr};
    shared_ptr<InfoHeader> ptrInfoHeader{nullptr};
    uint8_t *optional_header_buffer;
    uint8_t *buffer;
    int rowWidth;
    bool hasOptionalHeader = false;

    ifstream is{inputPath, ios::binary};
    if (is and is.is_open()) {
        ptrFileHeader = std::make_shared<FileHeader>();
        ptrInfoHeader = std::make_shared<InfoHeader>();

        if (verbose) cout << "Opened " << inputPath << endl;

        is.read(reinterpret_cast<char *>(ptrFileHeader.get()), sizeof(FileHeader));

        int ok = verifyFileHeader(ptrFileHeader, is);

        if(ok!=0){
            is.close();
            return -1;
        }

        is.read(reinterpret_cast<char *>(ptrInfoHeader.get()), sizeof(InfoHeader));

        ok = verifyHeaders(inputPath, ptrFileHeader, ptrInfoHeader, is);

        if(ok!=0){
            is.close();
            return -1;
        }


        if (verbose) showHeaders(ptrFileHeader, ptrInfoHeader);

        if(is.tellg() < ptrFileHeader->offset){
            int32_t cur = is.tellg();
            is.seekg(0, ios::end);
            int32_t last = is.tellg();
            int32_t size = static_cast<int32_t >(last - ptrFileHeader->offset);
            is.seekg(cur);
            optional_header_buffer = new uint8_t[size];
            is.read(reinterpret_cast<char *>(optional_header_buffer), size);
            if(is.fail()){
                is.close();
                cout << "Could not read optional headers" << endl;
                return -1;
            }
            hasOptionalHeader = true;
        }

        rowWidth = ptrInfoHeader->sizeData / abs(ptrInfoHeader->height);

        is.seekg(ptrFileHeader->offset);
        if (verbose) cout << "Reading Image buffer..." << endl;
        buffer = new uint8_t[ptrInfoHeader->sizeData];
        is.read(reinterpret_cast<char *>(buffer), ptrInfoHeader->sizeData);

        if (verbose) cout << "Creating negative bmp..." << endl;
        for (int i = 0; i < abs(ptrInfoHeader->height); ++i) {
            for (int j = 0; j < ptrInfoHeader->width; ++j) {
                for (int k = 0; k < 3; ++k) {
                    *(buffer + i * rowWidth + 3 * j + k) =
                            static_cast<uint8_t>(255 - *(buffer + i * rowWidth + 3 * j + k));
                }
            }
        }
        is.close();
        if (verbose) cout << "Closed Image " << inputPath << endl;
    } else {
        if (verbose) cout << "Could not open " << inputPath << endl;
        return -1;
    }

    if (verbose) cout << "Writing neagtive image..." << endl;

    ofstream os{outputPath, ios::binary};
    if (os and os.is_open()) {
        os.write(reinterpret_cast<char *>(ptrFileHeader.get()), sizeof(FileHeader));

        if(os.fail()){
            cout << "Could not write " << outputPath <<  " FileHeader" << endl;
            os.close();
            delete buffer;
            return -1;
        }

        os.write(reinterpret_cast<char *>(ptrInfoHeader.get()), sizeof(InfoHeader));

        if(os.fail()){
            cout << "Could not write " << outputPath <<  " InfoHeader" << endl;
            os.close();
            delete buffer;
            return -1;
        }

        if(hasOptionalHeader){
            os.write(reinterpret_cast<char *>(optional_header_buffer), sizeof(optional_header_buffer));
        }

        os.write(reinterpret_cast<char *>(buffer), ptrInfoHeader->sizeData);

        if(os.fail()){
            cout << "Could not write " << outputPath <<  " negative image" << endl;
            os.close();
            delete buffer;
            return -1;
        }

        os.close();
    } else {
        cout << "Could not open " << outputPath << endl;
        os.close();
        delete buffer;
        return -1;
    }

    if(hasOptionalHeader)
        delete optional_header_buffer;
    delete buffer;
    return 0;
}

int verifyFileHeader(const shared_ptr<FileHeader> &ptrFileHeader, const ifstream &is) {
    if (verbose) cout << "Verifying Image headers..." << endl;
    if (is.fail()) {
        cout << "Could not read file header" << endl;
            return -1;
    }

    if (ptrFileHeader->bm[0] != 'B' or ptrFileHeader->bm[1] != 'M') {
        cout << "BMP file header must start with BM" << endl;
            return -1;
    }
    return 0;
}

int verifyHeaders(const string &inputPath, const shared_ptr<FileHeader> &ptrFileHeader,
                  shared_ptr<InfoHeader> &ptrInfoHeader, ifstream &is) {
    if (is.fail()) {
        cout << "Could not read info header" << endl;
        return -1;
    }

    if (ptrFileHeader->offset < sizeof(FileHeader) + sizeof(InfoHeader)) {
        cout << "Pixel buffer block offset started before the ending of headers."
                "Current version of this program don't support this" << endl;
        return -1;
    }

    // even bmp files from good sources may have missing pixel array size information in the header. So try to get
    // this information by calculating manually.
    if (ptrInfoHeader->sizeData <= 0) {
        // can not use ptrFileHeader->fileSize - ptrFileHeader->offset because there may be headers other than
        // File and Info header i.e. color map header
        int32_t cur = is.tellg();
        is.seekg(0, ios::end);
        int32_t last = is.tellg();
        int32_t size = static_cast<int32_t >(last - ptrFileHeader->offset);
        is.seekg(cur);
        if (size > 0 and size % 4 == 0 and size % abs(ptrInfoHeader->height) == 0) {
            ptrInfoHeader->sizeData = static_cast<int32_t >(size);
        } else {
            cout << "Missing file size information in headers" << endl;
            return -1;
        }
    }


    if (abs(ptrInfoHeader->height) <= 0 or ptrInfoHeader->sizeData <= 0 or ptrInfoHeader->width <= 0) {
        if (verbose)
            cout << inputPath << "width(" << ptrInfoHeader->width << "), absolute of height(|"
                 << ptrInfoHeader->height
                 << "|) and size " << ptrInfoHeader->sizeData << "  are expected to be positive." << endl;
        return -1;
    }

    // Data size of the bmp image must be multiple of both 4 and height of the pixel array
    if (ptrInfoHeader->sizeData % 4 != 0 or ptrInfoHeader->sizeData % abs(ptrInfoHeader->height) != 0) {
        if (verbose) cout << inputPath << "Data size of the bmp image is not consistent with pixel resolution" << endl;
        return -1;
    }

    // Data size of the bmp image must be at least (|height| x width) unless
    // (|height| x width) causes overflow either way program should stop
    if (isOverflow(abs(ptrInfoHeader->height), ptrInfoHeader->width) or
        abs(ptrInfoHeader->height) * ptrInfoHeader->width > ptrInfoHeader->sizeData) {
        cout << inputPath << "Data size of the bmp image is not consistent with pixel resolution" << endl;
        return -1;
    }


    if (ptrInfoHeader->planes != 1) {
        cout << "The number of color planes (must be 1) " << endl;
        return -1;
    }

    if (ptrInfoHeader->bitsPerPixel != 24) {
        cout << "Current version of this program only supports 24-bit bmp file" << endl;
        return -1;
    }

    if (ptrInfoHeader->compression != 0) {
        if (verbose) cout << "Current version of this program does not support compression" << endl;
        return -1;
    }

    return 0;
}

void
showHeaders(const shared_ptr<FileHeader> &ptrFileHeader, const shared_ptr<InfoHeader> &ptrInfoHeader) {
    cout << "------FILE HEADER------" << endl;
    cout << "bm             " << ptrFileHeader->bm[0] << ptrFileHeader->bm[1] << endl;
    cout << "sizeFile       " << ptrFileHeader->sizeFile << endl;
    cout << "reserved       " << ptrFileHeader->reserved << endl;
    cout << "offset         " << ptrFileHeader->offset << endl;
    cout << "------IFO HEADER-------" << endl;
    cout << "sizeHeader             " << ptrInfoHeader->sizeHeader << endl;
    cout << "width                  " << ptrInfoHeader->width << endl;
    cout << "height                 " << ptrInfoHeader->height << endl;
    cout << "planes                 " << ptrInfoHeader->planes << endl;
    cout << "bitsPerPixel           " << ptrInfoHeader->bitsPerPixel << endl;
    cout << "compression            " << ptrInfoHeader->compression << endl;
    cout << "sizeData               " << ptrInfoHeader->sizeData << endl;
    cout << "horizontalResolution   " << ptrInfoHeader->horizontalResolution << endl;
    cout << "verticalResolution     " << ptrInfoHeader->verticalResolution << endl;
    cout << "colors                 " << ptrInfoHeader->colors << endl;
    cout << "importantColors        " << ptrInfoHeader->importantColors << endl;
}
