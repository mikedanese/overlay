package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "overlay",
    srcs = [ "main.c" ],
    deps = [
        ":liboverlay",
    ],
)

cc_library(
    name = "liboverlay",
    hdrs = [
        "actions.h",
        "overlay.h",
        "util.h",
    ],
    srcs = [
        "actions.c",
        "util.c",
    ],
    deps = [
        "@jsonnet//core:libjsonnet",
        "@jansson//:jansson",
    ],
)
