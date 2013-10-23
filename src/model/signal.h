#ifndef _BSML_SIGNAL_H
#define _BSML_SIGNAL_H


#include <string>

#include "model/object.h"
#include "model/recording.h"
#include "model/datetime.h"
#include "model/clock.h"
#include "model/units.h"


namespace bsml {


  class Signal : public AbstractObject
  /*================================*/
  {
   private:
    Recording *recording_ ;
//    Clock *clock_ ;   subelement
    Unit unit_ ;
//    double rate_ ;
//    double period_ ;
//    Duration offset_ ;
//    Duration duration_ ;

   public:
    Signal(void) ;
    Signal(const std::string &uri, const Unit &unit) ;
    Signal(const std::string &uri, const std::string &unit) ;
    void set_recording(Recording *recording) ;
    } ;

  } ;

/*
  metaclass = BSML.Signal     #: :attr:`.BSML.Signal`

  attributes = ['recording', 'units', 'transducer', 'filter', '_rate',  '_period', 'clock',
                'minFrequency', 'maxFrequency', 'minValue', 'maxValue',
                'index', 'signaltype', 'offset', '_duration',
               ]
  '''Generic attributes of a Signal.'''

  mapping = { 'recording':    PropertyMap(BSML.recording, to_rdf=PropertyMap.get_uri),
              'units':        PropertyMap(BSML.units, to_rdf=PropertyMap.get_uri),
              'sensor':       PropertyMap(BSML.sensor),
              'filter':       PropertyMap(BSML.preFilter),
              '_rate':        PropertyMap(BSML.rate, XSD.double),
              '_period':      PropertyMap(BSML.period, XSD.double),

              'clock':        PropertyMap(BSML.clock, to_rdf=PropertyMap.get_uri, subelement=True),

              'minFrequency': PropertyMap(BSML.minFrequency, XSD.double),
              'maxFrequency': PropertyMap(BSML.maxFrequency, XSD.double),
              'minValue':     PropertyMap(BSML.minValue, XSD.double),
              'maxValue':     PropertyMap(BSML.maxValue, XSD.double),
              'dataBits':     PropertyMap(BSML.dataBits, XSD.integer),
              'signaltype':   PropertyMap(BSML.signalType),


              'offset':       PropertyMap(BSML.offset, XSD.dayTimeDuration,
                                          utils.seconds_to_isoduration,
                                          utils.isoduration_to_seconds),
              '_duration':    PropertyMap(DCT.extent, XSD.dayTimeDuration,
                                          utils.seconds_to_isoduration,
                                          utils.isoduration_to_seconds),
            }

*/

#endif