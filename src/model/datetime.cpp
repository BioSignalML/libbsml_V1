#include <string>

// C++11 has <chrono>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "model/datetime.h"
#include "rdf/rdfnames.h"

using namespace boost::posix_time ;

using namespace bsml ;


Datetime::Datetime(void)
/*--------------------*/
: rdf::Literal("", rdf::XSD::XSD_dateTime),
  datetime(microsec_clock::universal_time())
{
  }

std::string Datetime::as_iso_string(void)
/*-------------------------------------*/
{
  boost::gregorian::date ymd = datetime.date() ;
  time_duration hms = datetime.time_of_day() ;
  return boost::gregorian::to_iso_extended_string(ymd) + "T" + to_simple_string(hms) + "Z" ;
  }


Duration::Duration(void)
/*--------------------*/
: rdf::Literal("", rdf::XSD::XSD_dayTimeDuration),
  duration(0, 0, 0, 0)
{
  }
