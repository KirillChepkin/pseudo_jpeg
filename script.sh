#gcc "main.c" src/*.c -I "headers" -O2 -o pj &&
#./pj  compress "samples/screenshot.bmp" "outputs/output.pj"
make
#./pj --help
#./pj
#./pj compress "samples/sample.bmp" "outputs/output.pj"
./pj decompress "outputs/output.pj" "samples/decompressed_image.bmp"