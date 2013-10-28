//    Copyright (C) 2014 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include "imageloadstorejpeg.h"
#include <stdexcept>
#include <cassert>
#include <cstring>

#include "utils/log.h"
#include "utils/fileoperations.h"
#include "image/image.h"

using namespace utils;

extern "C"
{
    #include <jpeglib.h>
}

namespace image
{

static void handleFatalError(j_common_ptr cinfo);

struct BufferWriter
{
    jpeg_destination_mgr    destMgr;
    uint8_t*                dataBuffer;
    std::vector<uint8_t>*   dataSink;
};

struct LoadStoreJpegData
{
    enum class Operation
    {
        Compress,
        Decompress
    };

    LoadStoreJpegData(Operation op)
    : operation(op)
    {
        jpeg_std_error(&errorHandler);
        errorHandler.error_exit = handleFatalError;
        errorHandler.output_message = [] (j_common_ptr cinfo) {
            char buffer[JMSG_LENGTH_MAX];
            (*cinfo->err->format_message) (cinfo, buffer);
            log::info(buffer);
        };
    
        if (operation == Operation::Compress)
        {
            jpeg_create_compress(&compression);
            compression.err = &errorHandler;
        }
        else
        {
            jpeg_create_decompress(&decompression);
            decompression.err = &errorHandler;
        }
    }

    ~LoadStoreJpegData()
    {
        if (operation == Operation::Compress)
            jpeg_destroy_compress(&compression);
        else
            jpeg_destroy_decompress(&decompression);
    }

    Operation               operation;
    jpeg_compress_struct    compression;
    jpeg_decompress_struct  decompression;
    jpeg_error_mgr          errorHandler;
};

static constexpr int JPEG_WORK_BUFFER_SIZE = 8192;
static void jpegInitDestination(j_compress_ptr pCompressionInfo);
static boolean jpegFlushWorkBuffer(j_compress_ptr pCompressionInfo);
static void jpegDestroyDestination(j_compress_ptr pCompressionInfo);

LoadStoreJpeg::LoadStoreJpeg()
{
    
}

LoadStoreJpeg::~LoadStoreJpeg() = default;

bool LoadStoreJpeg::isValidImageData(const std::vector<uint8_t>& data)
{
    if (data.size() < 4)
    {
        return false;
    }
    
    const uint32_t* pHeader = reinterpret_cast<const uint32_t*>(data.data());
    return *pHeader == 0xE0FFD8FF;
}

std::unique_ptr<Image> LoadStoreJpeg::loadFromReader(utils::IReader& reader)
{
    return loadFromMemory(reader.readAllData());
}

std::unique_ptr<Image> LoadStoreJpeg::loadFromMemory(const uint8_t* pData, uint64_t dataSize)
{
    std::unique_ptr<Image> image(new Image());
    
    LoadStoreJpegData jpeg(LoadStoreJpegData::Operation::Decompress);
    
    auto& decomp = jpeg.decompression;
    
    jpeg_mem_src(&decomp, const_cast<uint8_t*>(pData), dataSize);
    if (1 != jpeg_read_header(&decomp, TRUE))
    {
        throw std::runtime_error("Invalid JPEG data recieved");
    }
    
    jpeg_start_decompress(&decomp);
	
	image->width        = decomp.output_width;
	image->height       = decomp.output_height;
	image->bitDepth     = decomp.data_precision;
    image->colorPlanes  = decomp.output_components;
    image->data.resize(image->width * image->height * image->colorPlanes);
 
	// Now that you have the decompressor entirely configured, it's time
	// to read out all of the scanlines of the jpeg.
	//
	// By default, scanlines will come out in RGBRGBRGB...  order, 
	// but this can be changed by setting cinfo.out_color_space
	//
	// jpeg_read_scanlines takes an array of buffers, one for each scanline.
	// Even if you give it a complete set of buffers for the whole image,
	// it will only ever decompress a few lines at a time. For best 
	// performance, you should pass it an array with cinfo.rec_outbuf_height
	// scanline buffers. rec_outbuf_height is typically 1, 2, or 4, and 
	// at the default high quality decompression setting is always 1.
	
    JSAMPROW rowPointer[1];
    while (decomp.output_scanline < decomp.output_height)
    {
		rowPointer[0] = (unsigned char*)(&image->data[decomp.output_scanline * decomp.image_width * decomp.output_components]);
 		jpeg_read_scanlines(&decomp, rowPointer, 1);
 
	}
 
	jpeg_finish_decompress(&decomp);
 
    return image;
}

std::unique_ptr<Image> LoadStoreJpeg::loadFromMemory(const std::vector<uint8_t>& data)
{
    return loadFromMemory(data.data(), data.size());
}

void LoadStoreJpeg::storeToFile(const Image& image, const std::string& path)
{
    utils::fileops::writeFile(storeToMemory(image), path);
}

std::vector<uint8_t> LoadStoreJpeg::storeToMemory(const Image& image)
{
    constexpr int quality = 85;

    LoadStoreJpegData jpeg(LoadStoreJpegData::Operation::Compress);
    
    std::vector<uint8_t> jpegData;
    
    auto& comp = jpeg.compression;
    comp.dest = (jpeg_destination_mgr*)(comp.mem->alloc_small) ((j_common_ptr) &comp, JPOOL_PERMANENT, sizeof(BufferWriter));

    BufferWriter* pWriter = reinterpret_cast<BufferWriter*>(comp.dest);
    pWriter->destMgr.init_destination       = jpegInitDestination;
    pWriter->destMgr.empty_output_buffer    = jpegFlushWorkBuffer;
    pWriter->destMgr.term_destination       = jpegDestroyDestination;
    pWriter->dataSink                       = &jpegData;

    comp.image_width         = image.width;
    comp.image_height        = image.height;
    comp.input_components    = image.colorPlanes;
    comp.in_color_space      = JCS_RGB;

    jpeg_set_defaults(&comp);
    jpeg_set_quality(&comp, quality, TRUE);
    jpeg_start_compress(&comp, TRUE);

    JSAMPROW rowPointer[1];
    while (comp.next_scanline < comp.image_height)
    {
        rowPointer[0] = (unsigned char*)(&image.data[comp.next_scanline * comp.image_width * comp.input_components]);
        (void) jpeg_write_scanlines(&comp, rowPointer, 1);
    }

    jpeg_finish_compress(&comp);

    return jpegData;
}

static void jpegInitDestination(j_compress_ptr pCompressionInfo)
{
    BufferWriter* pWriter = reinterpret_cast<BufferWriter*>(pCompressionInfo->dest);

    pWriter->dataBuffer = (uint8_t*)(*pCompressionInfo->mem->alloc_small) ((j_common_ptr) pCompressionInfo, JPOOL_IMAGE, JPEG_WORK_BUFFER_SIZE);
    pWriter->destMgr.next_output_byte = pWriter->dataBuffer;
    pWriter->destMgr.free_in_buffer = JPEG_WORK_BUFFER_SIZE;
}

static boolean jpegFlushWorkBuffer(j_compress_ptr pCompressionInfo)
{
    BufferWriter* pWriter = reinterpret_cast<BufferWriter*>(pCompressionInfo->dest);

    size_t prevSize = pWriter->dataSink->size();
    pWriter->dataSink->resize(prevSize + JPEG_WORK_BUFFER_SIZE);
    memcpy(pWriter->dataSink->data() + prevSize, pWriter->dataBuffer, JPEG_WORK_BUFFER_SIZE);

    pWriter->destMgr.next_output_byte = pWriter->dataBuffer;
    pWriter->destMgr.free_in_buffer = JPEG_WORK_BUFFER_SIZE;

    return TRUE;
}


static void jpegDestroyDestination(j_compress_ptr pCompressionInfo)
{
    BufferWriter* pWriter = reinterpret_cast<BufferWriter*>(pCompressionInfo->dest);
    size_t datacount = JPEG_WORK_BUFFER_SIZE - pWriter->destMgr.free_in_buffer;

    size_t prevSize = pWriter->dataSink->size();
    pWriter->dataSink->resize(prevSize + datacount);
    memcpy(pWriter->dataSink->data() + prevSize, pWriter->dataBuffer, datacount);
}

static void handleFatalError(j_common_ptr cinfo)
{
    (*cinfo->err->output_message) (cinfo);
    throw std::runtime_error("Fatal error decoding jpeg file");
}

}
