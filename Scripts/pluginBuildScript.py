import os
import subprocess
import platform
import argparse


parser = argparse.ArgumentParser(description="Build plugins.")
parser.add_argument('--installers-only', action='store_true', help='Skip building plugins')
parser.add_argument('--skip-installers', action='store_true', help='Skip building installers')
args = parser.parse_args()

output_dir = "HayesInstallers"
scripts_dir = "BuildScripts"

plugins = [d for d in os.listdir() if os.path.isdir(d)]

for plugin in plugins:
    if str(plugin) == "JUCE" or str(plugin) == "Common" or str(plugin) == "HayesInstallers" or str(plugin) == ".git" or str(plugin) == "Scripts" or str(plugin) == "HayesBundle":
        continue

    os.chdir(plugin)

    subprocess.run(["projucer", "--resave", f"{plugin}.jucer"])

    if platform.system() == 'Windows':
        if not args.installers_only:
            subprocess.run([ "msbuild", f"Builds/VisualStudio2019/{plugin}.sln", "/t:Build", "/p:Configuration=Release"])
        
        if not args.skip_installers:
            os.chdir(scripts_dir)
            subprocess.run([ "ISCC.exe", f"{plugin}WindowsBuildScript.iss", f"../../{output_dir}" ])
            os.chdir("../../")

    elif platform.system() == 'Darwin':
        if not args.installers_only:
            subprocess.run([ "xcodebuild", "-project", f"Builds/MacOSX/{plugin}.xcodeproj", "-configuration", "Release", "-alltargets" ])
        
        if not args.skip_installers:
            subprocess.run([ "packagesbuild", f"{scripts_dir}/{plugin}MacInstaller.pkgproj" ])
        
        os.chdir("../")
    elif platform.system() == 'Linux':
        subprocess.run([ "make", f"Builds/LinuxMakefile/make" ])
        os.chdir("../")
    else:
        print("Unsupported operating system.")
