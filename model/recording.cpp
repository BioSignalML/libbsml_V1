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


