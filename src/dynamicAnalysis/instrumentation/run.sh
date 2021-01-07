make
./func_instrument ./hello foo2
echo "orginal run"
./hello
echo "instrumented run"
./newbinary
