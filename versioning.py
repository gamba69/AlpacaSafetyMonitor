# noinspection PyUnresolvedReferences
Import("env") # type: ignore
from itertools import chain
import os
import hashlib
import datetime

version = "3.0"  # Your desired major.minor version
IGNORE_FILES = ['firmware.h', 'secrets.h', 'version.h']
SOURCE_PATHS = ['src', 'lib']

def get_src_hash():
    """Calculate sources hash"""
    hash_md5 = hashlib.md5()
    for root, dirs, files in chain.from_iterable(os.walk(path) for path in SOURCE_PATHS):
        for file in sorted(files):
            if file.endswith(('.cpp', '.c', '.h', '.hpp', '.default')) and file not in IGNORE_FILES:
                filepath = os.path.join(root, file)
                with open(filepath, 'rb') as f:
                    hash_md5.update(f.read())
    return hash_md5.hexdigest()

# Execute immediately when script loads (before compilation)
hash_file = ".build_hash"
current_hash = get_src_hash()

# Check previous hash
if os.path.exists(hash_file):
    with open(hash_file, 'r') as f:
        old_hash = f.read().strip()
    
    if old_hash == current_hash:
        print("VERSIONING: Sources not changed, skip")
    else:
        # Sources changed - do versioning
        print("VERSIONING: Sources CHANGED, do versioning...")
        
        FILENAME_BUILDNO = "buildno"
        FILENAME_PATCHNO = "patchno"
        FILENAME_VERSION_H = "src/version.h"
        
        try:
            with open(FILENAME_BUILDNO) as f:
                build_no = int(f.readline()) + 1
        except:
            build_no = 0
        with open(FILENAME_BUILDNO, "w") as f:
            f.write(str(build_no))
        
        try:
            with open(FILENAME_PATCHNO) as f:
                patch_no = int(f.readline()) + 1
        except:
            patch_no = 0
        with open(FILENAME_PATCHNO, "w") as f:
            f.write(str(patch_no))
        
        with open(FILENAME_VERSION_H, "w") as f:
            f.write("#ifndef _VERSION_H\n")
            f.write("#define _VERSION_H\n")
            f.write(f"#define BUILD_NUMBER {build_no}\n")
            f.write(f'#define VERSION "{version}.{patch_no}"\n')
            f.write(f'#define BUILD_DATE "{datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")}"\n')
            f.write("#endif // _VERSION_H\n")
        
        # Save current hash
        with open(hash_file, 'w') as f:
            f.write(current_hash)
else:
    # First build
    print("VERSIONING: First build, creating version...")
    
    FILENAME_BUILDNO = "buildno"
    FILENAME_PATCHNO = "patchno"
    FILENAME_VERSION_H = "src/version.h"
    
    build_no = 0
    patch_no = 0
    
    with open(FILENAME_BUILDNO, "w") as f:
        f.write(str(build_no))
    with open(FILENAME_PATCHNO, "w") as f:
        f.write(str(patch_no))
    
    with open(FILENAME_VERSION_H, "w") as f:
        f.write("#ifndef _VERSION_H\n")
        f.write("#define _VERSION_H\n")
        f.write(f"#define BUILD_NUMBER {build_no}\n")
        f.write(f'#define VERSION "{version}.{patch_no}"\n')
        f.write(f'#define BUILD_DATE "{datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")}"\n')
        f.write("#endif // _VERSION_H\n")
    
    with open(hash_file, 'w') as f:
        f.write(current_hash)