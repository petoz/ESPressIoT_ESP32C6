import subprocess
import datetime

# Get git commit hash
revision = ""
try:
    revision = (
        subprocess.check_output(["git", "rev-parse", "--short", "HEAD"])
        .strip()
        .decode("utf-8")
    )
    # Check for uncommitted changes
    try:
        subprocess.check_call(["git", "diff", "--quiet"])
        subprocess.check_call(["git", "diff", "--cached", "--quiet"])
    except subprocess.CalledProcessError:
        revision += "-dirty"
except:
    revision = "unknown"

# Get build timestamp
timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

print("-DGIT_COMMIT='\"%s\"'" % revision)
print("-DBUILD_TIME='\"%s\"'" % timestamp)

Import("env")

env.Append(
    CPPDEFINES=[
        ("GIT_COMMIT", '\\"%s\\"' % revision),
        ("BUILD_TIME", '\\"%s\\"' % timestamp),
    ]
)
