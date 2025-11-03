import datetime

FILENAME_BUILDNO = "versioning"
FILENAME_VERSION_H = "src/version.h"
version = "1.0."  # Your desired base version

try:
    with open(FILENAME_BUILDNO) as f:
        build_no = int(f.readline()) + 1
except:
    build_no = 0

with open(FILENAME_BUILDNO, "w") as f:
    f.write(str(build_no))

with open(FILENAME_VERSION_H, "w") as f:
    f.write("#ifndef _VERSION_H\n")
    f.write("#define _VERSION_H\n")
    f.write(f"#define BUILD_NUMBER {build_no}\n")
    f.write(f'#define VERSION "{version}{build_no}"\n')
    f.write(
        f'#define BUILD_DATE "{datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")}"\n'
    )
    f.write("#endif // _VERSION_H\n")
