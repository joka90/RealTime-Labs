pushd .
cd ../../simple_os
./make_lib_arm
popd
make -f Makefile_arm_bb clean
make -f Makefile_arm_bb


