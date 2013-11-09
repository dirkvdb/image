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

#ifndef IMAGE_LOAD_STORE_PNG_H
#define IMAGE_LOAD_STORE_PNG_H

#include <memory>
#include <vector>
#include <string>

#include "utils/readerinterface.h"
#include "image/imageloadstoreinterface.h"

namespace image
{

class LoadStorePng : public ILoadStore
{
public:
    LoadStorePng() = default;
    virtual ~LoadStorePng();

    LoadStorePng(const LoadStorePng&) = delete;
    LoadStorePng& operator=(const LoadStorePng&) = delete;
    
    virtual bool isValidImageData(const std::vector<uint8_t>& data) override;
    virtual bool isValidImageData(const uint8_t* pData, uint64_t dataSize) override;

    virtual std::unique_ptr<Image> loadFromReader(utils::IReader& reader) override;
    virtual std::unique_ptr<Image> loadFromMemory(const uint8_t* pData, uint64_t dataSize) override;
    virtual std::unique_ptr<Image> loadFromMemory(const std::vector<uint8_t>& data) override;
    
    virtual void storeToFile(const Image& image, const std::string& path) override;
    virtual std::vector<uint8_t> storeToMemory(const Image& image) override;
    
    // Png specific operation
    // void setText(const std::string& key, const std::string& value);
    
private:
    void verifyPNGSignature(const uint8_t* pData);
};

}

#endif
