bsml_signal *bsml_signal_alloc(const char *uri)
/*===========================================*/
{
  bsml_signal *sig = ALLOCATE(bsml_signal) ;
  if (sig) {
    sig->uri = bsml_string_copy(uri) ;
    sig->metadata = bsml_dictionary_alloc() ;
    }
  return sig ;
  }


void bsml_signal_free(bsml_signal *sig)
/*===================================*/
{
  if (sig) {
    bsml_string_free(sig->uri) ;
    bsml_string_free(sig->label) ;
    bsml_string_free(sig->description) ;
    bsml_dictionary_free(sig->metadata) ;
    free(sig) ;
    }
  }


void bsml_signal_set_description(bsml_signal *sig, const char *desc)
/*================================================================*/
{
  bsml_string_free(sig->description) ;
  sig->description = bsml_string_copy(desc) ;
  }

void bsml_signal_set_label(bsml_signal *sig, const char *lbl)
/*=========================================================*/
{
  bsml_string_free(sig->label) ;
  sig->label = bsml_string_copy(lbl) ;
  }


bsml_dictionary *bsml_signal_build_metadata(bsml_signal *sig)
/*==========================================================*/
{
  bsml_dictionary *md = bsml_dictionary_create() ;



  bsml_dictionary_set_double(md, "rate", sig->rate) ;

  sig->metadata = md ; // ???
  }


bsml_dictionary *bsml_signal_get_metadata(bsml_signal *sig, const char *key)
/*========================================================================*/
{
  bsml_dictionary *md = bsml_dictionary_create() ;



  bsml_dictionary_set_double(md, "rate", sig->rate) ;

  }



static int append_data(bsml_timeseries *ts, void *ud)
/*=================================================*/
{

  bsml_log_debug("RCVD %d data values from %s starting at %g at rate %g\n",
    ts->length, ts->uri, bsml_time_as_seconds(ts->start), ts->rate) ;

  int taken = bsml_datasegment_extend((bsml_datasegment *)ud, ts) ;

  return (ts->length > 0 && taken == 0) ;   // Segment is full
  }



// When do we use bsml_time instead of double ??

bsml_datasegment *bsml_signal_read(bsml_signal *sig, double start, double duration)
{



  bsml_datasegment *ds = bsml_datasegment_alloc(sig->uri, start,
                                                    (size_t)(duration*sig->rate)
                                                     sig->rate)

  bsml_repository_get_timeseries(sig->repository, sig->uri, start, duration, append_data, ds) ;


  return ds ;
  }


bsml_timeseries *read_signal(bsml_repository *repo, const char *uri)
/*================================================================*/
{
  //bsml_recording rec = bsml_repository_get_recording_by_id(repo, "demo/1") ;
  //bsml_signal signal = bsml_recording_get_signal_by_id(rec, "id2") ;

  // Select a signal directly by its URI:
  bsml_signal *signal = bsml_repository_get_signal(repo, uri) ;
  //bsml_signal signal = bsml_repository_get_signal_by_id(repo, "demo/1/signal/id2") ;

  // Print some information about the recording and signal:
  const char *units = bsml_units_as_string(signal->units) ;
  printf("%s, %s, %s, %s\n", signal->recording->description, signal->uri, signal->label, units) ;
  bsml_string_free(units) ;
    
  // Retrieve and print time series data for the first second of the signal:
  bsml_signal_read_double(signal, 0.0, 1.0, print_data) ;

  bsml_signal_free(signal) ;
  }


