all:
	g++ main.cpp -o teste

run:
	./teste

clean:
	rm -f teste

zip:
	zip trabalho_SO.zip main.cpp corredor.hpp Makefile