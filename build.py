import os
import subprocess

def run_loud(cmd, **kwargs):
    """Run a command and print output live."""
    print(f"Running: {' '.join(cmd)}")
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, **kwargs)
    for line in process.stdout:
        print(line, end='')
    retcode = process.wait()
    if retcode != 0:
        raise subprocess.CalledProcessError(retcode, cmd)

# Build directory
build_dir = "build"
os.makedirs(build_dir, exist_ok=True)

# Read source files
with open("windows.c", "r", encoding="utf-8") as f:
    cpp_text = f.read()
with open("index.html", "rb") as f:
    html_content = f.read()

# Convert HTML to C byte array efficiently
html_array_str = ",\n ".join(
    ", ".join(f"0x{b:02X}" for b in html_content[i:i+16])
    for i in range(0, len(html_content), 16)
)

# Replace placeholders in C source
output_cpp = cpp_text.replace("/*HTML_DATA_PLACEHOLDER*/", html_array_str)
output_cpp = output_cpp.replace("/*HTML_LEN_PLACEHOLDER*/", str(len(html_content)))

# Write generated C file
with open(os.path.join(build_dir, "neighbours.c"), "w", encoding="utf-8") as f:
    f.write(output_cpp)

# Copy WebView2Loader.dll
run_loud(["cmd", "/c", "copy", "WebView2Loader.dll", os.path.join(build_dir, "WebView2Loader.dll")], shell=True)

# Compile with Clang using high optimization and fast-math
run_loud([
    "clang",
    "-nostdlib",
    os.path.join(build_dir, "neighbours.c"),
    "-I", "C:/vcpkg/installed/x86-windows/include/",
    "C:/vcpkg/installed/x86-windows/lib/WebView2Loader.dll.lib",
    "-Wl,/ENTRY:WinMainCRTStartup,/SUBSYSTEM:windows",
    "-lkernel32",
    "-luser32",
    "-m32",
    "-O3",
    "-ffunction-sections",
    "-fdata-sections",
    "-fno-exceptions",
    "-fno-rtti",
    "-fno-threadsafe-statics",
    "-ffast-math",
    "-o", os.path.join(build_dir, "neighbours.exe")
])

# Compress the DLL and EXE into a zip efficiently
run_loud([
    "powershell",
    "Compress-Archive",
    "-Path",
    f"{os.path.join(build_dir, 'neighbours.exe')},{os.path.join(build_dir, 'WebView2Loader.dll')}",
    "-DestinationPath",
    os.path.join(build_dir, "neighbours.zip"),
    "-Force"
])
