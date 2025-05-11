obj-m += lifo_driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc test_lifo.c -o test_lifo.out

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm test_lifo.out