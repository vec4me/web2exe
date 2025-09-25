import subprocess
from pathlib import Path

cpp_text = Path("windows.c").read_text(encoding="utf-8")
html_content = Path("index.html").read_text(encoding="utf-8")
escaped_html = html_content.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n\"\n\"")

Path("build/neighbours.c").write_text(cpp_text.replace('L"<html></html>"', f'L"{escaped_html}"'), encoding="utf-8")

subprocess.run(["build.bat"], check=True)
subprocess.run(["package.bat"], check=True)