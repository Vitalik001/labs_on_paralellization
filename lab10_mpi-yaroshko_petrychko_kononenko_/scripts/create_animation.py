import os
from PIL import Image

def create_animation(image_folder, output_path, duration=500, loop=0):
    image_files = [entry.name for entry in os.scandir(image_folder) if entry.name.lower().startswith('image') and entry.name.lower().endswith('.png')]
    image_files = sorted(image_files, key=lambda x: int(''.join(filter(str.isdigit, x))))

    if len(image_files) == 0:
        print("No image files found in the specified folder.")
        return

    images = []
    for image_file in image_files:
        image_path = os.path.join(image_folder, image_file)
        image = Image.open(image_path)
        images.append(image)

    images[0].save(output_path, save_all=True, append_images=images[1:],
                   optimize=False, duration=duration, loop=loop, disposal=2)

    print(f"Animation created at {output_path}")


if __name__ == "__main__":
    image_folder = "./data"
    output_path = "./data/animation.gif"
    duration = 500  # milliseconds
    loop = 0  # 0 means no loop
    create_animation(image_folder, output_path, duration, loop)