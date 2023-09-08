ffi = require "ffi"


ffi.cdef[[
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
]]

vec2 = ffi.C.vec2
vec3 = ffi.C.vec3
add3 = ffi.C.add3
sub3 = ffi.C.sub3
dot3 = ffi.C.dot3
mul3 = ffi.C.mul3
scl3 = ffi.C.scl3
cross = ffi.C.cross
len3 = ffi.C.len3
nor3 = ffi.C.nor3
mix = ffi.C.mix
mix3 = ffi.C.mix3
