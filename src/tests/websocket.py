import logging
import numpy as np

import biosignalml.rdf as rdf
import biosignalml.transports.stream as stream
import biosignalml.transports.websockets as ws


def get_data(endpoint, signals, start, duration):
#================================================
  for d in ws.WebStreamReader(endpoint, signals, start=start, duration=duration):
    if   d.type == stream.BlockType.INFO:
      print d
    elif d.type == stream.BlockType.DATA:
      print d.signaldata()


class DataSink(object):
#======================

  def __init__(self, endpoint, recording, signals, units, rates, channels=None):
  #-----------------------------------------------------------------------------
    self.ws = ws.WebStreamWriter(endpoint)
    self.ws.write_block(stream.InfoBlock(recording=recording, signals=signals,
                                         units=units, rates=rates, channels=channels))
    self.rates = rates
    self.blockno = 0

  def close(self):
  #---------------
    self.ws.close()

  def send_rdf(self, graph):
  #-------------------------
    self.ws.write_block(stream.RDFBlock(graph.serialise(rdf.Format.RDFXML), rdf.Format.RDFXML))

  def send_data(self, channel, start, data):
  #-----------------------------------------
    self.ws.write_block(stream.SignalDataBlock(self.blockno,
      dict(info=channel, start=start, rate=self.rates[channel],
           count=len(data), dtype=str(data.dtype.descr[0][1])),
      data))
    self.blockno += 1



def put_data(endpoint, recording):
#=================================
  # get following from a header...
  signals = None  # ==> use channel number to construct URI
  units = None
  rates = [1000, 1000]
  channels = 2

  ds = DataSink(endpoint, recording, None, units, rates, channels=channels)

  s0 = np.array([ -0.2,  -0.2,   -0.2,   -0.2,   -0.2,   -0.2,   -0.2,   -0.2,   -0.19,  -0.175,
                  -0.19,  -0.205, -0.21,  -0.2,   -0.195, -0.21,  -0.23,  -0.235, -0.245])
  s1 = np.array([ 0.005,  0.005,  0.005,  0.005,  0.005,  0.005,  0.005,  0.005,  0.005,  0.005,
                  0.005, -0.005, -0.005,  0.01,  -0.01,  -0.01,  -0.02,  -0.035, -0.025])

  ds.send_data(0, 0.0, s0)
  ds.send_data(1, 0.0, s1)


  g = rdf.Graph.create_from_string("http://example.org",
    """@prefix bsml: <http://www.biosignalml.org/ontologies/2011/04/biosignalml#> .
  
       <%(proc)s> a bsml:Process ;
         bsml:version "%(ver)s" .
       <%(rec)s> bsml:process <%(proc)s> .
    """ % dict(rec=recording, proc="http://example.org/some/process", ver="1.0"),
    rdf.Format.TURTLE)
  
  ds.send_rdf(g)
  ds.close()



# If directory is git controlled:
#   check status of processing scripts (or directory ??)
#     Add/commit scripts (directory) if modified, aborting if errors
#   get revision's SHA1 and use this to set version of script
#
#     <scriptuuid> a bsml:Process ;  bsml:revision "xxx" ;  bsml:baseprocess <script> .
#     <rec> bsml:process <scriptuuid> ;  bsml:runtime "yyy" .
#
#   Or a more complex model ???
#
# And if not git controlled??


if __name__ == '__main__':
#=========================
#  logging.getLogger().setLevel(logging.DEBUG)
##  get_data("http://devel.biosignalml.org/stream/data/",
##           "http://devel.biosignalml.org/resource/physiobank/mitdb/102", 0.0, 0.05)

  put_data("http://devel.biosignalml.org/stream/data/",
           "http://devel.biosignalml.org/resource/stream/test/1")

"""
  int CHUNKSIZE = 10000 ;

    else if (sb->type == BSML_STREAM_INFO_BLOCK) {
      if (siguris) json_decref(siguris) ;
      siguris = json_object_get(sb->header, "signals") ;
      // Also have "channels" field (plus "rates" and "units").
      if (siguris && json_is_array(siguris)) {
        size_t n = json_array_size(siguris) ;
        json_incref(siguris) ;
        if (channels != n) {
          channels = n ;
          if (data) free(data) ;
          data = calloc(sizeof(double), channels*CHUNKSIZE) ;
          }
        }
      }

    else if (sb->type == BSML_STREAM_DATA_BLOCK) {
      // Need to check rates match...
      // Or have rates in Info block and check when it's received...
      // What about signals where the rate changes??
      json_t *info = json_object_get(sb->header, "info") ;
      if (info && json_is_integer(info)) {
        int chan = json_integer_value(info) ;
        double *dp = data + chan ;
        double *cp = (double *)sb->content ;
        int l = sb->length/sizeof(double) ;
        while (l > 0) {
          *dp = *cp ;
          dp += channels ;
          ++cp ;
          --l ;
          }
        if (chan == (channels-1)) {
          double *dp = data ;
          int i ;
          l = sb->length/sizeof(double) ;
          while (l > 0) {
            printf("%d ", frameno) ;
            for (i = 0 ;  i < channels ;  ++i) {
              if (i > 0) printf(" ") ;
              printf("%f", *dp) ;
              ++dp ;
              }
            printf("\n") ;
            ++frameno ;
            --l ;
            }
          }
        }
      }

"""
