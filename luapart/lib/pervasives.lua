-- Name inspiration drawn from OCaml.

ffi = require "ffi"
require "lib/vector"
local buffer = require "string.buffer"

ffi.cdef[[
    typedef struct Image Image;
    typedef struct Model Model;
    typedef struct BVH BVH;

    // Defined in material.h
    typedef struct
    {
        float metallic;
        float ior;
        Vec3C emission;
        Vec3C ambient;
        Vec3C diffuse;
        Vec3C normal_bump;
        Vec3C specular;
    } HitInfo;

    HitInfo hit_info();

    // Defined in luaenv.h
    // Images
    Image *make_image(int width, int height);
    Image *load_image(const char *path);
    void set_pixel(Image *img, int x, int y, float r, float g, float b);
    bool save_image(Image *img, const char *path);
    void free_image(Image *img);
    Vec3C get_pixel(Image *img, int x, int y);
    Vec3C sample_image(Image *img, float u, float v);
    void generate_demo_image(int w, int h, const char *path);

    // Models
    Model *make_model(const char *path, const char *mtl_base_path);
    int model_tri_count(const Model *model);
    TriC *model_get_tri(const Model *model, int index);
    void free_model(Model *model);
    HitInfo model_hit_info(Model *model, int material_id, Vec2C uv);

    // BVHs
    BVH *make_bvh(Model *model);
    TriC *bvh_get_tri(const BVH *bvh, int index);
    int bvh_tri_count(const BVH *bvh);
    TriC *bvh_get_emitter(const BVH *bvh, int index);
    int bvh_emitter_count(const BVH *bvh);
    int bvh_push_node(BVH *bvh, const BBox &bbox, int start, int size, int l, int r);
    const Node bvh_get_node(BVH *bvh, int index);
    void bvh_node_set_children(BVH *bvh, int who, int l, int r);
    int bvh_node_count(const BVH *bvh);
    void free_bvh(BVH *bvh);

    /**
     * Returns an array of booleans. This will be used to partition the array.
     * The partitioning array __will be freed__ upon calling `partition`.
     */
    bool *make_partitioning_table(BVH *bvh);
    int partition(BVH *bvh, bool *table, int begin, int end);

    // Inventory
    void inventory_add(const char *k, void *v);
    void *inventory_get(const char *k);
    void inventory_clear();

    // Shared strings

    typedef struct
    {
        char *serialized;
        int size;
    } SharedInfo;
    void shared_add(const char *k, const char *serialized, int size);
    const SharedInfo *shared_get(const char *k);
    void shared_clear();

    /**
     * Shade function launches a bunch of threads (width*height).
     * Sure we can dump functions and whatnot like before but that's not really necessary.
     * Since resource is shared in this brand new world, simply passing in a Lua script path
     * is easier, offers finer-grained control, and much more safer.
     */
    void shade(int width, int height, const char *path);

    /**
     * Trigger the debugger.
     * Insert your own breakpoint here!
     */
    void debug();
]]

hit_info = ffi.C.hit_info

make_image = ffi.C.make_image
load_image = ffi.C.load_image
set_pixel = ffi.C.set_pixel
save_image = ffi.C.save_image
free_image = ffi.C.free_image
get_pixel = ffi.C.get_pixel
sample_image = ffi.C.sample_image
generate_demo_image = ffi.C.generate_demo_image

make_model = ffi.C.make_model
model_tri_count = ffi.C.model_tri_count
model_get_tri = ffi.C.model_get_tri
free_model = ffi.C.free_model
model_hit_info = ffi.C.model_hit_info

make_bvh = ffi.C.make_bvh
bvh_get_tri = ffi.C.bvh_get_tri
bvh_tri_count = ffi.C.bvh_tri_count
bvh_get_emitter = ffi.C.bvh_get_emitter
bvh_emitter_count = ffi.C.bvh_emitter_count
bvh_push_node = ffi.C.bvh_push_node
bvh_get_node = ffi.C.bvh_get_node
bvh_node_set_children = ffi.C.bvh_node_set_children
bvh_node_count = ffi.C.bvh_node_count
free_bvh = ffi.C.free_bvh
make_partitioning_table = ffi.C.make_partitioning_table
partition = ffi.C.partition

inventory_add = ffi.C.inventory_add
inventory_get = ffi.C.inventory_get
inventory_clear = ffi.C.inventory_clear

function shared_add(key, table)
    local str = buffer.encode(table)
    ffi.C.shared_add(key, str, #str)
end

function shared_get(key)
    local info = ffi.C.shared_get(key)
    if info == nil then
        return nil
    end
    local str = ffi.string(info.serialized, info.size)
    return buffer.decode(str)
end

shared_clear = ffi.C.shared_clear

-- pparams contains:
-- x, y, u, v, w, h
shade = ffi.C.shade
debug = ffi.C.debug

