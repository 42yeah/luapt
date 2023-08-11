print("Hello, world!")
a = make_image(100, 100)

for y = 1, 100 do
    for x = 1, 100 do
        set_pixel(a, x - 1, y - 1, (x / 100), (y / 100), 0.5)
    end
end

save_image(a, "hi.png")
free_image(a)
