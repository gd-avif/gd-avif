def can_build(env, platform):
    # TODO proper detection of tested platforms.
    return True


def get_opts(platform):
    return []


def configure(env):
    pass


def get_doc_classes():
    return [
        "ImageLoaderAVIF",
        "ResourceSaverAVIF",
    ]


def get_doc_path():
    return "doc_classes"
