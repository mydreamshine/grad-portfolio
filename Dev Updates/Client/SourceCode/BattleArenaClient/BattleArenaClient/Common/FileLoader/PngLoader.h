#pragma once
#include <stdio.h>
#include <memory>
#include <vector>
#include <stdexcept>

#include "../Util/libpng/src/png.h"

class PNG
{
private:
    std::unique_ptr<std::vector<std::uint8_t>> Pixels;
    std::uint32_t width, height, size, BitsPerPixel;

public:
    PNG(const char* FilePath)
    {
        FILE* fp = NULL;
        if (fopen_s(&fp, FilePath, "rb") != 0) { throw std::invalid_argument("File Not Found."); }

        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png) abort();

        png_infop info = png_create_info_struct(png);
        if (!info) abort();

        if (setjmp(png_jmpbuf(png))) abort();

        png_init_io(png, fp);

        png_read_info(png, info);

        width = png_get_image_width(png, info);
        height = png_get_image_height(png, info);
        png_byte color_type = png_get_color_type(png, info);
        png_byte bit_depth = png_get_bit_depth(png, info);
        png_byte channels = png_get_channels(png, info);

        // Read any color_type into 8bit depth, RGBA format.
        // See http://www.libpng.org/pub/png/libpng-manual.txt

        BitsPerPixel = 8 * channels;

        if (bit_depth == 16)
            png_set_strip_16(png);

        if (color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png);

        // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_expand_gray_1_2_4_to_8(png);

        if (png_get_valid(png, info, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png);

        // These color_type don't have an alpha channel then fill it with 0xff.
        if (color_type == PNG_COLOR_TYPE_RGB ||
            color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

        if (color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png);

        png_read_update_info(png, info);

        png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
        size_t row_pitch = width * channels;

        for (std::uint32_t y = 0; y < height; ++y)
        {
            if (row_pitch != png_get_rowbytes(png, info))
                throw std::invalid_argument("The row pitch of the png file is not constant.");
            row_pointers[y] = (png_byte*)malloc(row_pitch);
        }
        png_read_image(png, row_pointers);

        Pixels = std::make_unique<std::vector<std::uint8_t>>();
        auto& ImageData = *Pixels;
        ImageData.resize(height * row_pitch);
        for (std::uint32_t y = height - 1; y > 0; --y) {
            png_bytep row = row_pointers[y];
            for (std::uint32_t x = 0; x < width; ++x) {
                png_bytep px = &(row[x * 4]);
                size_t CurrentByte = ((height - 1) - y) * row_pitch + x * channels;
                size_t B = CurrentByte + 0;
                size_t G = CurrentByte + 1;
                size_t R = CurrentByte + 2;
                size_t A = CurrentByte + 3;
                ImageData[B] = px[0];
                ImageData[G] = px[1];
                ImageData[R] = px[2];
                ImageData[A] = px[3];
                // Do something awesome for each pixel here...
                //printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", x, y, px[0], px[1], px[2], px[3]);
            }
        }

        for (std::uint32_t y = 0; y < height; ++y)
            delete[] row_pointers[y];
        delete[] row_pointers;

        fclose(fp);

        png_destroy_read_struct(&png, &info, NULL);
    }
    std::vector<std::uint8_t>& GetPixels() { return *(this->Pixels.get()); }
    void MovePixels(std::unique_ptr<std::vector<std::uint8_t>>& pixels) { pixels = std::move(this->Pixels); }
    std::uint32_t GetWidth() const { return this->width; }
    std::uint32_t GetHeight() const { return this->height; }
    std::uint32_t GetBitPerPixel() const { return this->BitsPerPixel; }
    bool HasAlphaChannel() { return BitsPerPixel == 32; }
};