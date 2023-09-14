// Homemade vector library.
// SPDX-FileCopyrightText: 2023 42yeah hey@42yeah.is
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LUAMATH_H
#define LUAMATH_H


extern "C"
{
     /**
     * BEGIN
     * A more boring and vanilla triangle definition.
     */
    typedef struct
    {
        float x;
        float y;
        float z;
    } Vec3C;

    typedef struct
    {
        float u, v;
    } Vec2C;

    typedef struct
    {
        Vec3C position;
        Vec3C normal;
        Vec2C tex_coord;
        int material_id;
    } VertexC;

    typedef struct
    {
        VertexC a, b, c;
    } TriC;
    // END boring triangle definition //

    Vec3C vec3(float x, float y, float z);
    Vec2C vec2(float u, float v);
    Vec3C add3(const Vec3C &a, const Vec3C &b);
    Vec3C sub3(const Vec3C &a, const Vec3C &b);
    float dot3(const Vec3C &a, const Vec3C &b);
    Vec3C mul3(const Vec3C &a, const Vec3C &b);
    Vec3C scl3(const Vec3C &a, float s);
    Vec3C cross(const Vec3C &a, const Vec3C &b);
    float len3(const Vec3C &a);
    Vec3C nor3(const Vec3C &a);
    float mix(float a, float b, float val);
    Vec3C mix3(const Vec3C &a, const Vec3C &b, float val);
    Vec3C min3(const Vec3C &a, const Vec3C &b);
    Vec3C max3(const Vec3C &a, const Vec3C &b);
    Vec2C add2(const Vec2C &a, const Vec2C &b);
    Vec2C scl2(const Vec2C &a, float s);
    float dot2(const Vec2C &a, const Vec2C &b);

    // Boring BBox definition
    typedef struct
    {
        Vec3C min, max;
    } BBox;

    typedef struct
    {
        BBox bbox;
        int start, size;
        int l, r; // Left child & right child
    } Node;
    BBox bbox();
    void enclose(BBox &bbox, const Vec3C &p);
}

#endif // LUAMATH_H
