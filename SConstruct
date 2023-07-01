#!python

import os, sys, platform, json, subprocess

import SCons


# Minimum target platform versions.
if "ios_min_version" not in ARGUMENTS:
    ARGUMENTS["ios_min_version"] = "11.0"
if "macos_deployment_target" not in ARGUMENTS:
    ARGUMENTS["macos_deployment_target"] = "10.9"
if "android_api_level" not in ARGUMENTS:
    ARGUMENTS["android_api_level"] = "21"

env = SConscript("godot-cpp/SConstruct").Clone()

opts = Variables([], ARGUMENTS)

# Dependencies
for tool in ["cmake", "aom", "avif"]:
    env.Tool(tool, toolpath=["tools"])

opts.Update(env)

result_path = os.path.join("bin", "gdavif")

# Our includes and sources
env.Append(CPPDEFINES=["GDEXTENSION"])  # Tells our sources we are building a GDExtension, not a module.
sources = [
    "register_types.cpp",
    "image_loader_avif.cpp",
    "resource_saver_avif.cpp",
]

# Make our dependencies
aom = env.BuildAOM()
avif = env.BuildLibAvif(aom)

# Make the shared library
result_name = "gdavif{}{}".format(env["suffix"], env["SHLIBSUFFIX"])
library = env.SharedLibrary(target=os.path.join(result_path, "lib", result_name), source=sources)

Default(library)

# GDNativeLibrary
extfile = env.InstallAs(os.path.join(result_path, "gdavif.gdextension"), "misc/gdavif.gdextension")
Default(extfile)
