from PIL import Image

# Read original PBM
img = Image.open("input_images/test.pbm")
width, height = img.size

# Calculate 4K dimensions (exact 2x for a 1920x1080 image is 3840x2160)
new_width = width * 2
new_height = height * 2

# Resize using nearest neighbor to preserve binary edges
img_4k = img.resize((new_width, new_height), Image.NEAREST)

# Save the new 4K image
img_4k.save("input_images/test_4k.pbm")
print(f"Creato input_images/test_4k.pbm con dimensioni {new_width}x{new_height}")
