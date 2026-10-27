# Development environment for the mysh assignment.
# Ubuntu 24.04 has the same toolchain the tests were verified with.
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential gdb valgrind strace make git procps ca-certificates \
 && rm -rf /var/lib/apt/lists/*

# The base image already has a non-root user "ubuntu" (uid 1000). The tests must
# NOT run as root: one of them checks that a file without execute permission
# gives exit status 126, which root can bypass.
USER ubuntu
WORKDIR /workspaces/mysh
