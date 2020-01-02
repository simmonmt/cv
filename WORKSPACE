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
    commit = "ad904b6cd3906ddf79878003d92b7bc08d7786ae", 
    shallow_since = "1576788056 -0500",
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
