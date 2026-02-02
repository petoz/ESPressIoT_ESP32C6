import os
import subprocess

def compile_scss():
    print("Compiling SCSS...")
    try:
        # Check if sass is available
        subprocess.check_call(["sass", "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        # Compile SCSS
        subprocess.check_call(["sass", "web/style.scss", "web/style.css"])
        print("SCSS compilation successful.")
    except Exception as e:
        print(f"Error compiling SCSS: {e}")
        # Build failed if SCSS cannot be compiled
        # exit(1) 
        # Fallback for now or strictly fail? strictly fail is better for visibility.
        if os.path.exists("web/style.css"):
             print("Using existing style.css due to sass error.")
        else:
             raise e

def read_file(path):
    with open(path, "r") as f:
        return f.read()

def generate_header():
    print("Generating src/WebStatic.h...")
    
    html_content = read_file("web/index.html")
    config_html_content = read_file("web/config.html")
    css_content = read_file("web/style.css")
    js_content = read_file("web/script.js")

    header_content = f"""#ifndef WEBSTATIC_H
#define WEBSTATIC_H

#include <Arduino.h>

static const char index_html[] = R"rawliteral(
{html_content}
)rawliteral";

static const char config_html[] = R"rawliteral(
{config_html_content}
)rawliteral";

const char style_css[] PROGMEM = R"rawliteral(
{css_content}
)rawliteral";

const char script_js[] PROGMEM = R"rawliteral(
{js_content}
)rawliteral";

#endif
"""
    
    with open("src/WebStatic.h", "w") as f:
        f.write(header_content)
    print("src/WebStatic.h generated.")

if __name__ == "__main__":
    compile_scss()
    generate_header()
