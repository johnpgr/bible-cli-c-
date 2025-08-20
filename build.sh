set -e

CC_FLAGS="-std=c++23 -g -Wall -Wextra -Wpedantic -Wno-writable-strings"

mkdir -p build

echo "Compiling application"

clang++ $CC_FLAGS -o build/bible src/main.cpp

echo "Compiled successfully"
echo "Executable: build/bible"

chmod +x build/bible

