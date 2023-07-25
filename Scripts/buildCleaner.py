import os
import shutil

# Set the directory you want to start from
root_dir = '.'

# Directories to skip
skip_dirs = { "Common", "JUCE", "HayesInstallers", "Scripts", ".git" }

for dir_name, sub_dirs, files in os.walk(root_dir):
    # Get the parent directory name
    parent_dir = os.path.basename(os.path.dirname(dir_name))

    # Check if the parent directory is in the skip list
    if parent_dir in skip_dirs:
        # Skip this directory: remove its name from sub_dirs to prevent os.walk() from visiting it
        sub_dirs[:] = []
        continue

    # Check if 'Builds' is a direct child of the current directory
    if 'Builds' in sub_dirs:
        # Construct the full path to the Builds directory
        builds_dir = os.path.join(dir_name, 'Builds')

        # Remove the Builds directory and its contents
        shutil.rmtree(builds_dir)

        print(f'Removed directory: {builds_dir}')
