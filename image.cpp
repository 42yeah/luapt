// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize.h>


int BaseImage::image_id_counter = 0;

template<>
bool U8Image::save(const std::string &dest) const
{
    assert(initialized && "Image is not initialized");

    stbi_flip_vertically_on_write(true);
    int res = stbi_write_png(dest.c_str(), w, h, ch, image.get(), w * ch);

    return res != 0;
}

template<>
bool U8Image::save_compressed(const std::string &dest, int quality) const
{
    assert(initialized && "Image is not initialized");

    stbi_flip_vertically_on_write(true);
    int res = stbi_write_jpg(dest.c_str(), w, h, ch, image.get(), quality);

    return res != 0;
}

template<>
bool U8Image::load(const std::string &path)
{
    stbi_set_flip_vertically_on_load(true);

    unsigned char *data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    assert((ch == 3 || ch == 4) && "Unsupported number of channels");

    if (!data)
    {
        return false;
    }

    image.reset(data);
    initialized = true;

    return true;
}

template<>
bool U8Image::resize(int nx, int ny)
{
    // 1. Allocate needed amount of memory
    std::unique_ptr<unsigned char[]> ptr(new unsigned char[nx * ny * ch]);
    int ok = stbir_resize_uint8(image.get(), w, h, 0, ptr.get(), nx, ny, 0, ch);

    if (ok == 0)
    {
        return false;
    }

    w = nx;
    h = ny;
    image = std::move(ptr);
    return true;
}

template<>
RGB<unsigned char> Image<unsigned char>::get_rgb(int x, int y) const
{
    assert(initialized && "Image is not initialized");

    RGB<unsigned char> ret;
    ret.r = image[at(x, y) + 0];
    ret.g = image[at(x, y) + 1];
    ret.b = image[at(x, y) + 2];
    return ret;
}

template<>
RGB<float> Image<unsigned char>::get_rgb_float(int x, int y) const
{
    assert(initialized && "Image is not initialized");

    RGB<float> ret;
    ret.r = (float) image[at(x, y) + 0] / 255.0f;
    ret.g = (float) image[at(x, y) + 1] / 255.0f;
    ret.b = (float) image[at(x, y) + 2] / 255.0f;
    return ret;
}

unsigned char float_to_u8(float t)
{
    if (t < 0.0f)
    {
        t = 0.0f;
    }
    else if (t > 1.0f)
    {
        t = 1.0f;
    }
    unsigned char c = (unsigned char) (t * 255.0f);
    return c;
}

std::shared_ptr<U8Image> generate_gradient_image(int w, int h)
{
    std::shared_ptr<U8Image> ret = std::make_shared<U8Image>(w, h, 4);
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            float r = ((float) x / w);
            float g = ((float) y / h);
            ret->set_rgb(x, y, float_to_u8(r), float_to_u8(g), 0);
        }
    }
    return ret;
}


template<>
bool FloatImage::load(const std::string &path)
{
    stbi_set_flip_vertically_on_load(true);

    unsigned char *data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    assert((ch == 3 || ch == 4) && "Unsupported number of channels");

    if (!data)
    {
        return false;
    }

    image.reset(new float[w * h * 4]);

    // Now we need to convert the untis
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int offset_load = (y * w + x) * ch;
            RGB<unsigned char> color = { data[offset_load + 0], data[offset_load + 1], data[offset_load + 2] };
            image[at(x, y) + 0] = (float) color.r / 255.0f;
            image[at(x, y) + 1] = (float) color.g / 255.0f;
            image[at(x, y) + 2] = (float) color.b / 255.0f;
        }
    }

    initialized = true;

    return true;
}



template<>
RGB<unsigned char> FloatImage::get_rgb(int x, int y) const
{
    assert(initialized && "Image is not initialized");

    RGB<unsigned char> ret;
    ret.r = float_to_u8(image[at(x, y) + 0]);
    ret.g = float_to_u8(image[at(x, y) + 1]);
    ret.b = float_to_u8(image[at(x, y) + 2]);
    return ret;
}

template<>
RGB<float> FloatImage::get_rgb_float(int x, int y) const
{
    assert(initialized && "Image is not initialized");

    RGB<float> ret;
    ret.r = image[at(x, y) + 0];
    ret.g = image[at(x, y) + 1];
    ret.b = image[at(x, y) + 2];
    return ret;
}

template<>
bool FloatImage::save(const std::string &dest) const
{
    assert(initialized && "Image is not initialized");

    std::unique_ptr<unsigned char[]> uc(new unsigned char[w * h * ch]);
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int offset = (y * w + x) * ch;
            RGB<unsigned char> urgb = get_rgb(x, y);
            uc[offset + 0] = urgb.r;
            uc[offset + 1] = urgb.g;
            uc[offset + 2] = urgb.b;
            uc[offset + 3] = 255;
        }
    }

    stbi_flip_vertically_on_write(true);
    int res = stbi_write_png(dest.c_str(), w, h, ch, uc.get(), w * ch);

    return res != 0;
}
