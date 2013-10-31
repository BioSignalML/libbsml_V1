#include <string>

#include "rdf/rdfnames.h"

using namespace rdf ;


std::string RDF::NS("http://www.w3.org/1999/02/22-rdf-syntax-ns#") ;
rdf::Uri RDF::uri(RDF::NS) ;
rdf::Resource RDF::type(RDF::NS + "type") ;

std::string RDFS::NS("http://www.w3.org/2000/01/rdf-schema#") ;
rdf::Uri RDFS::uri(RDFS::NS) ;
rdf::Resource RDFS::comment(RDFS::NS + "comment") ;
rdf::Resource RDFS::label(RDFS::NS + "label") ;

std::string DCT::NS("http://purl.org/dc/terms/") ;
rdf::Uri DCT::uri(DCT::NS) ;
rdf::Resource DCT::created(DCT::NS + "created") ;
rdf::Resource DCT::creator(DCT::NS + "creator") ;
rdf::Resource DCT::description(DCT::NS + "description") ;
rdf::Resource DCT::extent(DCT::NS + "extent") ;
rdf::Resource DCT::format(DCT::NS + "format") ;
rdf::Resource DCT::source(DCT::NS + "source") ;
rdf::Resource DCT::subject(DCT::NS + "subject") ;

std::string PRV::NS("http://purl.org/net/provenance/ns#") ;
rdf::Uri PRV::uri(PRV::NS) ;
rdf::Resource PRV::precededBy(PRV::NS + "precededBy") ;

std::string XSD::NS("http://www.w3.org/2001/XMLSchema#") ;
rdf::Uri XSD::uri(XSD::NS) ;
rdf::Resource XSD::XSD_integer(XSD::NS + "integer") ;
rdf::Resource XSD::XSD_double(XSD::NS + "double") ;
rdf::Resource XSD::XSD_dateTime(XSD::NS + "dateTime") ;
rdf::Resource XSD::XSD_dayTimeDuration(XSD::NS + "dayTimeDuration") ;

