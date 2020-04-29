#pragma once
#include <memory>
#include <stdexcept>
#include <vector>
#include "../FileLoader/TgaLoader.h"
#include "../FileLoader/DDSLoader.h"
#include "../FileLoader/PngLoader.h"

////////////////////////////////////////////////////////////////////////////////////////////
// texture 파일 경로가 절대경로여서 texture 파일은 따로 불러와야 한다.
// texture가 여러개일 경우에는? 메쉬별로 텍스쳐가 다를 경우?
// 해당 메쉬가 어떤 텍스쳐 리소스를 쓰는지 전부 파악하고
// 별도로 텍스쳐'들'을 관리해야 한다.
////////////////////////////////////////////////////////////////////////////////////////////

struct TextureData
{
    std::unique_ptr<std::vector<std::uint8_t>> Pixels = nullptr;
    std::uint32_t Width, Height, BytesPerPixel;
    TextureData()
    {
        TextureData::Init();
    }
    void Init()
    {
        Pixels = nullptr;
        Width = Height = BytesPerPixel = 0;
    }

    TextureData& operator=(TextureData& other)
    {
        this->Pixels = std::move(other.Pixels);
        this->Width = other.Width;
        this->Height = other.Height;
        this->BytesPerPixel = other.BytesPerPixel;
        return *this;
    }
};

class TextureLoader
{
    TextureData mTextureData;

public:
    TextureLoader() = default;
    TextureLoader(const char* filepath) { LoadTextureFromFile(filepath); }
    bool LoadTextureFromFile(const char* filepath)
    {
        char _Drive[_MAX_DRIVE];
        char _Dir[_MAX_DIR];
        char _Filename[_MAX_FNAME];
        char _Ext[_MAX_EXT];
        _splitpath_s(filepath, _Drive, _Dir, _Filename, _Ext);

        if (strcmp(_Ext, ".dds") == 0)
        {
            DDS ddsLoader(filepath);
            ddsLoader.MovePixels(mTextureData.Pixels);
            mTextureData.Width = ddsLoader.GetWidth();
            mTextureData.Height = ddsLoader.GetHeight();
            mTextureData.BytesPerPixel = 4;
            return true;
        }
        else if (strcmp(_Ext, ".tga") == 0)
        {
            Tga tgaLoader(filepath);
            tgaLoader.MovePixels(mTextureData.Pixels);
            mTextureData.Width = tgaLoader.GetWidth();
            mTextureData.Height = tgaLoader.GetHeight();
            mTextureData.BytesPerPixel = 4;
            return true;
        }
        else if (strcmp(_Ext, ".png") == 0)
        {
            PNG pngLoader(filepath);
            pngLoader.MovePixels(mTextureData.Pixels);
            mTextureData.Width = pngLoader.GetWidth();
            mTextureData.Height = pngLoader.GetHeight();
            mTextureData.BytesPerPixel = 4;
        }
        //else if (strcmp(_Ext, ".bmp") == 0)
        //{
        //    // process bmp
        //}
        else
        {
            throw std::invalid_argument("Invalided Texture file format.");
        }
        return false;
    }

    bool MoveTextureData(TextureData& pOut)
    {
        if (mTextureData.Pixels != nullptr)
        {
            pOut = mTextureData;
            mTextureData.Init();
            return true;
        }
        return false;
    }
};
