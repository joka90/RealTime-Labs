pushd .
cd ../../simple_os
./make_lib_32
popd
make -f Makefile_32 clean
make -f Makefile_32
