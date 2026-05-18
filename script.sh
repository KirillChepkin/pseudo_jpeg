gcc "main.c" "src/bmp.c" "src/compression.c" -I "headers" -O2 -o main &&
./main  compress "samples/logo.bmp" "outputs/downsampled_output.bmp"