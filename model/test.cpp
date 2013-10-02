#include <iostream>

#include "rdf.h"

#include "signal.h"


int main(void)
/*==========*/
{
  rdf::Graph g = rdf::Graph("graph/uri") ;


  bsml::Signal s = bsml::Signal("sig/uri") ;

  s.set_label("label...") ;

  s.to_rdf(g) ;

  std::cout << g.serialise() << "\n" ;

  }
