import os


def avif_cmake_config(env):
    config = {
        "BUILD_SHARED_LIBS": "0",
        "AVIF_CODEC_AOM": "1",
        "AOM_LIBRARY": env["AOM_BUILD"] + "/libaom.a",
        "AOM_INCLUDE_DIR": env["AOM_INCLUDE"],
        "CMAKE_POSITION_INDEPENDENT_CODE": "1",
        "CMAKE_BUILD_TYPE": "%s" % ("RelWithDebInfo" if env["debug_symbols"] else "Release"),
    }
    return config


def build_library(env, aom):
    avif = env.CMake(
        [env.Dir(env["AVIF_BUILD"])] + env["AVIF_LIBS"],
        [env.Dir(env["AVIF_SOURCE"])] + aom,
        CMAKECONFFLAGS=["-D%s=%s" % it for it in avif_cmake_config(env).items()],
    )
    env.Append(LIBPATH=[env["AVIF_BUILD"]])
    env.Append(CPPPATH=[env["AVIF_INCLUDE"]])
    env.Prepend(LIBS=env["AVIF_LIBS"])
    return avif


def exists(env):
    return "CMakeConfigure" in env and "CMakeBuild" in env


def generate(env):
    env["AVIF_BUILD"] = env.Dir("#bin/thirdparty/libavif/${platform}/${arch}").abspath
    env["AVIF_SOURCE"] = env.Dir("#thirdparty/libavif").abspath
    env["AVIF_INCLUDE"] = env.Dir("${AVIF_SOURCE}/include").abspath
    env["AVIF_LIBS"] = [env.File("${AVIF_BUILD}/libavif.a")]
    env.AddMethod(build_library, "BuildLibAvif")
