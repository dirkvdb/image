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

#include <iostream>

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
                    double gy = (double(y) / double(newHeight)) * double(height);
                
                    for (uint32_t x = 0; x < newWidth; ++x)
                    {
                        double gx = (double(x) / double(newWidth)) * double(width);
                        
                        const int gxi = int(gx);
                        const int gyi = int(gy) * stride;
                        
                        const Pixel* c00 = reinterpret_cast<Pixel*>(&data[gyi + (gxi * planes)]);
                        const Pixel* c10 = reinterpret_cast<Pixel*>(&data[gyi + (gxi * planes + planes)]);
                        const Pixel* c01 = reinterpret_cast<Pixel*>(&data[(gyi + stride) + (gxi * planes)]);
                        const Pixel* c11 = reinterpret_cast<Pixel*>(&data[(gyi + stride) + (gxi * planes + planes)]);
                        
                        const double tx = gx - double(gxi);
                        const double ty = gy - double(gyi / stride);
                        
                        const double txi = 1.0 - tx;
                        const double tyi = 1.0 - ty;
                        
                        Pixel a, b;
                        a.r = (c00->r * txi + c10->r * tx) * tyi;
                        a.g = (c00->g * txi + c10->g * tx) * tyi;
                        a.b = (c00->b * txi + c10->b * tx) * tyi;
                        b.r = (c01->r * txi + c11->r * tx) * ty;
                        b.g = (c01->g * txi + c11->g * tx) * ty;
                        b.b = (c01->b * txi + c11->b * tx) * ty;
                        
                        Pixel* result = reinterpret_cast<Pixel*>(&resizedData[(y * newWidth * planes) + (x * planes)]);
                        result->r = a.r + b.r;
                        result->g = a.g + b.g;
                        result->b = a.b + b.b;
                        
                        if (planes == 4)
                        {
                            a.a = (c00->a * txi + c10->a * tx) * tyi;
                            b.a = (c01->a * txi + c11->a * tx) * ty;
                            result->a = a.a + b.a;
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
