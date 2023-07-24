import os
import subprocess
import platform

plugin_dir = "."
output_dir = "../../HayesInstallers"
inno_dir = "BuildScripts"
os.chdir(plugin_dir)

plugins = [d for d in os.listdir() if os.path.isdir(d)]

for plugin in plugins:
    if str(plugin) == "JUCE" or str(plugin) == "Common" or str(plugin) == "HayesInstallers" or str(plugin) == ".git":
        continue

    os.chdir(plugin)

    subprocess.run(["projucer", "--resave", f"{plugin}.jucer"])

    if platform.system() == 'Windows':
        subprocess.run(["msbuild", f"Builds/VisualStudio2019/{plugin}.sln", "/t:Build", "/p:Configuration=Release"])
        
        print(os.getcwd())
        os.chdir(inno_dir)
        subprocess.run(["ISCC.exe", f"{plugin}WindowsBuildScript.iss", f"/O{output_dir}"])
        
        os.chdir("../../")

    elif platform.system() == 'Darwin':
        subprocess.run(["xcodebuild", "-project", f"Builds/MacOSX/{plugin}.xcodeproj", "-configuration", "Release", "-alltargets"])
        
        home_dir = os.path.expanduser("~")
        plugin_path = os.path.join(home_dir, "Library/Audio/Plug-Ins/Components", f"{plugin}.component")
        pkg_path = f"../{output_dir}/{plugin}.pkg"
        subprocess.run(["pkgbuild", 
                "--root", plugin_path, 
                "--identifier", f"com.yourcompany.{plugin}", 
                "--install-location", "/Library/Audio/Plug-Ins/Components", 
                #"--scripts", "scripts_directory", 
                f"../{output_dir}/{plugin}.pkg"])

        os.chdir("../")
    elif platform.system() == 'Linux':
        subprocess.run(["make", f"Builds/LinuxMakefile/make"])
        os.chdir("../")
    else:
        print("Unsupported operating system.")
