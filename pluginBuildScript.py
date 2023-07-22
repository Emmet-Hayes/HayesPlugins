import os
import subprocess
import platform

plugin_dir = "."
output_dir = "HayesInstallers"
os.chdir(plugin_dir)

plugins = [d for d in os.listdir() if os.path.isdir(d)]

for plugin in plugins:
    if str(plugin) == "JUCE" or str(plugin) == "Common" or str(plugin) == "HayesInstallers":
        continue
    os.chdir(plugin)

    subprocess.run(["projucer", "--resave", f"{plugin}.jucer"])

    if platform.system() == 'Windows':
        os.chdir(plugin)
        subprocess.run(["msbuild", f"Builds/VisualStudio2019/{plugin}.sln", "/t:Build", "/p:Configuration=Release"])
        os.chdir(inno_dir)
        subprocess.run(["ISCC.exe", f"{plugin}WindowsBuildScript.iss", f"/O{output_dir}"])
        os.chdir("../../")

    elif platform.system() == 'Darwin':
        subprocess.run(["xcodebuild", "-project", f"Builds/MacOSX/{plugin}.xcodeproj", "-configuration", "Release", "-alltargets"])
        subprocess.run(["pkgbuild", 
                        "--root", f"./Builds/MacOSX/build/Release/{plugin}.component", 
                        "--identifier", f"com.yourcompany.{plugin}", 
                        "--install-location", "/Library/Audio/Plug-Ins/Components", 
                        f"../{output_dir}/{plugin}.pkg"])
        os.chdir("../")
    elif platform.system() == 'Linux':
        subprocess.run(["make", f"Builds/LinuxMakefile/make"])
        os.chdir("../")
    else:
        print("Unsupported operating system.")
