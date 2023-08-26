// ImageGL is an OpenGL wrapper over an Image. I guess it's also called Texture.
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEGL_H
#define IMAGEGL_H

#include <glad/glad.h>
#include <memory>
#include "image.h"

/**
 * ImageGL is an OpenGL wrapper over an Image. I guess it's also called Texture.
 */
class ImageGL
{
public:
    /**
     * Default constructor
     */
    ImageGL();

    ImageGL(std::shared_ptr<Image> image);

    bool import_from_image(const Image &image);
    const std::shared_ptr<Image> get_base_image() const;

    /**
     * No copy constructor
     */
    ImageGL(const ImageGL& other) = delete;

    /**
     * Destructor
     */
    ~ImageGL();

private:
    bool initialized;
    GLuint texture;
    std::shared_ptr<Image> image;
};

#endif // IMAGEGL_H
