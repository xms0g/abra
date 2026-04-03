from PIL import Image
import os

def pack_orm_texture(ao_path, roughness_path, metallic_path=None,
                     output_name="assets/DamagedHelmet/glTF/Default_occlusionRoughnessMetallic.jpg"):
    """
    Packs textures into glTF 2.0 ORM format (R: AO, G: Roughness, B: Metallic).
    If metallic_path is None, roughness_path is treated as a combined Roughness-Metallic map.
    """
    # 1. Load Ambient Occlusion (Always Greyscale)
    ao_img = Image.open(ao_path).convert('L')

    if metallic_path:
        # CASE A: Three separate files
        rough_img = Image.open(roughness_path).convert('L')
        metal_img = Image.open(metallic_path).convert('L')
    else:
        # CASE B: Two files (AO + Combined RoughnessMetallic)
        combined_rm = Image.open(roughness_path).convert('RGB')
        # Split and take Green (Rough) and Blue (Metal)
        _, rough_img, metal_img = combined_rm.split()

    # 2. Synchronize Sizes (using Roughness as the master size)
    target_size = rough_img.size

    if ao_img.size != target_size:
        ao_img = ao_img.resize(target_size, Image.Resampling.LANCZOS)
    if metal_img.size != target_size:
        metal_img = metal_img.resize(target_size, Image.Resampling.LANCZOS)

    # 3. Merge into glTF standard ORM (R=AO, G=Roughness, B=Metallic)
    orm_img = Image.merge('RGB', (ao_img, rough_img, metal_img))

    # 4. Save (Ensure directory exists)
    os.makedirs(os.path.dirname(output_name), exist_ok=True)
    orm_img.save(output_name)
    print(f"Successfully packed ORM: {output_name}")


# --- Examples of usage ---

# Usage 1: Three separate maps
# pack_orm_texture('ao.png', 'roughness.png', 'metallic.png', 'output/orm.png')

# Usage 2: Merging AO into an existing RoughnessMetallic map
# pack_orm_texture('ao.png', 'rough_metal_combined.png', output_name='output/orm.png')
# Run the function with your uploaded filenames
pack_orm_texture('assets/DamagedHelmet/glTF/Default_AO.jpg',
                 'assets/DamagedHelmet/glTF/Default_metalRoughness.jpg')
