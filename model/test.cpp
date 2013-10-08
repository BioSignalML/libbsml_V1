#include <iostream>

#include "rdf.h"

#include "recording.h"   // But we want just a single "biosignalml.h"
#include "signal.h"


int main(void)
/*==========*/
{

  std::list<rdf::Prefix> prefixes(1, rdf::Prefix("bsml", bsml::BSML::uri)) ;

  bsml::Recording r("recording/uri") ;
  r.set_description("Something describing the recording.") ;

  bsml::Signal s("sig/uri") ;
  s.set_label("label...") ;
  s.set_comment("This is a test!") ;
  r.add_signal(&s) ;

  rdf::Graph g("graph/uri") ;
  r.to_rdf(g) ;
  std::cout << g.serialise(prefixes) << "\n" ;

  rdf::end_world() ;
  }
