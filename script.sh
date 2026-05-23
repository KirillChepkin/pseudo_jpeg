gcc "main.c" src/*.c -I "headers" -O2 -o pj &&
./pj  compress "samples/sample.bmp" "outputs/output.pj"