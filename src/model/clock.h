#ifndef _BSML_CLOCK_H
#define _BSML_CLOCK_H

#include <string>
#include <vector>

#include "config.h"
#include "model/object.h"
#include "model/units.h"
#include "rdf/rdf.h"


namespace bsml {

  class BSML_EXPORT Clock : public AbstractObject
  /*===========================================*/
  {
   protected:
    Unit unit_ ;
    std::vector<double> times_ ;
    double rate_ ;
    rdf::Literal literal_rate ;

   public:
    Clock(void) ;
    Clock(const std::string &uri, const Unit &unit, double rate) ;
    Clock(const std::string &uri, const Unit &unit, const std::vector<double> &times) ;

    virtual size_t size(void) ;
    virtual double time(size_t pos) ;
    virtual size_t index(double time) ;
    } ;

  } ;

#endif
