// ImageGL is an OpenGL wrapper over an Image. I guess it's also called Texture.
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEGL_H
#define IMAGEGL_H

#include <glad/glad.h>
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

    /**
     * Copy constructor
     *
     * @param other TODO
     */
    ImageGL(const ImageGL& other);

    /**
     * Destructor
     */
    ~ImageGL();

private:
    bool initialized;
    GLuint texture;
    const Image *image;
};

#endif // IMAGEGL_H
