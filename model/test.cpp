#include <iostream>

#include "rdf.h"

#include "signal.h"


int main(void)
/*==========*/
{

  bsml::Signal s("sig/uri") ;
  s.set_label("label...") ;

  rdf::Graph g("graph/uri") ;
  s.to_rdf(g) ;
  std::cout << g.serialise() << "\n" ;

  }
