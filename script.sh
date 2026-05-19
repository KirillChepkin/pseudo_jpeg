gcc "main.c" "src/bmp.c" "src/compression.c" -I "headers" -O2 -o main &&
./main  compress "samples/desert.bmp" "outputs/output.bmp"