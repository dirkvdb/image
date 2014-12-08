//    Copyright (C) 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "imageloadstorepng.h"
#include <stdexcept>
#include <cassert>
#include <cstring>
#include <png.h>

#include "utils/log.h"
#include "utils/fileoperations.h"
#include "image/image.h"

#include <iostream>

using namespace std;
using namespace utils;

namespace image
{

struct PngReadData
{
    uint64_t        offset = 0;
    uint64_t        dataSize = 0;
    const uint8_t*  data = nullptr;
};

class PngPointers
{
public:
    enum class Operation
    {
        Read,
        Write
    };

    PngPointers(Operation op)
    : operation(op)
    {
        pngPtr = (operation == Operation::Read) ? png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr)
                                                : png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

        if (!pngPtr)
        {
            throw runtime_error("Failed to create png structure");
        }
        
        infoPtr = png_create_info_struct(pngPtr);
        if (!infoPtr)
        {
            if (operation == Operation::Read)
                png_destroy_read_struct(&pngPtr, nullptr, nullptr);
            else
                png_destroy_write_struct(&pngPtr, nullptr);
            
            throw runtime_error("Failed to create png info structure");
        }
    }
    
    ~PngPointers()
    {
        if (operation == Operation::Read)
            png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
        else
            png_destroy_write_struct(&pngPtr, &infoPtr);
    }
    
    operator png_structp() const
    {
        return pngPtr;
    }
    
    operator png_infop() const
    {
        return infoPtr;
    }

private:
    Operation               operation;
    png_structp             pngPtr = nullptr;
	png_infop 	            infoPtr = nullptr;
};

static void readImageProperties(PngPointers& png, Image& image);
static void writeDataCallback(png_structp png_ptr, png_bytep data, png_size_t length);
static void readDataCallback(png_structp png_ptr, png_bytep data, png_size_t length);
static void readDataFromReaderCallback(png_structp png_ptr, png_bytep data, png_size_t length);

constexpr uint32_t PngSignatureLength = 8;

LoadStorePng::~LoadStorePng() = default;

bool LoadStorePng::isValidImageData(const std::vector<uint8_t>& data)
{
    return isValidImageData(data.data(), data.size());
}

bool LoadStorePng::isValidImageData(const uint8_t* pData, uint64_t dataSize)
{
    if (dataSize < 8)
    {
        return false;
    }
    
    return png_sig_cmp(pData, 0, PngSignatureLength) == 0;
}

std::unique_ptr<Image> LoadStorePng::loadFromReader(utils::IReader& reader)
{
    PngPointers png(PngPointers::Operation::Read);
    uint8_t pngSignature[PngSignatureLength];
    auto read = reader.read(pngSignature, PngSignatureLength);
    if (read != PngSignatureLength)
    {
        throw std::runtime_error("Failed to read png header");
    }
    
    verifyPNGSignature(pngSignature);
    png_set_sig_bytes(png, PngSignatureLength);
    
    auto image = std::make_unique<Image>();
    
    png_set_read_fn(png, reinterpret_cast<png_voidp>(&reader), readDataFromReaderCallback);
    readImageProperties(png, *image);
    std::cout << "6 " << image->colorPlanes << std::endl;
    
    std::vector<png_bytep> rowPointers(image->height, nullptr);
    //png_bytep* rowPointers = new png_bytep[image->height];
    for (size_t y = 0; y < image->height; ++y)
    {
        rowPointers[y] = (png_bytep)(&image->data[image->width * y * image->colorPlanes]);
        std::cout << std::hex << rowPointers[y] << std::dec << std::endl;
    }
    
    png_read_image(png, rowPointers.data());
    for (size_t y = 0; y < image->height; ++y)
    {
        std::cout << std::hex << *rowPointers[y] << std::dec << std::endl;
    }

    png_read_end(png, nullptr);
    rowPointers.clear();
    std::cout << "7" << std::endl;
    return image;
}

std::unique_ptr<Image> LoadStorePng::loadFromMemory(const uint8_t* pData, uint64_t dataSize)
{
    PngPointers png(PngPointers::Operation::Read);

    verifyPNGSignature(pData);

    auto image = std::make_unique<Image>();
    
    PngReadData readData;
    readData.dataSize = dataSize;
    readData.data = pData;
    
    png_set_read_fn(png, reinterpret_cast<png_voidp>(&readData), readDataCallback);
    readImageProperties(png, *image);
    
    std::vector<png_bytep> rowPointers(image->height, nullptr);
    for (size_t y = 0; y < image->height; ++y)
    {
        rowPointers[y] = (png_bytep)(&image->data[image->width * y * image->colorPlanes]);
    }
    
    png_read_image(png, rowPointers.data());
    png_read_end(png, nullptr);
    
    return image;
}

std::unique_ptr<Image> LoadStorePng::loadFromMemory(const std::vector<uint8_t>& data)
{
    return loadFromMemory(data.data(), data.size());
}

static int32_t colorTypeFromColorPlanes(uint32_t colorPlanes)
{
    switch (colorPlanes)
    {
    case 4: return PNG_COLOR_TYPE_RGB_ALPHA;
    case 3: return PNG_COLOR_TYPE_RGB;
    case 2: return PNG_COLOR_TYPE_GRAY_ALPHA;
    case 1: return PNG_COLOR_TYPE_GRAY;
    default:
        throw std::runtime_error("Invalid number of dataplanes in image");
    }
}

void LoadStorePng::storeToFile(const Image& image, const std::string& path)
{
    utils::fileops::writeFile(storeToMemory(image), path);
}

std::vector<uint8_t> LoadStorePng::storeToMemory(const Image& image)
{
    PngPointers png(PngPointers::Operation::Write);

    std::vector<uint8_t> pngData;

    png_set_write_fn(png, reinterpret_cast<png_voidp>(&pngData), writeDataCallback, nullptr);

    if (setjmp(png_jmpbuf(png)))
	{
		throw logic_error("Writing png file failed");
	}
	
	png_set_IHDR(png, png, image.width, image.height, image.bitDepth, colorTypeFromColorPlanes(image.colorPlanes),
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    
    std::vector<png_bytep> rowPointers(image.height, nullptr);
    for (size_t y = 0; y < image.height; ++y)
    {
        rowPointers[y] = (png_bytep)(&image.data[image.width * y * image.colorPlanes]);
    }
    
    png_set_rows(png, png, rowPointers.data());
    png_write_png(png, png, 0, nullptr);
    png_write_end(png, nullptr);
    
    return pngData;
}

void LoadStorePng::verifyPNGSignature(const uint8_t* pData)
{
    if (!png_check_sig(pData, PngSignatureLength))
    {
        throw std::runtime_error("Invalid PNG data recieved");
    }
}

static void readImageProperties(PngPointers& png, Image& image)
{
    png_read_info(png, png);
    
    png_uint_32 width   = 0;
    png_uint_32 height  = 0;
    int bitDepth        = 0;
    int colorType       = -1;
    
    if (1 != png_get_IHDR(png, png, &width, &height, &bitDepth, &colorType, nullptr, nullptr, nullptr))
    {
        throw std::runtime_error("Filed to read png header");
    }

    image.width     = width;
    image.height    = height;
    image.bitDepth  = static_cast<uint32_t>(bitDepth);
    
    switch (colorType)
    {
    case PNG_COLOR_TYPE_GRAY:
        image.colorPlanes = 1;
        break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
        image.colorPlanes = 2;
        break;
    case PNG_COLOR_TYPE_RGB:
        image.colorPlanes = 3;
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        image.colorPlanes = 4;
        break;
    case PNG_COLOR_TYPE_PALETTE:
        png_set_palette_to_rgb(png);
        image.colorPlanes = 3;
        break;
    default:
        throw std::runtime_error("Unsupported PNG color type encountered");
    }
    
    // reserve the necessary memory
    std::cout << width << " " << height << " " << bitDepth << " " << image.colorPlanes << " " << width * height * (bitDepth / 8) * image.colorPlanes << std::endl;
    image.data.resize(width * height * (bitDepth / 8) * image.colorPlanes);
}

//void LoadStorePng::setText(const string& key, const string& value)
//{
//	png_text pngText;
//		
//	pngText.compression = -1;
//	pngText.key = const_cast<char*>(key.c_str());
//	pngText.text = const_cast<char*>(value.c_str());
//
//	png_set_text(m_Data->pngPtr, m_Data->infoPtr, &pngText, 1);
//}

void writeDataCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
    vector<uint8_t>& outputBuffer = *(reinterpret_cast<vector<uint8_t>* >(png_get_io_ptr(png_ptr)));
    auto prevBufSize = outputBuffer.size();
    outputBuffer.resize(outputBuffer.size() + length);
    memcpy(&outputBuffer[prevBufSize], data, length);
}

void readDataCallback(png_structp png_ptr, png_bytep data, png_size_t bytesToRead)
{
    PngReadData* pData = reinterpret_cast<PngReadData*>(png_get_io_ptr(png_ptr));
    
    if (pData->offset + bytesToRead > pData->dataSize)
    {
        throw std::runtime_error("PNG reading failed, read outside of buffer");
    }
    
    memcpy(data, pData->data + pData->offset, bytesToRead);
    pData->offset += bytesToRead;
}

void readDataFromReaderCallback(png_structp png_ptr, png_bytep data, png_size_t bytesToRead)
{
    utils::IReader& reader = *(reinterpret_cast<utils::IReader*>(png_get_io_ptr(png_ptr)));
    auto readBytes = reader.read(data, bytesToRead);
    std::cout << readBytes << std::endl;
    if (readBytes != bytesToRead)
    {
        log::error("Error reading png data: invalid number of bytes read (requested = {} actual = {})", bytesToRead, readBytes);
    }
}

}
