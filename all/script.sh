gcc "main.c" "src/read_bmp.c" -I "headers" -O2 -o main
./main "samples/sample.bmp" "outputs/output.bmp"