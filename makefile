compile:
	mpicc src/main.c src/lib/mechanics.c -lpthread -o main 
run:
	lamboot
	mpirun -np 4 main
clean:
	rm main
