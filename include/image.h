// The image interface.
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGE_H
#define IMAGE_H

#include <iostream>
#include <memory>
#include <cassert>
#include <cstring>
#include <cmath>


template<typename T>
struct RGB
{
    RGB() : r(0), g(0), b(0)
    {

    }

    RGB(T r, T g, T b) : r(r), g(g), b(b)
    {

    }

    T r;
    T g;
    T b;
};

enum class SampleMethod
{
    Clamp, Repeat
};

/**
 * Images need to satisfy:
 * 1. Have a width and a height;
 * 2. Have a high-precision get_rgb (that returns float);
 * 3. Have a low-precision get_rgb (that returns unsigned char);
 * 4. Have a sample method (that returns float)
 */
class BaseImage
{
public:
    virtual RGB<unsigned char> get_rgb(int x, int y) const = 0;
    virtual RGB<float> get_rgb_float(int x, int y) const = 0;
    virtual RGB<float> sample_rgb(float u, float v, SampleMethod method) const = 0;

    virtual ~BaseImage() = default;
    virtual const void *const get() const = 0;

    virtual int unit_size() const = 0;

    virtual int id() const = 0;
    virtual int width() const = 0;
    virtual int height() const = 0;

    static int image_id_counter;
};

template<typename T>
class Image : public BaseImage
{
public:
    /**
     * Default constructor
     */
    Image() : w(0), h(0), ch(4), image(nullptr), id_(BaseImage::image_id_counter++), initialized(false)
    {

    }

    Image(int w, int h, int ch) : w(w), h(h), ch(ch), id_(BaseImage::image_id_counter++), image(nullptr), initialized(false)
    {
        assert((ch == 3 || ch == 4) && "Unsupported numer of channels");
        image.reset(new T[w * h * ch]);
        initialized = true;
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                for (int i = 0; i < ch; i++)
                {
                    image[at(x, y) + i] = 0;
                }
            }
        }
    }

    /**
     * Load image from file.
     * Returns false upon failure.
     */
    bool load(const std::string &path);

    int at(int x, int y) const
    {
        assert(initialized && "Image is not initialized");

        if (x < 0) { x = 0; }
        if (y < 0) { y = 0; }
        if (x >= w) { x = w - 1; }
        if (y >= h) { y = h - 1; }
        return (y * w + x) * ch;
    }

    void set_rgb(int x, int y, T r, T g, T b)
    {
        assert(initialized && "Image is not initialized");

        image[at(x, y) + 0] = r;
        image[at(x, y) + 1] = g;
        image[at(x, y) + 2] = b;

        if (ch == 4)
        {
            image[at(x, y) + 3] = 255;
        }
    }

    void set_rgb(int x, int y, const RGB<T> &rgb)
    {
        assert(initialized && "Image is not initialized");

        set_rgb(x, y, rgb.r, rgb.g, rgb.b);
    }

    RGB<unsigned char> get_rgb(int x, int y) const override;
    RGB<float> get_rgb_float(int x, int y) const override;

    RGB<float> sample_rgb(float u, float v, SampleMethod method) const override
    {
        switch (method)
        {
            case SampleMethod::Repeat:
                u = u - floorf(u);
                v = v - floorf(v);
                break;

            case SampleMethod::Clamp:
                if (u < 0.0f)
                {
                    u = 0.0f;
                }
                else if (u > 1.0f)
                {
                    u = 1.0f;
                }
                if (v < 0.0f)
                {
                    v = 0.0f;
                }
                else if (v > 1.0f)
                {
                    v = 1.0f;
                }
                break;
        }
        int x = (int) (u * w), y = (int) (v * h);
        return get_rgb_float(x, y);
    }

    bool save(const std::string &dest) const;
    bool save_compressed(const std::string &dest, int quality) const;

    const void *const get() const override // Most consts I've seen in my life
    {
        return image.get();
    }

    /**
     * Resize the image using stb_image_resize.
     */
    bool resize(int nx, int ny);

    /**
     * Copy constructor
     *
     * @param other The other image, to be copied from.
     */
    Image(const Image& other)
    {
        w = other.w;
        h = other.h;
        ch = other.ch;
        initialized = other.initialized;
        std::memcpy(image.get(), other.image.get(), sizeof(T) * w * h * ch);
    }

    virtual int unit_size() const override
    {
        return sizeof(T);
    }

    virtual int id() const override
    {
        return id_;
    }

    virtual int width() const override
    {
        return w;
    }

    virtual int height() const override
    {
        return h;
    }

    /**
     * Destructor
     */
    ~Image()
    {
        initialized = false;
    }

    int w, h, ch;

protected:
    int id_;

private:
    std::unique_ptr<T[]> image;
    bool initialized;
};

using U8Image = Image<unsigned char>;
using FloatImage = Image<float>;

unsigned char float_to_u8(float t);

/**
 * Generates a test gradient image.
 */
std::shared_ptr<U8Image> generate_gradient_image(int w, int h);

#endif // IMAGE_H
