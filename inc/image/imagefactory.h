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

#ifndef IMAGE_READER_FACTORY_H
#define IMAGE_READER_FACTORY_H

#include <string>
#include <vector>
#include <memory>

namespace image
{

enum class Type
{
    Jpeg,
    Png
};

class Image;
class ILoadStore;

class Factory
{
public:
    static std::unique_ptr<ILoadStore> createLoadStore(Type imageType);

    static std::unique_ptr<Image> createFromUri(const std::string& uri);
    static std::unique_ptr<Image> createFromUri(const std::string& uri, Type imageType);
    
    static std::unique_ptr<Image> createFromData(const std::vector<uint8_t>& data);
    static std::unique_ptr<Image> createFromData(const uint8_t* pData, uint64_t dataSize);
    static std::unique_ptr<Image> createFromData(const std::vector<uint8_t>& data, Type imageType);
    static std::unique_ptr<Image> createFromData(const uint8_t* pData, uint64_t dataSize, Type imageType);
};

}

#endif
