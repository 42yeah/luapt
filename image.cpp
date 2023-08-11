// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "image.h"
#include <cassert>
#include <cstring>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

constexpr int num_ch = 4;
int id = 0;

Image::Image() : w(0), h(0), image(nullptr), id_(::id++), initialized(false)
{

}

Image::Image(int w, int h, int ch) : w(w), h(h), image(nullptr), id_(::id++), initialized(false)
{
    assert(ch == num_ch && "Unsupported: channels != 4");
    image.reset(new CComp[w * h * num_ch]);
    initialized = true;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            image[at(x, y) + 0] = 0;
            image[at(x, y) + 1] = 0;
            image[at(x, y) + 2] = 0;
            image[at(x, y) + 3] = 0;
        }
    }
}

unsigned int Image::at(int x, int y) const
{
    assert(initialized && "Image is not initialized");

    if (x < 0) { x = 0; }
    if (y < 0) { y = 0; }
    if (x >= w) { x = w; }
    if (y >= h) { y = h; }
    return (y * w + x) * num_ch;
}

void Image::set_rgb(int x, int y, CComp r, CComp g, CComp b)
{
    assert(initialized && "Image is not initialized");

    image[at(x, y) + 0] = r;
    image[at(x, y) + 1] = g;
    image[at(x, y) + 2] = b;
    image[at(x, y) + 3] = 255;
}

void Image::set_rgb(int x, int y, const RGB &rgb)
{
    assert(initialized && "Image is not initialized");

    set_rgb(x, y, rgb.r, rgb.g, rgb.b);
}

RGB Image::get_rgb(int x, int y) const
{
    assert(initialized && "Image is not initialized");

    RGB ret;
    ret.r = image[at(x, y) + 0];
    ret.g = image[at(x, y) + 1];
    ret.b = image[at(x, y) + 2];
    return ret;
}

Image::Image(const Image& other)
{
    w = other.w;
    h = other.h;
    initialized = other.initialized;
    std::memcpy(image.get(), other.image.get(), sizeof(CComp) * w * h * num_ch);
}

bool Image::save(const std::string &dest) const
{
    assert(initialized && "Image is not initialized");

    stbi_flip_vertically_on_write(true);
    int res = stbi_write_png(dest.c_str(), w, h, num_ch, image.get(), w * num_ch);

    return res != 0;
}

Image::~Image()
{

}

CComp ccomp(float t)
{
    CComp c = (CComp) (t * 255.0f);
    return c;
}

std::shared_ptr<Image> generate_gradient_image(int w, int h)
{
    std::shared_ptr<Image> ret = std::make_shared<Image>(w, h, 4);
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            float r = ((float) x / w);
            float g = ((float) y / h);
            ret->set_rgb(x, y, ccomp(r), ccomp(g), 0);
        }
    }
    return ret;
}

int Image::id() const
{
    return id_;
}
