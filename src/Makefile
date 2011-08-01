CC = h5cc

CFLAGS = -O3 -Iapi -Iutility -DHDF5TEST -lhdf5_hl

LDFLAGS = -lrdf

all: hdf5
 
hdf5:   \
  api/bsml_hdf5.o         \
  api/bsml_model.o        \
  api/bsml_rdf.o          \
  api/bsml_rdf_mapping.o  \
  api/bsml_internal.o     \
  utility/values.o        \
  utility/list.o          \
  utility/dictionary.o
	$(CC) $(LDFLAGS) $^ -o $@ 

clean:
	rm -f hdf5 api/*.o utility/*.o
 
.PHONY: clean
