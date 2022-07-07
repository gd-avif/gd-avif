import os
from SCons.Script import Environment


def get_cmake_build_type(env):
    if not env["debug_dependencies"]:
        return "Release"
    if env["optimize"] == "none":
        return "Debug"
    if env["debug_symbols"]:
        return "RelWithDebInfo"
    if env["optimize"] == "size":
        return "MinSizeRel"
    return "Release"


def get_android_api(env):
    return env["android_api_level"] if int(env["android_api_level"]) > 28 else "28"


def get_deps_dir(env):
    return env.Dir("#deps").abspath


def get_deps_build_dir(env):
    return get_deps_dir(env) + "/build/build{}.{}".format(env["suffix"], get_cmake_build_type(env))


def get_avif_source_dir(env):
    return get_deps_dir(env) + "/libavif"


def get_avif_build_dir(env):
    return get_deps_build_dir(env) + "/libavif"


def get_avif_include_dir(env):
    return get_avif_source_dir(env) + "/include"


def get_aom_source_dir(env):
    return get_deps_dir(env) + "/aom"


def get_aom_build_dir(env):
    return get_deps_build_dir(env) + "/aom"


def get_aom_install_dir(env):
    return get_aom_build_dir(env) + "/aom"


def get_aom_include_dir(env):
    return get_aom_source_dir(env) + "/"


def get_cmake_platform_flags(env):
    args = []
    if env["platform"] == "android":
        abi = {
            "arm64": "arm64-v8a",
            "arm32": "armeabi-v7a",
            "x86_32": "x86",
            "x86_64": "x86_64",
        }[env["arch"]]
        args.extend(
            [
                "-DCMAKE_SYSTEM_NAME=Android",
                "-DCMAKE_SYSTEM_VERSION=%s" % get_android_api(env),
                "-DCMAKE_ANDROID_ARCH_ABI=%s" % abi,
                "-DANDROID_ABI=%s" % abi,
                "-DCMAKE_TOOLCHAIN_FILE=%s/build/cmake/android.toolchain.cmake" % env["ANDROID_NDK_ROOT"],
                "-DCMAKE_ANDROID_STL_TYPE=c++_static",
            ]
        )

    elif env["platform"] == "linux":
        if env["arch"] == "x86_32":
            args.extend(["-DCMAKE_C_FLAGS=-m32"])
        else:
            args.extend(["-DCMAKE_C_FLAGS=-m64"])

    elif env["platform"] == "macos":
        if env["macos_deployment_target"] != "default":
            args.extend(["-DCMAKE_OSX_DEPLOYMENT_TARGET=%s" % env["macos_deployment_target"]])
        if env["arch"] == "x86_64":
            args.extend(["-DCMAKE_OSX_ARCHITECTURES=x86_64"])
        elif env["arch"] == "arm64":
            args.extend(["-DCMAKE_OSX_ARCHITECTURES=arm64"])
        else:
            raise ValueError("OSX architecture not supported: %s" % env["arch"])

    elif env["platform"] == "ios":
        if env["arch"] == "universal":
            raise ValueError("iOS architecture not supported: %s" % env["arch"])
        args.extend(["-DCMAKE_SYSTEM_NAME=iOS", "-DCMAKE_OSX_ARCHITECTURES=%s" % env["arch"]])
        if env["ios_min_version"] != "default":
            args.extend(["-DCMAKE_OSX_DEPLOYMENT_TARGET=%s" % env["ios_min_version"]])
        if env["ios_simulator"]:
            args.extend(["-DCMAKE_OSX_SYSROOT=iphonesimulator"])

    elif env["platform"] == "windows":
        if env["arch"] == "x86_32":
            if env["use_mingw"]:
                args.extend(
                    [
                        "-G 'Unix Makefiles'",
                        "-DCMAKE_C_COMPILER=i686-w64-mingw32-gcc",
                        "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++",
                        "-DCMAKE_SYSTEM_NAME=Windows",
                    ]
                )
        else:
            if env["use_mingw"]:
                args.extend(
                    [
                        "-G 'Unix Makefiles'",
                        "-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc",
                        "-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++",
                        "-DCMAKE_SYSTEM_NAME=Windows",
                    ]
                )
    return args


def aom_emitter(target, source, env):
    build_dir = get_aom_build_dir(env)
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
    libs = ["libaom_version.a"] + libs + ["libaom.a"]
    install_dir = get_aom_install_dir(env)
    return [env.File(build_dir + "/" + l) for l in libs], source


def aom_action(target, source, env):
    build_dir = get_aom_build_dir(env)
    source_dir = source[0].abspath

    aom_env = Environment()
    install_dir = get_aom_install_dir(env)
    args = [
        "-B",
        build_dir,
        "-DBUILD_SHARED_LIBS=0",
        "-DCMAKE_BUILD_TYPE=%s" % get_cmake_build_type(env),
        "-DENABLE_DOCS=0",
        "-DENABLE_EXAMPLES=0",
        "-DENABLE_TESTDATA=0",
        "-DENABLE_TESTS=0",
        "-DENABLE_TOOLS=0",
        "-DCMAKE_POSITION_INDEPENDENT_CODE=1",
    ]
    args.extend(get_cmake_platform_flags(env))
    if env["arch"] == "x86_64":
        args.append("-DAOM_TARGET_CPU=x86_64")
    elif env["arch"] == "x86_32":
        args.append("-DAOM_TARGET_CPU=x86")
    elif env["arch"] == "arm64":
        args.append("-DAOM_TARGET_CPU=aarch64")
    elif env["arch"] == "arm64":
        args.append("-DAOM_TARGET_CPU=arm")
    args.append(source_dir)
    jobs = env.GetOption("num_jobs")
    aom_env.Execute(
        [
            "cmake " + " ".join(args),
            "cmake --build %s -j%s" % (build_dir, jobs),
        ]
    )
    return None


def avif_emitter(target, source, env):
    build_dir = get_avif_build_dir(env)
    return [get_avif_build_dir(env) + "/libavif.a"], source


def avif_action(target, source, env):
    build_dir = get_avif_build_dir(env)
    source_dir = source[0].abspath
    args = [
        "-B",
        build_dir,
        "-DBUILD_SHARED_LIBS=0",
        "-DAVIF_CODEC_AOM=1",
        "-DAOM_LIBRARY=%s/libaom.a" % get_aom_build_dir(env),
        "-DAOM_INCLUDE_DIR=%s" % get_aom_include_dir(env),
        "-DCMAKE_POSITION_INDEPENDENT_CODE=1",
        "-DCMAKE_BUILD_TYPE=%s" % get_cmake_build_type(env),
    ]
    args.extend(get_cmake_platform_flags(env))
    args.append(source_dir)
    jobs = env.GetOption("num_jobs")
    avif_env = Environment()
    avif_env.Execute(
        [
            "cmake " + " ".join(args),
            "cmake --build %s -j%s" % (build_dir, jobs),
        ]
    )
    return None
