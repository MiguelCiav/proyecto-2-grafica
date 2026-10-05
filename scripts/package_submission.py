#!/usr/bin/env python3
"""
Script de Empaquetado para Entrega Oficial
Proyecto 2 - Computación Gráfica (UCV)

Uso:
    python3 scripts/package_submission.py <CEDULA_DEV_A> <CEDULA_DEV_B>

Ejemplo:
    python3 scripts/package_submission.py 28123456 29654321
Genera:
    PROY2_28123456_29654321.zip
"""

import sys
import os
import zipfile
from pathlib import Path

# Directorios y archivos esenciales para la compilación y evaluación
INCLUDE_DIRS = ["assets", "external", "src"]
INCLUDE_FILES = ["CMakeLists.txt", "README.md"]

# Patrones o extensiones a excluir
EXCLUDE_DIRS = {".git", "build", "cmake-build-debug", "cmake-build-release", ".vscode", ".idea", "__pycache__"}
EXCLUDE_EXTENSIONS = {".o", ".obj", ".exe", ".out", ".log", ".tmp"}
EXCLUDE_FILES = {".DS_Store", "Thumbs.db", "desktop.ini", "test_run.scene", "pepe.scene"}

def should_exclude_file(filepath: Path) -> bool:
    if filepath.name in EXCLUDE_FILES:
        return True
    if filepath.suffix.lower() in EXCLUDE_EXTENSIONS:
        return True
    for parent in filepath.parents:
        if parent.name in EXCLUDE_DIRS:
            return True
    return False

def package_submission(cedula1: str, cedula2: str):
    root_dir = Path(__file__).resolve().parent.parent
    zip_name = f"PROY2_{cedula1}_{cedula2}.zip"
    zip_path = root_dir / zip_name

    print(f"==================================================")
    print(f"📦 Generando paquete de entrega: {zip_name}")
    print(f"📂 Raíz del proyecto: {root_dir}")
    print(f"==================================================")

    total_files = 0
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zf:
        # 1. Agregar archivos raíz obligatorios
        for filename in INCLUDE_FILES:
            file_path = root_dir / filename
            if file_path.exists():
                arcname = str(file_path.relative_to(root_dir))
                zf.write(file_path, arcname)
                print(f"  + {arcname}")
                total_files += 1
            else:
                print(f"  ⚠️ Archivo no encontrado: {filename}")

        # 2. Agregar carpetas de código y recursos
        for dirname in INCLUDE_DIRS:
            dir_path = root_dir / dirname
            if not dir_path.exists():
                print(f"  ⚠️ Directorio no encontrado: {dirname}")
                continue

            for root, dirs, files in os.walk(dir_path):
                # Filtrar directorios excluidos in-place
                dirs[:] = [d for d in dirs if d not in EXCLUDE_DIRS]

                for file in files:
                    file_path = Path(root) / file
                    if should_exclude_file(file_path):
                        continue

                    arcname = str(file_path.relative_to(root_dir))
                    zf.write(file_path, arcname)
                    print(f"  + {arcname}")
                    total_files += 1

    file_size_kb = zip_path.stat().st_size / 1024.0
    print(f"==================================================")
    print(f"✅ Paquete creado exitosamente: {zip_path.name}")
    print(f"📁 Tamaño: {file_size_kb:.2f} KB ({total_files} archivos)")
    print(f"==================================================")
    return zip_path

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Uso: python3 scripts/package_submission.py <CEDULA_DEV_A> <CEDULA_DEV_B>")
        print("Ejemplo: python3 scripts/package_submission.py 28123456 29654321")
        sys.exit(1)

    c1 = sys.argv[1].strip()
    c2 = sys.argv[2].strip()
    package_submission(c1, c2)
