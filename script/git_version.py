import subprocess
import os
import shutil
import sys

RED     = '\033[0;31m'
GREEN   = '\033[0;32m'
YELLOW  = '\033[0;33m'
END     = '\033[0m'

ROOT_DIR = os.path.abspath(os.curdir)
TARGET_FILE = os.path.join(ROOT_DIR, 'include/version.h')
GIT_PATH = 'git'  # Use 'git' command (works on both Windows and Linux)

def run_git_command(command):
    """ Run a git command and return its output. """
    try:
        result = subprocess.run(
            [GIT_PATH] + command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=True,
            text=True
        )
        return result.stdout.strip()
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        print(f"Git command failed: {e}")
        return None

def get_commit_hash():
    """ Get the current commit hash. """
    result = run_git_command(['rev-parse', 'HEAD'])

    if(result != None):
        print(f"Git hash:\t\t{GREEN}{result[:7]}{END}")
        return result[:7]

    print(f"Git hash:\t\t{YELLOW}None{END}")
    return "unknown"

def get_build_tag():
    """ Get the build tag if available (assuming it is stored as a Git tag). """
    result = run_git_command(['describe', '--tags', '--abbrev=0'])

    if(result != None):
        print(f"Git tag:\t\t{GREEN}{result}{END}")
        return result

    print(f"Git tag:\t\t{YELLOW}None{END}")
    return "dev"

def write_version_file(version_data):
    """ Write the version data to the target file. """
    with open(TARGET_FILE, 'w') as file:
        file.write(f'#ifndef VERSION_H_\n#define VERSION_H_\n\n')
        file.write(f'#define BUILD_TAG "{version_data["build_tag"]}"\n')
        file.write(f'#define COMMIT_HASH "{version_data["commit_hash"]}"\n')
        file.write(f'\n#endif')

version_data = {
    "build_tag": get_build_tag(),
    "commit_hash": get_commit_hash(),
}

write_version_file(version_data)