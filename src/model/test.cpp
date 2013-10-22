#include <iostream>

#include "rdf/rdf.h"
#include "biosignalml.h"

using namespace bsml ;


void cleanup(void)
/*==============*/
{
  rdf::end_world() ;
  }


int main(void)
/*==========*/
{
  atexit(cleanup) ;

  std::list<rdf::Prefix> prefixes(1, rdf::Prefix("bsml", BSML::uri)) ;

  Recording r("recording/uri") ;
  r.set_description("Something describing the recording.") ;

  Signal s1("uri/sig1", "mV") ;
  s1.set_label("Pressure") ;
  s1.set_comment("This is a test!") ;

  r.add_signal(&s1) ;


  Signal s2 = r.new_signal("uri/sig2", unit::Millivolt) ;
  s2.set_label("ECG1") ;




  rdf::Graph g("graph/uri") ;
  r.to_rdf(g) ;
  std::cout << g.serialise(prefixes) << std::endl ;

  }
