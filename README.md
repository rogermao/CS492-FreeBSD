# CS492-FreeBSD
FreeBSD Senior Project @ UIUC


1/27 Compilation Instructions

make

sudo kldload ./main.ko

clang++ daemon.cpp -lutil -o daemon

./daemon

clang memhog.c -o memhog

sudo ./memhog [daemon pid] [signal #]

4/12

pkg install atf

pkg install kyua

clang -I/usr/local/include -L/usr/local/lib registration_test.cpp -latf-c++ -o registration_test 

kyua test
