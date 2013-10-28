//    Copyright (C) 2012 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include "gtest/gtest.h"

#include <array>
#include <iostream>

#include "utils/fileoperations.h"

#include "imagetestconfig.h"
#include "image/imagefactory.h"
#include "image/imageloadstoreinterface.h"

using namespace utils;
using namespace testing;

namespace image
{
namespace test
{

static const std::string g_jpegTestData = IMAGE_TEST_DATA_DIR "/frog.jpg";
static const std::string g_pngTestData = IMAGE_TEST_DATA_DIR "/frog.png";
static const std::string g_corruptData = IMAGE_TEST_DATA_DIR "/corrupt.jpg";

static const std::string g_testJpegFile = "imageloadingtestfile.jpg";
static const std::string g_testPngFile = "imageloadingtestfile.png";

class ImageLoadingTest : public Test
{
public:
    ImageLoadingTest()
    {
    }

protected:
    void SetUp()
    {
        // make sure all temp files are gone
        try { fileops::deleteFile(g_testJpegFile); } catch (std::exception&) {}
        try { fileops::deleteFile(g_testPngFile); } catch (std::exception&) {}
    }
    
    void TearDown()
    {
        //EXPECT_NO_THROW(fileops::deleteFile(g_testJpegFile));
        //EXPECT_NO_THROW(fileops::deleteFile(g_testPngFile));
    }
};

TEST_F(ImageLoadingTest, dataValidation)
{
    auto jpegData = fileops::readFile(g_jpegTestData);
    auto pngData = fileops::readFile(g_pngTestData);
    
    auto jpegStore = Factory::createLoadStore(Type::Jpeg);
    auto pngStore = Factory::createLoadStore(Type::Png);
    
    EXPECT_TRUE(jpegStore->isValidImageData(jpegData));
    EXPECT_TRUE(pngStore->isValidImageData(pngData));
    
    EXPECT_FALSE(jpegStore->isValidImageData(pngData));
    EXPECT_FALSE(pngStore->isValidImageData(jpegData));
}

TEST_F(ImageLoadingTest, loadJpeg)
{
    auto image = Factory::createFromUri(g_jpegTestData);
    
    auto jpegStore = Factory::createLoadStore(Type::Jpeg);
    jpegStore->storeToFile(*image, "JpegSource" + g_testJpegFile);
    
    auto pngStore = Factory::createLoadStore(Type::Png);
    pngStore->storeToFile(*image, "JpegSource" + g_testPngFile);
}

TEST_F(ImageLoadingTest, loadPng)
{
    auto image = Factory::createFromUri(g_pngTestData);
    
    auto jpegStore = Factory::createLoadStore(Type::Jpeg);
    jpegStore->storeToFile(*image, "PngSource" + g_testJpegFile);
    
    auto pngStore = Factory::createLoadStore(Type::Png);
    pngStore->storeToFile(*image, "PngSource" + g_testPngFile);
}

TEST_F(ImageLoadingTest, loadCorruptJpeg)
{
    EXPECT_THROW(Factory::createFromUri(g_corruptData), std::runtime_error);
}

TEST_F(ImageLoadingTest, resizeNeirestNeightbor)
{
    auto jpegData = fileops::readFile(g_jpegTestData);

    auto image = Factory::createFromData(jpegData);
    image->resize(1700, 1100, ResizeAlgorithm::NearestNeighbor);
    
    auto jpegStore = Factory::createLoadStore(Type::Jpeg);
    jpegStore->storeToFile(*image, "ResizedNearest" + g_testJpegFile);
}

TEST_F(ImageLoadingTest, resizeNeirestBilinear)
{
    auto jpegData = fileops::readFile(g_jpegTestData);

    auto image = Factory::createFromData(jpegData);
    image->resize(1700, 1100, ResizeAlgorithm::Bilinear);
    
    auto jpegStore = Factory::createLoadStore(Type::Jpeg);
    jpegStore->storeToFile(*image, "ResizedBilinear" + g_testJpegFile);
}

}
}
