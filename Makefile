all:
	gcc *.c -o s3
	./s3

clean:
	rm -f s3
	