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
   private:
    Unit unit_ ;
    const double rate_ ;
    const double period_ ;
    std::vector<double> times_ ;

   public:
    Clock(void) ;
    Clock(const std::string &uri, const Unit &unit, const double &period) ;
    Clock(const std::string &uri, const Unit &unit, double *times, size_t size) ;

    size_t size(void) ;
    double time(size_t pos) ;
    size_t index(double time) ;
    } ;

  } ;

#endif
