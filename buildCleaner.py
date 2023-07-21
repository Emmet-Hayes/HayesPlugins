import os
import shutil

# Set the directory you want to start from
root_dir = '.'

for dir_name, sub_dirs, files in os.walk(root_dir):
    if 'Builds' in sub_dirs:
        # Construct the full path to the Builds directory
        builds_dir = os.path.join(dir_name, 'Builds')
        
        # Remove the Builds directory and its contents
        shutil.rmtree(builds_dir)

        print(f'Removed directory: {builds_dir}')