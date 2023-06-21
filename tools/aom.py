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


def build_library(env, jobs=None):
    if jobs is None:
        jobs = int(env.GetOption("num_jobs"))

    # Since AOM does not support macOS universal binaries, we first need to build the two libraries
    # separately, then we join them together using lipo.
    if env["platform"] == "macos" and env["arch"] == "universal":
        build_envs = {
            "x86_64": env.Clone(),
            "arm64": env.Clone(),
        }
        arch_aom = []
        for arch in build_envs:
            benv = build_envs[arch]
            benv["arch"] = arch
            generate(benv)
            benv["CMAKEBUILDJOBS"] = max([1, int(jobs / len(build_envs))])
            aom = benv.CMake(
                [benv.Dir("${AOM_BUILD}")] + benv["AOM_LIBS"],
                benv.Dir("${AOM_SOURCE}"),
                CMAKECONFFLAGS=["-D%s=%s" % it for it in aom_cmake_config(benv).items()],
            )
            arch_aom.extend(aom)
            benv.NoCache(aom)  # Needs refactoring to properly cache generated headers.

        aom = []
        common = ["libaom_version.a", "libaom_pc.a", "libaom.a"]
        x64_libs = filter(lambda lib: not any([str(lib).endswith(c) for c in common]), build_envs["x86_64"]["AOM_LIBS"])
        # Join libraries using lipo.
        lipo_action = "lipo $SOURCES -create -output $TARGET"

        # First the x86-only libraries ("fake" fat libraries).
        for lib in x64_libs:
            aom += env.Command(env.File("${AOM_BUILD}/" + os.path.basename(str(lib))), lib, lipo_action)

        # Then the actual fat ones.
        for lib in common:
            aom += env.Command(
                env.File("${AOM_BUILD}/" + lib),
                [benv.File("${AOM_BUILD}/" + lib) for benv in build_envs.values()],
                lipo_action,
            )
        env.Depends(aom, arch_aom)

    else:
        aom = env.CMake(
            [env.Dir("${AOM_BUILD}")] + env["AOM_LIBS"],
            env.Dir("${AOM_SOURCE}"),
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
    if env["arch"] == "x86_64" or (env["platform"] == "macos" and env["arch"] == "universal"):
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
