local ffi = require("ffi")

ffi.cdef[[
    void print_hello(int a);
]]

ffi.C.print_hello(42);
