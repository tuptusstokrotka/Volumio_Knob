import os
import gzip
import subprocess
from pathlib import Path

RED     = '\033[0;31m'
GREEN   = '\033[0;32m'
YELLOW  = '\033[0;33m'
END     = '\033[0m'

ROOT_DIR = os.path.abspath(os.curdir)

FILES = [
    {
        "input":    os.path.join(ROOT_DIR, 'src\\webserver\\index.html'),
        "header":   os.path.join(ROOT_DIR, 'src\\webserver\\build\\html.h'),
        "var":      "html",
        "type":     "html"
    }
]

def open_or_create(filepath):
    Path(filepath).parent.mkdir(parents=True, exist_ok=True)
    if not os.path.exists(filepath):
        with open(filepath, 'w') as f:
            f.write("")

def minify_asset_minify_exe(src, dst):
    minifier_path = os.path.join(ROOT_DIR, "script", "minify.exe")
    try:
        subprocess.run([minifier_path, "-o", dst, src], check=True, capture_output=True)
        return True
    except Exception as e:
        print(f"{RED}Minify failed: {e}")
        return False

def gzip_file(input_file, output_file):
    with open(input_file, 'rb') as f_in:
        with gzip.open(output_file, 'wb', compresslevel=9) as f_out:
            f_out.write(f_in.read())

def file_to_header(source_gz, header_file, var_name):
    with open(source_gz, 'rb') as f:
        data = f.read()

    with open(header_file, 'w') as f_out:
        f_out.write(f"const uint8_t {var_name} PROGMEM [] = {{")
        for i, byte in enumerate(data):
            if i % 16 == 0:
                f_out.write("\n  ")
            f_out.write(f"0x{byte:02x}, ")
        f_out.write("\n};\n")
        f_out.write(f"const unsigned int {var_name}_len = {len(data)};\n")

def clean(minified, gzipped):
    try:
        os.remove(minified)
    except Exception as e:
        print(f"{RED}Failed to remove {minified}: {e}")

    try:
        os.remove(gzipped)
    except Exception as e:
        print(f"{RED}Failed to remove {gzipped}: {e}")

def process_file(entry):
    src = entry["input"]
    base, ext = os.path.splitext(src)
    minified = base + ".min" + ext
    gzipped = minified + ".gz"
    header = entry["header"]
    var = entry["var"]

    open_or_create(header)
    if not os.path.exists(src):
        print(f"{RED}Source file not found: {src}")
        return

    if minify_asset_minify_exe(src, minified):
        gzip_file(minified, gzipped)
        file_to_header(gzipped, header, var)
        print(f"Embedding {var}:\t\t" + GREEN + "SUCCESS" + END + f" ({os.path.getsize(gzipped)} bytes)")
    else:
        print(f"Embedding {var}:\t\t" + RED + "FAILED")

    clean(minified, gzipped)

for entry in FILES:
    process_file(entry)