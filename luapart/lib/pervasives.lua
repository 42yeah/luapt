-- Name inspiration drawn from OCaml.

ffi = require("ffi")

ffi.cdef[[
    typedef struct Image Image;
    typedef struct Model Model;

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
