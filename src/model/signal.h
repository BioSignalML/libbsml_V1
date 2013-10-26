#ifndef _BSML_SIGNAL_H
#define _BSML_SIGNAL_H

#include <string>
#include <vector>
#include <map>

#include "model/object.h"
#include "model/datetime.h"
#include "model/clock.h"
#include "model/units.h"
#include "rdf/rdf.h"


namespace bsml {

  class Recording ;

  class Signal : public AbstractObject
  /*================================*/
  {
   private:
    Recording *recording_ ;
    Clock *clock_ ;
    Unit unit_ ;
    double rate_ ;
    rdf::Literal literal_rate ;
//    Duration offset_ ;
//    Duration duration_ ;

   public:
    Signal(void) ;
    Signal(const std::string &uri, const Unit &unit, double rate) ;
    Signal(const std::string &uri, const Unit &unit, Clock *clock) ;
    void set_recording(Recording *recording) ;
    } ;


  template <class SIGNAL>
  class SignalGroup : public std::vector<SIGNAL *>
  /*============================================*/
  {
   private:
    std::map<std::string, SIGNAL *> urimap ;

   public:
    SignalGroup(size_t size)
    /*--------------------*/
    : std::vector<SIGNAL *>(size) { }

    void add_signal(SIGNAL *signal)
    /*---------------------------*/
    {
      urimap.insert(std::pair<std::string, SIGNAL *>(signal->get_uri_as_string(), signal)) ;
      this->push_back(signal) ;
      }

    SIGNAL *get_signal(const std::string uri)
    /*-------------------------------------*/
    {
      return urimap.at(uri) ;
      }

    SIGNAL *get_signal(size_t index)
    /*----------------------------*/
    {
      return this->at(index) ;
      }

    virtual void extend(std::vector<double> data __attribute__((__unused__)))
    /*---------------------------------------------------------------------*/
    {
      }


    } ;


  } ;

#endif
