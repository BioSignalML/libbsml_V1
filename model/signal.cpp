#include "rdf.h"
#include "rdfmap.h"
#include "recording.h"
#include "signal.h"


void bsml::Signal::set_recording(bsml::Recording *recording)
/*--------------------------------------------------------*/
{
  recording_ = recording ;
  rdfmap.push_back(new rdf::Mapping<rdf::Resource>(bsml::BSML::recording, recording->get_resource())) ;
  }


