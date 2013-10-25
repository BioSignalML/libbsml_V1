#ifndef _BSML_RECORDING_H
#define _BSML_RECORDING_H

#include <string>
#include <map>

#include "model/bsml.h"
#include "model/object.h"
#include "model/datetime.h"
#include "model/units.h"

#include "rdf/rdf.h"
#include "rdf/rdfmap.h"


namespace bsml {

  class Signal ;

  class Recording : public AbstractObject
  /*===================================*/
  {
   private:
    std::map<std::string, Signal *> signals_ ;

    rdf::Literal format_ ;

//String ??         'format'
//Node              'dataset'
//Node              'source'          multiple
//Node              'investigation'
//Node              'investigator'
    Datetime starttime_ ;
    Duration duration_ ;
//Resource      'timeline'            subelement
//Resource      'generatedBy'         subelement

   public:
    Recording(void) ;
    Recording(const std::string &uri) ;
    virtual ~Recording() ;

    static Recording *new_recording(const std::string &uri,
                                    const rdf::Literal &format, const std::string &dataset) ;

    void add_signal(Signal *signal) ;
    Signal new_signal(const std::string &uri, const Unit &unit) ;
    Signal new_signal(const std::string &uri, const std::string &unit) ;

    void set_format(const rdf::Literal &format) ;
    void set_starttime(const Datetime &starttime) ;
    void set_duration(const Duration &duration) ;

    void to_rdf(const rdf::Graph &graph) ;
    } ;

  } ;

/*

  "subelement == True" means to recursively generate RDF for property value(s).

  "functional == False" means can have multiple values ==> keep as a list...

  SignalClass = Signal       #: The class of Signals in the Recording
  EventClass  = Event        #: The class of Events in the Recording

*/

#endif
