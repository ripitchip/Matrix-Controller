from PIL import Image
import sys
import os

def png_to_raw(input_path, output_path=None, size=(64, 64)):
    # Open and convert the image
    img = Image.open(input_path).convert("RGB")
    
    # Resize to match your panel if needed
    if img.size != size:
        print(f"Resizing image from {img.size} to {size}")
        img = img.resize(size, Image.LANCZOS)
    
    # Output file name
    if output_path is None:
        output_path = os.path.splitext(input_path)[0] + ".raw"
    
    # Extract pixel data
    pixels = list(img.getdata())
    
    # Write raw bytes (R, G, B)
    with open(output_path, "wb") as f:
        for r, g, b in pixels:
            f.write(bytes([r, g, b]))
    
    print(f"✅ RAW file saved as {output_path}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python png_to_raw.py <image.png> [output.raw]")
    else:
        input_path = sys.argv[1]
        output_path = sys.argv[2] if len(sys.argv) > 2 else None
        png_to_raw(input_path, output_path)
