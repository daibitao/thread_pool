OBJS := test.o thread_pool.o 
HEADS := thread_pool.h
SRCS := test.c thread_pool.c
LIBS := -lpthread
 
test : $(OBJS)
	gcc $(OBJS) -o test $(LIBS)
	
$(OBJS) : $(HEADS) $(SRCS)
	gcc -c $(HEADS) $(SRCS)
	
clean :
	rm -rf $(OBJS) 