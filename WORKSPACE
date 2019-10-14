load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

new_local_repository(
    name = "opencv",
    path = "/usr/local/Cellar/opencv/4.1.2",
    build_file = "BUILD.opencv",
)

git_repository(
    name   = "com_github_gflags_gflags",
    #tag    = "v2.2.0",
    commit = "30dbc81fb5ffdc98ea9b14b1918bfe4e8779b26e", # v2.2.0 + fix of include path
    remote = "https://github.com/gflags/gflags.git"
)

git_repository(
    name = "com_google_absl",
    tag = "20190808",
    remote = "https://github.com/abseil/abseil-cpp.git",
)
