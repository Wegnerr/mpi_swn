compile:
	mpicc src/main.c -lpthread -o main 
run:
	lamboot
	mpirun -np 4 main
clean:
	rm main
