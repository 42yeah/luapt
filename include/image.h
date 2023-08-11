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

    unsigned int at(int x, int y) const;
    void set_rgb(int x, int y, CComp r, CComp g, CComp b);
    void set_rgb(int x, int y, const RGB &rgb);
    RGB get_rgb(int x, int y) const;
    bool save(const std::string &dest) const;

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

    int w, h;

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
