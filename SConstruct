#!python

import os, sys, platform, json, subprocess

import SCons


def add_sources(sources, dirpath, extension):
    for f in os.listdir(dirpath):
        if f.endswith("." + extension):
            sources.append(dirpath + "/" + f)


ARGUMENTS["ios_min_version"] = "11.0"
ARGUMENTS["macos_deployment_target"] = "10.9"
env = SConscript("godot-cpp/SConstruct").Clone()

opts = Variables([], ARGUMENTS)
opts.Add(BoolVariable("debug_dependencies", "Build dependencies respecting godot-cpp debug options", False))
opts.Update(env)

result_path = os.path.join("bin", "gdavif")

# Our includes and sources
env.Append(CPPPATH=["src/"])
sources = []
add_sources(sources, "src/", "cpp")

mac_universal = env["platform"] == "macos" and env["arch"] == "universal"
build_targets = []
build_envs = [env]

# For macOS universal builds, setup one build environment per architecture.
if mac_universal:
    build_envs = []
    for arch in ["x86_64", "arm64"]:
        benv = env.Clone()
        benv["arch"] = arch
        benv["CCFLAGS"] = SCons.Util.CLVar(str(benv["CCFLAGS"]).replace("-arch x86_64 -arch arm64", "-arch " + arch))
        benv["LINKFLAGS"] = SCons.Util.CLVar(
            str(benv["LINKFLAGS"]).replace("-arch x86_64 -arch arm64", "-arch " + arch)
        )
        benv["suffix"] = benv["suffix"].replace("universal", arch)
        benv["SHOBJSUFFIX"] = benv["suffix"] + benv["SHOBJSUFFIX"]
        build_envs.append(benv)

for benv in build_envs:
    # Dependencies
    for tool in ["cmake", "common", "aom", "avif"]:
        benv.Tool(tool, toolpath=["tools"])

    aom = benv.BuildAOM()
    avif = benv.BuildLibAvif()

    benv.Depends(sources, [aom, avif])

    # Make the shared library
    result_name = "gdavif{}{}".format(benv["suffix"], benv["SHLIBSUFFIX"])
    library = benv.SharedLibrary(target=os.path.join(result_path, "lib", result_name), source=sources)
    build_targets.append(library)

Default(build_targets)

# For macOS universal builds, join the libraries using lipo.
if mac_universal:
    result_name = "libgdavif{}{}".format(env["suffix"], env["SHLIBSUFFIX"])
    universal_target = env.Command(
        os.path.join(result_path, "lib", result_name), build_targets, "lipo $SOURCES -output $TARGETS -create"
    )
    Default(universal_target)

# GDNativeLibrary
extfile = env.InstallAs(os.path.join(result_path, "gdavif.gdextension"), "misc/gdavif.gdextension")
Default(extfile)
