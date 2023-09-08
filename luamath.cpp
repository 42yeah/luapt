// <one line to give the program's name and a brief idea of what it does.>
// SPDX-FileCopyrightText: 2023 42yeah <email>
// SPDX-License-Identifier: GPL-3.0-or-later


#include "luamath.h"
#include <cmath>


Vec2C vec2(float u, float v)
{
    return Vec2C{ u, v };
}

Vec3C vec3(float x, float y, float z)
{
    return Vec3C{ x, y, z };
}

Vec3C add3(const Vec3C &a, const Vec3C &b)
{
    return Vec3C{ a.x + b.x, a.y + b.y, a.z + b.z };
}

Vec3C sub3(const Vec3C &a, const Vec3C &b)
{
    return Vec3C{ a.x - b.x, a.y - b.y, a.z - b.z };
}

float dot3(const Vec3C &a, const Vec3C &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3C mul3(const Vec3C &a, const Vec3C &b)
{
    return Vec3C{ a.x * b.x, a.y * b.y, a.z * b.z };
}

Vec3C scl3(const Vec3C &a, float s)
{
    return Vec3C{ s * a.x, s * a.y, s * a.z };
}

Vec3C cross(const Vec3C &a, const Vec3C &b)
{
    return Vec3C{ a.y * b.z - a.z * b.y,
                  a.z * b.x - a.x * b.z,
                  a.x * b.y - a.y * b.x };
}

float len3(const Vec3C &a)
{
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

Vec3C nor3(const Vec3C &a)
{
    return scl3(a, 1.0f / len3(a));
}

float mix(float a, float b, float val)
{
    return (1.0f - val) * a + val * b;
}

Vec3C mix3(const Vec3C &a, const Vec3C &b, float val)
{
    return Vec3C{ mix(a.x, b.x, val), mix(a.y, b.y, val), mix(a.z, b.z, val) };
}
