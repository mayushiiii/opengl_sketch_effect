# Sketch Effect Filter for Images

The program applies filters on an image using GPU operations done in Fragment Shader. In the CPP code, whenever multiple filters are used together, the intermediate result is stored in a Framebuffer to assure the correct final result.

The filters and their keybinds are as follows:

- `1` - Original image, no changes
- `2` - Sobel filter
- `3` - Horizontal blur
- `4` - Vertical + Horizontal blur => Blur effect
- `5` - Blur effect + Hatching effect 1
- `6` - Blur effect + Hatching effect 2
- `7` - Blur effect + Hatching effect 3
- `8` - Blur effect + Sobel filter + Hatching effect 1 + Hatching effect 2 + Hatching effect 3.
- `9` - Grayscale effect.

The hatching effects' parameters are adjusted in such ways as to give different looks to the image. When overlaid they are supposed to have a cross-hatching look.


![collage](https://github.com/user-attachments/assets/2e88d5d2-4cf8-4a71-a092-8acf6438f31a)
