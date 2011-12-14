CC = h5cc

#CFLAGS = -O3 -Iapi -Iutility -DHDF5TEST
CFLAGS = -O3 -Iapi -Iutility -fno-common

#LDFLAGS = -lrdf -lhdf5_hl

LDFLAGS = -L/opt/local/lib -lrdf -lhdf5_hl \
          -dynamiclib -flat_namespace \
          -install_name /usr/local/lib/libbsml.dylib \
          -compatibility_version 1.0 -current_version 1.0.0

OBJS = \
  api/bsml_hdf5.o         \
  api/bsml_model.o        \
  api/bsml_rdf.o          \
  api/bsml_rdf_mapping.o  \
  api/bsml_internal.o     \
  api/bsml_data.o         \
  utility/values.o        \
  utility/list.o          \
  utility/dictionary.o    \
  ssf/cJSON/cJSON.o       \
  ssf/md5/md5.o           \
  ssf/ssf.o               \
  ssf/dict.o              \
  ssf/curl.o


all: lib
 
lib: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o /usr/local/lib/libbsml.dylib
 
clean:
	rm -f hdf5 api/*.o utility/*.o
 
.PHONY: clean
