#ifndef _BSML_RECORDING_H
#define _BSML_RECORDING_H

#include <assert.h>
#include <string>
#include <vector>
#include <map>

#include "model/bsml.h"
#include "model/object.h"
#include "model/signal.h"
#include "model/clock.h"
#include "model/datetime.h"
#include "model/units.h"

#include "rdf/rdf.h"
#include "rdf/rdfmap.h"


namespace bsml {

  class Recording : public AbstractObject
  /*===================================*/
  {
   private:
    std::map<std::string, AbstractObject *> resources_ ;

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

    virtual void close(void) { }

    static Recording *new_recording(const std::string &uri,
                                    const rdf::Literal &format, const std::string &dataset) ;

    bool add_resource(AbstractObject *resource) ;
    void add_signal(Signal *signal) ;

    template <class SIGNAL_T>
    SIGNAL_T *new_signal(const std::string &uri, const Unit &unit, double rate)
    /*---------------------------------------------------------------------*/
    {
      SIGNAL_T *signal = new SIGNAL_T(uri, unit, rate) ;
      this->add_signal(signal) ;
      return signal ;
      }

    template <class SIGNAL_T>
    SIGNAL_T *new_signal(const std::string &uri, const Unit &unit, Clock *clock)
    /*---------------------------------------------------------------------*/
    {
      SIGNAL_T *signal = new SIGNAL_T(uri, unit, clock) ;
      this->add_signal(signal) ;
      return signal ;
      }

    template <class SIGNAL_T>
    SignalGroup<SIGNAL_T> signalgroup(const std::vector<std::string> &uris,
    /*-----------------------------------------------------------------*/
                            const std::vector<Unit> &units, double rate)
    {
      assert(uris.size() == units.size()) ;
      SignalGroup<SIGNAL_T> signalgroup(uris.size()) ;
      for (size_t i = 0 ;  i < uris.size() ;  ++i) {
        SIGNAL_T *signal = this->new_signal<SIGNAL_T>(uris[i], units[i], rate) ;
        signalgroup.add_signal(signal) ;
        }
      return signalgroup ;
      }

    template <class SIGNAL_T>
    SignalGroup<SIGNAL_T> signalgroup(const std::vector<std::string> &uris,
    /*-----------------------------------------------------------------*/
                            const std::vector<Unit> &units, Clock *clock)
    {
      assert(uris.size() == units.size()) ;
      SignalGroup<SIGNAL_T> signalgroup(uris.size()) ;
      for (size_t i = 0 ;  i < uris.size() ;  ++i) {
        SIGNAL_T *signal = this->new_signal<SIGNAL_T>(uris[i], units[i], clock) ;
        signalgroup.add_signal(signal) ;
        }
      return signalgroup ;
      }

    Clock *new_clock(const std::string &uri, const Unit &units, double rate)
    /*--------------------------------------------------------------------*/
    {
      Clock *clock = new Clock(uri, units, rate) ;
      this->add_resource(clock) ;
      return clock ;
      }

    Clock *new_clock(const std::string &uri, const Unit &units, const std::vector<double> &times)
    /*----------------------------------------------------------------------------------*/
    {
      Clock *clock = new Clock(uri, units, times) ;
      this->add_resource(clock) ;
      return clock ;
      }

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
