//    Copyright (C) 2013 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#ifndef IMAGE_LOAD_STORE_INTERFACE_H
#define IMAGE_LOAD_STORE_INTERFACE_H

namespace image
{

class ILoadStore
{
public:
    ~ILoadStore() {}

    virtual void LoadFromUri(const std::string& uri) = 0;
    virtual void LoadFromMemory(uint8_t* pData, uint64_t dataSize) = 0;
    virtual void LoadFromMemory(const std::vector<uint8_t>& data) = 0;
    
    virtual void StoreToFile(const std::string& path) = 0;
    virtual std::vector<uint8_t> StoreToMemory() = 0;
};

}

#endif
