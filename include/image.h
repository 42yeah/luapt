// The image interface.
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGE_H
#define IMAGE_H

#include <iostream>
#include <memory>

using CComp = unsigned char; // CComp for color component

struct RGB
{
    CComp r;
    CComp g;
    CComp b;
};

class Image
{
public:
    /**
     * Default constructor
     */
    Image();

    Image(int w, int h, int ch);

    /**
     * Load image from file.
     * Returns false upon failure.
     */
    bool load(const std::string &path);

    unsigned int at(int x, int y) const;
    void set_rgb(int x, int y, CComp r, CComp g, CComp b);
    void set_rgb(int x, int y, const RGB &rgb);
    RGB get_rgb(int x, int y) const;
    bool save(const std::string &dest) const;
    bool save_compressed(const std::string &dest, int quality) const;

    /**
     * Resize the image using stb_image_resize.
     */
    bool resize(int nx, int ny);

    /**
     * Copy constructor
     *
     * @param other The other image, to be copied from.
     */
    Image(const Image& other);

    /**
     * Destructor
     */
    ~Image();

    int id() const;

    int w, h, ch;

private:
    std::unique_ptr<CComp[]> image;
    int id_;
    bool initialized;
};

CComp ccomp(float t);

/**
 * Generates a test gradient image.
 */
std::shared_ptr<Image> generate_gradient_image(int w, int h);

#endif // IMAGE_H
