load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

new_local_repository(
    name = "opencv",
    path = "/usr/local/Cellar/opencv/4.1.2",
    build_file = "BUILD.opencv",
)

git_repository(
    name = "com_google_absl",
    tag = "20190808",
    remote = "https://github.com/abseil/abseil-cpp.git",
)

http_archive(
    name = "com_google_googletest",
    url = "https://github.com/google/googletest/archive/release-1.10.0.zip",
    strip_prefix="googletest-release-1.10.0",
    sha256 = "94c634d499558a76fa649edb13721dce6e98fb1e7018dfaeba3cd7a083945e91",
)
