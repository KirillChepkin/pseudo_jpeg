gcc "main.c" "src/read_bmp.c" -I "headers" -O2 -o main &&
./main  compress "samples/sample.bmp" "outputs/output.bmp"