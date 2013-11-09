//    Copyright (C) 2009 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include "image/imagefactory.h"

#include <stdexcept>
#include <cassert>
#include <exception>

#include "utils/log.h"
#include "utils/stringoperations.h"
#include "utils/fileoperations.h"
#include "utils/readerfactory.h"

#include "imageconfig.h"

#ifdef HAVE_JPEG
    #include "imageloadstorejpeg.h"
#endif

#ifdef HAVE_PNG
    #include "imageloadstorepng.h"
#endif

using namespace utils;

namespace image
{

template <typename LoaderType>
static std::unique_ptr<Image> loadImage(utils::IReader& reader)
{
    LoaderType loader;
    return loader.loadFromReader(reader);
}

template <typename LoaderType>
static std::unique_ptr<Image> loadImageFromMemory(const uint8_t* pData, uint64_t dataSize)
{
    LoaderType loader;
    return loader.loadFromMemory(pData, dataSize);
}

static Type detectImageTypeFromUri(const std::string& uri)
{
    std::string extension = fileops::getFileExtension(uri);
    stringops::lowercase(extension);

    if (extension == "jpg" || extension == "jpeg")
    {
        return Type::Jpeg;
    }
    
    if (extension == "png")
    {
        return Type::Png;
    }

    throw std::runtime_error("Failed to detect file type from extenstion");
}

std::unique_ptr<ILoadStore> Factory::createLoadStore(Type imageType)
{
    switch (imageType)
    {
    case Type::Jpeg:
#ifdef HAVE_JPEG
        return std::unique_ptr<ILoadStore>(new LoadStoreJpeg());
#else
        throw std::runtime_error("Library not compiled with jpeg support");
#endif
    case Type::Png:
#ifdef HAVE_PNG
        return std::unique_ptr<ILoadStore>(new LoadStorePng());
#else
        throw std::runtime_error("Library not compiled with png support");
#endif

    default:
        assert(!"This is not possible");
        throw std::runtime_error("Unexpected image type");
        break;
    }
}

std::unique_ptr<Image> Factory::createFromUri(const std::string& uri)
{
    try
    {
        return createFromUri(uri, detectImageTypeFromUri(uri));
    }
    catch (std::exception& e)
    {
        // TODO: detect filetype base on file contents
        log::warn(e.what());
        std::rethrow_exception(std::current_exception());
    }
}

std::unique_ptr<Image> Factory::createFromUri(const std::string& uri, Type imageType)
{
    std::unique_ptr<utils::IReader> reader(ReaderFactory::create(uri));
    reader->open(uri);

    switch (imageType)
    {
    case Type::Jpeg:
#ifdef HAVE_JPEG
        return loadImage<LoadStoreJpeg>(*reader);
#else
        throw std::runtime_error("Library not compiled with jpeg support");
#endif
    case Type::Png:
#ifdef HAVE_PNG
        return loadImage<LoadStorePng>(*reader);
#else
        throw std::runtime_error("Library not compiled with png support");
#endif

    default:
        assert(!"This is not possible");
        throw std::runtime_error("Unexpected image type");
        break;
    }
}

std::unique_ptr<Image> Factory::createFromData(const std::vector<uint8_t>& data)
{
    return createFromData(data.data(), data.size());
}

std::unique_ptr<Image> Factory::createFromData(const uint8_t* pData, uint64_t dataSize)
{
    // try to detect the image type from the provided data
    
#ifdef HAVE_JPEG
    LoadStoreJpeg loadStoreJpeg;
    if (loadStoreJpeg.isValidImageData(pData, dataSize))
    {
        return loadStoreJpeg.loadFromMemory(pData, dataSize);
    }
#endif

#ifdef HAVE_PNG
    LoadStorePng loadStorePng;
    if (loadStorePng.isValidImageData(pData, dataSize))
    {
        return loadStorePng.loadFromMemory(pData, dataSize);
    }
#endif

    throw std::runtime_error("Provided image data not supported");
}

std::unique_ptr<Image> Factory::createFromData(const std::vector<uint8_t>& data, Type imageType)
{
    return createFromData(data.data(), data.size());
}

std::unique_ptr<Image> Factory::createFromData(const uint8_t* pData, uint64_t dataSize, Type imageType)
{
    switch (imageType)
    {
    case Type::Jpeg:
#ifdef HAVE_JPEG
        return loadImageFromMemory<LoadStoreJpeg>(pData, dataSize);
#else
        throw std::runtime_error("Library not compiled with jpeg support");
#endif
    case Type::Png:
#ifdef HAVE_PNG
        return loadImageFromMemory<LoadStorePng>(pData, dataSize);
#else
        throw std::runtime_error("Library not compiled with png support");
#endif

    default:
        assert(!"This is not possible");
        throw std::runtime_error("Unexpected image type");
        break;
    }
}

}
   
