#!python

import os, sys, platform, json, subprocess

import builders


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

# Dependencies
deps_source_dir = "libs"
env.Append(
    BUILDERS={
        "BuildAOM": env.Builder(action=builders.aom_action, emitter=builders.aom_emitter),
        "BuildLibAvif": env.Builder(action=builders.avif_action, emitter=builders.avif_emitter),
    }
)

# AOM
aom = env.BuildAOM(env.Dir(builders.get_aom_build_dir(env)), env.Dir(builders.get_aom_source_dir(env)))

env.Prepend(CPPPATH=[builders.get_aom_include_dir(env)])
env.Prepend(LIBPATH=[builders.get_aom_build_dir(env)])
env.Append(LIBS=[aom])

# avif
avif = env.BuildLibAvif(env.Dir(builders.get_avif_build_dir(env)), [env.Dir(builders.get_avif_source_dir(env))] + aom)

env.Append(LIBPATH=[builders.get_avif_build_dir(env)])
env.Append(CPPPATH=[builders.get_avif_include_dir(env)])
env.Prepend(LIBS=[avif])

# Our includes and sources
env.Append(CPPPATH=["src/"])
sources = []
add_sources(sources, "src/", "cpp")
env.Depends(sources, [aom, avif])

# Make the shared library
result_path = os.path.join("bin", "gdavif")
result_name = "gdavif{}{}".format(env["suffix"], env["SHLIBSUFFIX"])
library = env.SharedLibrary(target=os.path.join(result_path, "lib", result_name), source=sources)
Default(library)

# GDNativeLibrary
extfile = env.InstallAs(os.path.join(result_path, "gdavif.gdextension"), "misc/gdavif.gdextension")
Default(extfile)
