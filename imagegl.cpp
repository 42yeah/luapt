// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagegl.h"
#include <cassert>

ImageGL::ImageGL() : initialized(false), texture(GL_NONE), image(nullptr)
{

}

ImageGL::ImageGL(std::shared_ptr<BaseImage> image) : ImageGL()
{
    if (image != nullptr)
    {
        this->image = image;
        import_from_image(*image);
    }
}

bool ImageGL::import_from_image(const BaseImage &image)
{
    initialized = false;
    if (texture == GL_NONE)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    if (image.unit_size() == sizeof(unsigned char))
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.get());
    }
    else if (image.unit_size() == sizeof(float))
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_FLOAT, image.get());
    }
    else
    {
        assert(false && "Unsupported base image");
    }

    initialized = true;

    return true;
}

const std::shared_ptr<BaseImage> ImageGL::get_base_image() const
{
    assert(initialized && "ImageGL is not initialized");
    return image;
}

ImageGL::~ImageGL()
{
    if (initialized)
    {
        glDeleteTextures(1, &texture);
        initialized = false;
    }
}

void ImageGL::bind() const
{
    assert(initialized && "ImageGL is not initialized");
    glBindTexture(GL_TEXTURE_2D, texture);
}
