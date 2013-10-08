#include <string>

#include "recording.h"
#include "signal.h"


void bsml::Recording::add_signal(bsml::Signal *signal)
/*--------------------------------------------------*/
{
  const std::string uri = signal->get_uri_as_string() ;
  if (signals.count(uri) == 0) {
    signals.insert(std::pair<std::string, Signal *>(uri, signal)) ;
    signal->set_recording(this) ;
    }
  }


void bsml::Recording::to_rdf(const rdf::Graph &graph)
/*-------------------------------------------------*/
{
  std::map<std::string, Signal *>::iterator s ;
  AbstractObject::to_rdf(graph) ;
  for (s = signals.begin() ;  s != signals.end() ;  ++s) {
    s->second->to_rdf(graph) ;
    }
  }

