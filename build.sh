#!/bin/bash
#
# @file build.sh
# @brief A simple script to compile the Snake game using the Makefile.
# @author chmodxChironex
# @date 2025

set -e

echo "Building the Snake game..."
make

echo "Build process finished. Run with './snake_game' or 'make run'."