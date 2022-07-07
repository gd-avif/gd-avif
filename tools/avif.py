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
    return env.CMakePlatformFlags(config)


def avif_emitter(target, source, env):
    env.Depends(env["AVIF_LIBS"], env["AOM_LIBS"])
    env.Depends(
        env["AVIF_LIBS"],
        [env.File(__file__), env.Dir(env["AVIF_SOURCE"]), env.File(env["AVIF_SOURCE"] + "/CMakeLists.txt")],
    )
    return env["AVIF_LIBS"], env.Dir(env["AVIF_SOURCE"])


def avif_action(target, source, env):
    avif_env = env.Clone()
    build_dir = env["AVIF_BUILD"]
    source_dir = env["AVIF_SOURCE"]
    opts = avif_cmake_config(avif_env)
    avif_env.CMakeConfigure(source_dir, build_dir, ["-D%s=%s" % it for it in opts.items()])
    avif_env.CMakeBuild(build_dir)
    return None


def exists(env):
    return "CMakeConfigure" in env and "CMakeBuild" in env


def generate(env):
    env["AVIF_SOURCE"] = env["DEPS_SOURCE"] + "/libavif"
    env["AVIF_BUILD"] = env["DEPS_BUILD"] + "/libavif"
    env["AVIF_INCLUDE"] = env["AVIF_SOURCE"] + "/include"
    env["AVIF_LIBS"] = [
        env.File(env["AVIF_BUILD"] + "/" + lib)
        for lib in [
            "libavif.a",
        ]
    ]
    env.Append(BUILDERS={"BuildLibAvif": env.Builder(action=avif_action, emitter=avif_emitter)})
    env.Append(LIBPATH=[env["AVIF_BUILD"]])
    env.Append(CPPPATH=[env["AVIF_INCLUDE"]])
    env.Prepend(LIBS=env["AVIF_LIBS"])
