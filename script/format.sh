#!/bin/bash
SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd) 

if [ ! $(which clang-format) ]; then
  echo "Cannot find clang-format, Please install it!"
fi

