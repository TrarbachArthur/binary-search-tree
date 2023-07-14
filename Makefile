all:
	gcc -o main *.c

run: all
	./main

test: all
	./main < test.txt

valgrind: all
	valgrind --leak-check=full ./main

valgrind-test: all
	valgrind --leak-check=full ./main < test.txt

clean:
	rm main