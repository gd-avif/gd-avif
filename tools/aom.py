import os


def aom_cmake_config(env):
    config = {
        "BUILD_SHARED_LIBS:": "0",
        "ENABLE_DOCS": "0",
        "ENABLE_EXAMPLES": "0",
        "ENABLE_TESTDATA": "0",
        "ENABLE_TESTS": "0",
        "ENABLE_TOOLS": "0",
        "CMAKE_POSITION_INDEPENDENT_CODE": "1",
        "CMAKE_BUILD_TYPE": "%s" % ("RelWithDebInfo" if env["debug_symbols"] else "Release"),
    }
    config["AOM_TARGET_CPU"] = {"x86_64": "x86_64", "x86_32": "x86", "arm64": "aarch64", "arm32": "arm"}[env["arch"]]
    return env.CMakePlatformFlags(config)


def aom_emitter(target, source, env):
    env.Depends(
        env["AOM_LIBS"],
        [env.File(__file__), env.Dir(env["AOM_SOURCE"]), env.File(env["AOM_SOURCE"] + "/CMakeLists.txt")],
    )
    return env["AOM_LIBS"], env.Dir(env["AOM_SOURCE"])


def aom_action(target, source, env):
    aom_env = env.Clone()
    build_dir = env["AOM_BUILD"]
    source_dir = env["AOM_SOURCE"]
    opts = aom_cmake_config(aom_env)
    aom_env.CMakeConfigure(source_dir, build_dir, ["-D%s=%s" % it for it in opts.items()])
    aom_env.CMakeBuild(build_dir)
    return None


def exists(env):
    return "CMakeConfigure" in env and "CMakeBuild" in env


def generate(env):
    env["AOM_SOURCE"] = env["DEPS_SOURCE"] + "/aom"
    env["AOM_BUILD"] = env["DEPS_BUILD"] + "/aom"
    env["AOM_INCLUDE"] = env["AOM_SOURCE"]
    libs = []
    if env["arch"] == "x86_64":
        libs += [
            "libaom_dsp_common_ssse3.a",
            "libaom_av1_encoder_ssse3.a",
            "libaom_dsp_encoder_ssse3.a",
            "libaom_dsp_common_sse2.a",
            "libaom_av1_encoder_sse2.a",
            "libaom_dsp_encoder_sse2.a",
        ]
    elif env["arch"] == "x86_32":
        libs += [
            "libaom_dsp_common_ssse3.a",
            "libaom_dsp_common_sse2.a",
            "libaom_av1_encoder_sse2.a",
            "libaom_dsp_encoder_sse2.a",
        ]
    libs = ["libaom_version.a", "libaom_pc.a"] + libs + ["libaom.a"]

    env["AOM_LIBS"] = [env.File(env["AOM_BUILD"] + "/" + lib) for lib in libs]
    env.Append(BUILDERS={"BuildAOM": env.Builder(action=aom_action, emitter=aom_emitter)})
    env.Append(LIBPATH=[env["AOM_BUILD"]])
    env.Append(CPPPATH=[env["AOM_INCLUDE"]])
    env.Prepend(LIBS=env["AOM_LIBS"])
