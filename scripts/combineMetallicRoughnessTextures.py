from PIL import Image
import os

def pack_gltf_textures(roughness_path, metallic_path, output_name="assets/textures/pbr/gold/metallicRoughness.png"):
    # 1. Load images and convert to Greyscale (L mode)
    # This ensures we are working with raw 0-255 data
    rough_img = Image.open(roughness_path).convert('L')
    metal_img = Image.open(metallic_path).convert('L')

    # 2. Match sizes (if they differ, we resize the metallic to match roughness)
    if rough_img.size != metal_img.size:
        print(f"Resizing metallic map to match roughness: {rough_img.size}")
        metal_img = metal_img.resize(rough_img.size, Image.Resampling.LANCZOS)

    # 3. Create the Occlusion channel (Red)
    # glTF expects AO in Red. If you don't have one, 255 (white) means "no occlusion".
    ao_channel = Image.new('L', rough_img.size, 255)

    # 4. Merge into a single RGB image
    # R = Ambient Occlusion | G = Roughness | B = Metallic
    packed_img = Image.merge('RGB', (ao_channel, rough_img, metal_img))

    # 5. Save as lossless PNG
    packed_img.save(output_name)
    print(f"Successfully generated: {output_name}")

# Run the function with your uploaded filenames
pack_gltf_textures('assets/textures/pbr/gold/roughness.png', 'assets/textures/pbr/gold/metallic.png')