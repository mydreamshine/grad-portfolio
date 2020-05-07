#include <memory>
#include <fstream>
#include <vector>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// DDS struct
///////////////////////////////////////////////////////////////////////////////

// from ddraw.h
#define DDSD_CAPS               0x00000001l     // default
#define DDSD_HEIGHT             0x00000002l
#define DDSD_WIDTH              0x00000004l
#define DDSD_PITCH              0x00000008l
#define DDSD_PIXELFORMAT        0x00001000l
#define DDSD_LINEARSIZE         0x00080000l

#define DDSPF_ALPHAPIXELS        0x00000001l
#define DDSPF_FOURCC             0x00000004l
#define DDSPF_RGB                0x00000040l

#ifndef _WINDEF_
typedef unsigned long DWORD;
#endif

struct DDPIXELFORMAT
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    DWORD dwRGBBitCount;
    DWORD dwRBitMask;
    DWORD dwGBitMask;
    DWORD dwBBitMask;
    DWORD dwABitMask;
};

struct DDS_HEADER
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    DWORD dwPitchOrLinearSize;
    DWORD dwDepth;
    DWORD dwMipMapCount;
    DWORD dwReserved1[11];
    DDPIXELFORMAT ddspfPixelFormat;
    DWORD dwCaps;
    DWORD dwCaps2;
    DWORD dwCaps3;
    DWORD dwCaps4;
    DWORD dwReserved2;
};


///////////////////////////////////////////////////////////////////////////////
// load dds file
///////////////////////////////////////////////////////////////////////////////
class DDS
{
private:
    std::unique_ptr<std::vector<std::uint8_t>> Pixels = nullptr;
    bool ImageCompressed;
    std::uint32_t width, height, size, BitsPerPixel;

public:
    DDS(const char* FilePath)
    {
        std::ifstream hFile(FilePath, std::ios::in | std::ios::binary);
        if (!hFile.is_open()) { throw std::invalid_argument("File Not Found."); }

        DDS_HEADER header;
        std::vector<std::uint8_t> ImageData;

        hFile.read(reinterpret_cast<char*>(&header), sizeof(header));

        ImageCompressed = false;
        if (header.ddspfPixelFormat.dwFlags & DDSPF_RGB)
        {
            BitsPerPixel = (std::uint32_t)header.ddspfPixelFormat.dwRGBBitCount;
            width = header.dwWidth;
            height = header.dwHeight;
            size = ((width * BitsPerPixel + 31) / 32) * 4 * height;

            size_t pitch;
            if (header.dwFlags & DDSD_PITCH)
                pitch = header.dwPitchOrLinearSize;
            else
                pitch = header.dwWidth * header.ddspfPixelFormat.dwRGBBitCount / 8;

            if ((BitsPerPixel != 24) && (BitsPerPixel != 32))
            {
                hFile.close();
                throw std::invalid_argument("Invalid File Format. Required: 24 or 32 Bit Image.");
            }
            else
            {
                if (header.ddspfPixelFormat.dwRBitMask != 0x00ff0000
                    || header.ddspfPixelFormat.dwGBitMask != 0x0000ff00
                    || header.ddspfPixelFormat.dwBBitMask != 0x000000ff
                    || ((header.ddspfPixelFormat.dwFlags & DDSPF_ALPHAPIXELS)
                        && header.ddspfPixelFormat.dwABitMask != 0xff000000))
                {
                    hFile.close();
                    throw std::invalid_argument("Invalid Pixel Format. Required: RGB_BitMask == 0x00FFFFFF or Alpha_BitMask == 0xFF000000");
                }
                else
                {
                    Pixels = std::make_unique<std::vector<std::uint8_t>>(size);
                    std::uint8_t* dst = Pixels->data();
                    std::vector<std::uint8_t> pitchBuffer(pitch);
                    for (size_t y = 0; y < header.dwHeight; ++y) {
                        hFile.read(reinterpret_cast<char*>(pitchBuffer.data()), pitch);
                        std::uint8_t* src = pitchBuffer.data();
                        for (size_t x = 0; x < header.dwWidth; ++x, dst += 4, src += 4) {
                            dst[0] = src[2]; // R
                            dst[1] = src[1]; // G
                            dst[2] = src[0]; // B
                            dst[3] = src[3]; // A
                        }
                    }
                }
            }
        }
        else if (header.ddspfPixelFormat.dwFlags & DDSPF_FOURCC)
        {
            ImageCompressed = true;
            std::string invalid_message("DDS File is compressed. ");
            invalid_message += "Compress Value: ";
            invalid_message += std::to_string(header.ddspfPixelFormat.dwFourCC);
            throw std::invalid_argument(invalid_message.c_str());
        }
        else
        {
            throw std::invalid_argument("Invalid File Header.");
        }
    }

    std::vector<std::uint8_t>& GetPixels() { return *this->Pixels.get(); }
    void MovePixels(std::unique_ptr<std::vector<std::uint8_t>>& pixels) { pixels = std::move(this->Pixels); }
    std::uint32_t GetWidth() const { return this->width; }
    std::uint32_t GetHeight() const { return this->height; }
    std::uint32_t GetBitPerPixel() const { return this->BitsPerPixel; }
    bool HasAlphaChannel() { return BitsPerPixel == 32; }
};
