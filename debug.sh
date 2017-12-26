gcc -std=gnu99 -Wall -DEBUG -g3 -c task.c -o task.o  -fno-stack-protector
gcc -std=gnu99 -Wall -DEBUG -g3 -c scheduling_simulator.c -o scheduling_simulator.o  -fno-stack-protector
gcc -std=gnu99 -Wall -DEBUG -g3 -o scheduling_simulator scheduling_simulator.o task.o  -fno-stack-protector
