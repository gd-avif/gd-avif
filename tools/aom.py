import os


def aom_cmake_config(env):
    config = {
        "CMAKE_BUILD_TYPE": "%s" % ("RelWithDebInfo" if env["debug_symbols"] else "Release"),
        "BUILD_SHARED_LIBS:": "0",
        "ENABLE_DOCS": "0",
        "ENABLE_EXAMPLES": "0",
        "ENABLE_TESTDATA": "0",
        "ENABLE_TESTS": "0",
        "ENABLE_TOOLS": "0",
        "CMAKE_POSITION_INDEPENDENT_CODE": "1",
    }
    config["AOM_TARGET_CPU"] = {"x86_64": "x86_64", "x86_32": "x86", "arm64": "aarch64", "arm32": "arm"}[env["arch"]]
    return config


def build_library(env):
    aom_env = env.Clone()
    aom = env.CMake(
        [env.Dir(env["AOM_BUILD"])] + env["AOM_LIBS"],
        env.Dir(env["AOM_SOURCE"]),
        CMAKECONFFLAGS=["-D%s=%s" % it for it in aom_cmake_config(env).items()],
    )
    env.Append(LIBPATH=[env["AOM_BUILD"]])
    env.Append(CPPPATH=[env["AOM_INCLUDE"]])
    env.Prepend(LIBS=env["AOM_LIBS"])
    return aom


def exists(env):
    return "CMake" in env


def generate(env):
    env["AOM_SOURCE"] = env.Dir("#thirdparty/aom").abspath
    env["AOM_BUILD"] = env.Dir("#bin/thirdparty/aom/${platform}/${arch}").abspath
    env["AOM_INCLUDE"] = env["AOM_SOURCE"]
    libs = []
    if env["arch"] == "x86_64":
        libs += [
            "libaom_av1_encoder_sse2_static.a",
            "libaom_av1_encoder_ssse3_static.a",
            "libaom_dsp_common_sse2_static.a",
            "libaom_dsp_common_ssse3_static.a",
            "libaom_dsp_encoder_sse2_static.a",
            "libaom_dsp_encoder_ssse3_static.a",
        ]
    elif env["arch"] == "x86_32":
        libs += [
            "libaom_av1_encoder_sse2_static.a",
            "libaom_dsp_common_sse2_static.a",
            "libaom_dsp_common_ssse3_static.a",
            "libaom_dsp_encoder_sse2_static.a",
        ]
    libs = ["libaom_version.a", "libaom_pc.a"] + libs + ["libaom.a"]

    env["AOM_LIBS"] = [env.File("${AOM_BUILD}/" + lib) for lib in libs]
    env.AddMethod(build_library, "BuildAOM")
