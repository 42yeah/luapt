-- Name inspiration drawn from OCaml.

ffi = require "ffi"
require "lib/vector"

ffi.cdef[[
    typedef struct Image Image;
    typedef struct Model Model;
    typedef struct BVH BVH;

    // Images
    Image *make_image(int width, int height);
    void set_pixel(Image *img, int x, int y, float r, float g, float b);
    bool save_image(Image *img, const char *path);
    void free_image(Image *img);
    void generate_demo_image(int w, int h, const char *path);

    // Models
    Model *make_model(const char *path, const char *mtl_base_path);
    int model_tri_count(const Model *model);
    TriC *model_get_tri(const Model *model, int index);
    void free_model(Model *model);

    // BVHs
    BVH *make_bvh(Model *model);
    TriC *bvh_get_tri(const BVH *bvh, int index);
    int bvh_tri_count(const BVH *bvh);
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

make_image = ffi.C.make_image
set_pixel = ffi.C.set_pixel
save_image = ffi.C.save_image
free_image = ffi.C.free_image
generate_demo_image = ffi.C.generate_demo_image

make_model = ffi.C.make_model
model_tri_count = ffi.C.model_tri_count
model_get_tri = ffi.C.model_get_tri
free_model = ffi.C.free_model

make_bvh = ffi.C.make_bvh
bvh_get_tri = ffi.C.bvh_get_tri
bvh_tri_count = ffi.C.bvh_tri_count
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

shade = ffi.C.shade
debug = ffi.C.debug

-- pparams contains:
-- x, y, u, v, w, h

