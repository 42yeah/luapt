-- Name inspiration drawn from OCaml.

ffi = require "ffi"
require "lib/vector"

ffi.cdef[[
    typedef struct Image Image;
    typedef struct Model Model;

    // Images
    Image *make_image(int width, int height);
    void set_pixel(Image *img, int x, int y, float r, float g, float b);
    bool save_image(Image *img, const char *path);
    void free_image(Image *img);

    // Models
    Model *make_model(const char *path);
    int model_tri_count(const Model *model);
    TriC *model_get_tri(const Model *model, int index);
    void free_model(Model *model);

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
]]

make_image = ffi.C.make_image
set_pixel = ffi.C.set_pixel
save_image = ffi.C.save_image
free_image = ffi.C.free_image

make_model = ffi.C.make_model
model_tri_count = ffi.C.model_tri_count
model_get_tri = ffi.C.model_get_tri
free_model = ffi.C.free_model

inventory_add = ffi.C.inventory_add
inventory_get = ffi.C.inventory_get
inventory_clear = ffi.C.inventory_clear

shade = ffi.C.shade

-- pparams contains:
-- x, y, u, v, w, h
