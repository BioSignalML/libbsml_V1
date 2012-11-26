cc -o teststream -I. tests/websocket.c tests/print_json.c bsml_stream.c \
                     bsml_queue.o  bsml_string.o  bsml_log.o            \
                     -lwebsockets -lmhash -ljansson
