#!/bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Building...\nCurrent directory is $DIR"
PATH=$PATH:/usr/local/bin
echo "PATH is $PATH\nStart build results:\n"

/usr/local/bin/node-gyp configure
/usr/local/bin/node-gyp build
/usr/local/bin/node test.js