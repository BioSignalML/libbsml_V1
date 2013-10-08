#include <iostream>

#include "rdf.h"

#include "recording.h"   // But we want just a single "biosignalml.h"
#include "signal.h"


int main(void)
/*==========*/
{

  bsml::Recording r("recording/uri") ;
  r.set_description("Something describing the recording.") ;

  bsml::Signal s("sig/uri") ;
  s.set_label("label...") ;
  s.set_comment("This is a test!") ;
  r.add_signal(&s) ;

  rdf::Graph g("graph/uri") ;
  std::cout << g.serialise() << "\n" ;
  r.to_rdf(g) ;

  }
