#ifndef _BSML_CLOCK_H
#define _BSML_CLOCK_H

#include <string>
#include <vector>

#include "model/object.h"
#include "model/units.h"


namespace bsml {

  class Clock : public AbstractObject
  /*================================*/
  {
   protected:
    Unit unit_ ;
    std::vector<double> times_ ;
    double rate_ ;

   public:
    Clock(void) ;
    Clock(const std::string &uri, const Unit &unit) ;

    virtual size_t size(void) ;
    virtual double time(size_t pos) ;
    virtual size_t index(double time) ;
    } ;

  } ;

#endif
