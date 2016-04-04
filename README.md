# CS492-FreeBSD
FreeBSD Senior Project @ UIUC

1/27 Compilation Instructions
make
sudo kldload ./main.ko
clang++ daemon.cpp -lutil -o daemon
./daemon
clang memhog.c -o memhog
sudo ./memhog [daemon pid] [signal #]

clang -o registration_test -I/usr/local/unclude -L/usr/local/lib registration_test.c -latf-c 
kyua test
