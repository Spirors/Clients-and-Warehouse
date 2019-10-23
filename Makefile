all: main.x

main.x:
	gcc src/client.c src/client_ds.c -I./inc -o client -lpthread
	gcc src/warehouse_db.c src/warehouse_ds.c -I./inc -o warehouse_db -lpthread

clean:
	-rm client
	-rm warehouse_db
