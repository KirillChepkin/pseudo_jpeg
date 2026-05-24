#gcc "main.c" src/*.c -I "headers" -O2 -o pj &&
#./pj  compress "samples/screenshot.bmp" "outputs/output.pj"
make
./pj compress "samples/screenshot.bmp" "outputs/output.pj"