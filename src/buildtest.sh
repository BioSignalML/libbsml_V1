cc -o teststream -I. tests/websocket.c bsml_stream.c \
                     bsml_queue.o  bsml_string.o  bsml_log.o            \
                     -lwebsockets -lmhash -ljansson
