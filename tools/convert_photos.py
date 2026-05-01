#!/usr/bin/env python3
"""
convert_photos.py
Convierte imágenes (JPG, PNG, etc.) a BMP 24-bit 240x240
listas para copiar a la microSD en /photos/

Uso:
    python convert_photos.py foto1.jpg foto2.png ...
    python convert_photos.py *.jpg          (todas las fotos)
    python convert_photos.py -i carpeta/    (todos los archivos de una carpeta)

Requiere Pillow:
    pip install Pillow
"""

import sys
import os
import argparse
from pathlib import Path
from PIL import Image

OUTPUT_DIR  = Path("photos_output")
TARGET_SIZE = (240, 240)


def convert_image(src_path: Path, index: int) -> bool:
    try:
        img = Image.open(src_path).convert("RGB")
    except Exception as e:
        print(f"  [ERROR] No se pudo abrir {src_path}: {e}")
        return False

    # Recortar al centro (crop cuadrado antes de resize)
    w, h = img.size
    side  = min(w, h)
    left  = (w - side) // 2
    top   = (h - side) // 2
    img   = img.crop((left, top, left + side, top + side))

    # Redimensionar a 240x240 con alta calidad
    img = img.resize(TARGET_SIZE, Image.LANCZOS)

    # Guardar como BMP 24-bit con nombre numerado
    out_name = OUTPUT_DIR / f"{index:03d}.bmp"
    img.save(out_name, format="BMP")
    print(f"  [OK] {src_path.name} → {out_name.name}")
    return True


def main():
    parser = argparse.ArgumentParser(description="Convierte fotos a BMP 240x240 para Pico Photo Frame")
    parser.add_argument("files", nargs="*", help="Archivos de imagen a convertir")
    parser.add_argument("-i", "--input-dir", help="Carpeta con imágenes a convertir")
    args = parser.parse_args()

    # Recopilar archivos fuente
    sources = []
    if args.input_dir:
        folder = Path(args.input_dir)
        exts   = {".jpg", ".jpeg", ".png", ".webp", ".heic", ".tiff", ".bmp"}
        sources = sorted([f for f in folder.iterdir()
                          if f.is_file() and f.suffix.lower() in exts])
    if args.files:
        sources += [Path(f) for f in args.files]

    if not sources:
        print("No se especificaron imágenes. Usa: python convert_photos.py foto1.jpg foto2.jpg")
        sys.exit(1)

    # Crear carpeta de salida
    OUTPUT_DIR.mkdir(exist_ok=True)
    print(f"\nConvirtiendo {len(sources)} imagen(es) a BMP 240x240...\n")

    ok = 0
    for i, src in enumerate(sources, start=1):
        if convert_image(src, i):
            ok += 1

    print(f"\n✓ {ok}/{len(sources)} imágenes convertidas en '{OUTPUT_DIR}/'")
    print(f"  Copia el contenido de '{OUTPUT_DIR}/' a la carpeta /photos/ de tu microSD")


if __name__ == "__main__":
    main()
