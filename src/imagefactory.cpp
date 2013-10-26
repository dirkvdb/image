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

#include "utils/log.h"
#include "utils/stringoperations.h"
#include "utils/fileoperations.h"

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

Image* Factory::createFromUri(const std::string& uri)
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

Image* Factory::createFromUri(const std::string& uri, Type imageType)
{
    switch (imageType)
    {
    case Type::Jpeg:
#ifdef HAVE_JPEG
        return LoadStoreJpeg();
#else
        throw std::runtime_error("Library not compiled with jpeg support");
#endif
    case Type::Png:
#ifdef HAVE_PNG
#else
        throw std::runtime_error("Library not compiled with png support");
#endif

    default:
        assert(!"This is not possible");
        break;
}

Image* Factory::createFromData(const std::vector<uint8_t>& data, Type imageType)
{
}

}
   
