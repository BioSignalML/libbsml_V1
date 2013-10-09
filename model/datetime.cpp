#include "datetime.h"
#include "rdfnames.h"


using namespace bsml ;


Datetime::Datetime(void)
/*--------------------*/
: rdf::Literal("", rdf::XSD::dateTime)
{
  }


Duration::Duration(void)
/*--------------------*/
: rdf::Literal("", rdf::XSD::dayTimeDuration)
{
  }
