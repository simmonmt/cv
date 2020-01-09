load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

new_local_repository(
    name = "opencv",
    path = "/usr/local/Cellar/opencv/4.1.2",
    build_file = "BUILD.opencv",
)

git_repository(
    name = "com_google_absl",
    remote = "https://github.com/abseil/abseil-cpp.git",
    # To update: comment out commit and shallow_since, uncomment branch,
    # build, update commit and shallow_since per the debug warnings.
    commit = "63ee2f8877915a3565c29707dba8fe4d7822596a", 
    shallow_since = "1578426644 -0500",
    # branch = "master",
)

git_repository(
    name = "com_google_googletest",
    remote = "https://github.com/google/googletest.git",
    # To update: comment out commit and shallow_since, uncomment branch,
    # build, update commit and shallow_since per the debug warnings.
    commit = "306f3754a71d6d1ac644681d3544d06744914228", 
    shallow_since = "1578001778 -0500",
    # branch = "master",
)

http_archive(
    name = "io_bazel_rules_go",
    urls = [
        "https://storage.googleapis.com/bazel-mirror/github.com/bazelbuild/rules_go/releases/download/v0.20.3/rules_go-v0.20.3.tar.gz",
        "https://github.com/bazelbuild/rules_go/releases/download/v0.20.3/rules_go-v0.20.3.tar.gz",
    ],
    sha256 = "e88471aea3a3a4f19ec1310a55ba94772d087e9ce46e41ae38ecebe17935de7b",
)

load("@io_bazel_rules_go//go:deps.bzl", "go_rules_dependencies", "go_register_toolchains")

go_rules_dependencies()

go_register_toolchains()
