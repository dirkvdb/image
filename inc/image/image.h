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

#ifndef IMAGE_IMAGE_H
#define IMAGE_IMAGE_H

#include <vector>
#include <stdexcept>
#include <cinttypes>

namespace image
{

enum class ResizeAlgorithm
{
    NearestNeighbor,
    Bilinear
};

class Image
{
    struct Pixel
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

public:
    void resize(uint32_t newWidth, uint32_t newHeight, ResizeAlgorithm algo)
    {
        if (data.empty())
        {
            throw std::runtime_error("Failed to resize image, no data present");
        }
        
        if (bitDepth != 8)
        {
            throw std::runtime_error("Resizing is only supported for images with a bitdepth of 8");
        }

        std::vector<uint8_t> resizedData(newWidth * newHeight * colorPlanes);

        if (algo == ResizeAlgorithm::NearestNeighbor)
        {
            double scaleWidth   = static_cast<double>(newWidth) / static_cast<double>(width);
            double scaleHeight  = static_cast<double>(newHeight) / static_cast<double>(height);
        
            if (colorPlanes == 3)
            {
                for (uint32_t y = 0; y < newHeight; ++y)
                {
                    for (uint32_t x = 0; x < newWidth; ++x)
                    {
                        int pixel = (y * (newWidth * 3)) + (x * 3);
                        int nearestMatch = (((int)(y / scaleHeight) * (width * 3)) + ((int)(x / scaleWidth) * 3));
                        
                        resizedData[pixel    ] =  data[nearestMatch    ];
                        resizedData[pixel + 1] =  data[nearestMatch + 1];
                        resizedData[pixel + 2] =  data[nearestMatch + 2];
                    }
                }
            }
        }
        else
        {
            if (colorPlanes == 3)
            {
                const uint32_t planes = colorPlanes;
                const uint32_t stride = (width * planes);
            
                for (uint32_t y = 0; y < newHeight; ++y)
                {
                    const float gy = (float(y) / newHeight) * height - 0.5f;
                    const float fy = gy - uint32_t(gy);
                    const float fy1 = 1.0f - fy;
                    const uint32_t gyi = uint32_t(gy) * stride;
                    
                    for (uint32_t x = 0; x < newWidth; ++x)
                    {
                        const float gx = (float(x) / newWidth) * width - 0.5f;
                        const float fx = gx - uint32_t(gx);
                        const float fx1 = 1.0f - fx;
                        const uint32_t gxi = uint32_t(gx) * planes;
                        
                        const Pixel* p1 = reinterpret_cast<Pixel*>(&data[gyi + gxi]);
                        const Pixel* p2 = reinterpret_cast<Pixel*>(&data[gyi + gxi + planes]);
                        const Pixel* p3 = reinterpret_cast<Pixel*>(&data[(gyi + stride) + gxi]);
                        const Pixel* p4 = reinterpret_cast<Pixel*>(&data[(gyi + stride) + gxi + planes]);
                        
                        // Calculate the weights for each pixel
                        const uint32_t w1 = fx1 * fy1 * 256.0f;
                        const uint32_t w2 = fx  * fy1 * 256.0f;
                        const uint32_t w3 = fx1 * fy  * 256.0f;
                        const uint32_t w4 = fx  * fy  * 256.0f;

                        // Calculate the weighted sum of pixels (for each color channel)
                        Pixel* result = reinterpret_cast<Pixel*>(&resizedData[(y * newWidth * planes) + (x * planes)]);
                        result->r = (p1->r * w1 + p2->r * w2 + p3->r * w3 + p4->r * w4) >> 8;
                        result->g = (p1->g * w1 + p2->g * w2 + p3->g * w3 + p4->g * w4) >> 8;
                        result->b = (p1->b * w1 + p2->b * w2 + p3->b * w3 + p4->b * w4) >> 8;
                        if (planes == 4)
                        {
                            result->a = (p1->a * w1 + p2->a * w2 + p3->a * w3 + p4->a * w4) >> 8;
                        }
                    }
                }
            }

        }
        
        data    = resizedData;
        width   = newWidth;
        height  = newHeight;
    }


    uint32_t                width = 0;
    uint32_t                height = 0;
    uint32_t                bitDepth = 0;
    uint32_t                colorPlanes = 0;
    
    std::vector<uint8_t>    data;
};

}

#endif
