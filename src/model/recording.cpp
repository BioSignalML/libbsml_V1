#include <string>
#include <map>

#include "model/recording.h"
#include "model/signal.h"
#include "rdf/rdf.h"
#include "rdf/rdfnames.h"

#include "hdf5/bsml_h5.h"

using namespace bsml ;


Recording::Recording(void)
/*----------------------*/
{ }

Recording::Recording(const std::string &uri)
/*----------------------------------------*/
: AbstractObject(BSML::Recording, uri)
{
  }

Recording::~Recording(void)
/*-----------------------*/
{
  typename std::map<std::string, AbstractObject *>::iterator rp ;
  for (rp = resources_.begin() ;  rp != resources_.end() ;  ++rp) {
    delete (*rp).second ;
    }
  }


Recording *Recording::new_recording(const std::string &uri,
                                    const rdf::Literal &format, const std::string &dataset)
/*---------------------------------------------------------------------------------------*/
{
  Recording *recording = NULL ;
  if (format == Format::HDF5) {
    recording = H5Recording::H5create(uri, dataset, true) ;
    }
  if (recording) recording->set_format(format) ;
  return recording ;
  }


void Recording::set_format(const rdf::Literal &format)
/*--------------------------------------------------*/
{
  format_ = format ;
  rdfmap.push_back(new rdf::Mapping<rdf::Node>(rdf::DCT::format, &format_)) ;
  }


bool Recording::add_resource(AbstractObject *resource)
/*--------------------------------------------------*/
{
  const std::string uri = resource->get_uri_as_string() ;
  if (resources_.count(uri) == 0) {
    resources_.insert(std::pair<std::string, AbstractObject *>(uri, resource)) ;
    return true ;
    }
  return false ;
  }

void Recording::add_signal(Signal *signal)
/*--------------------------------------*/
{
  if (this->add_resource(signal)) signal->set_recording(this) ;
  // Error if signal already attached to a recording
  }

void Recording::to_rdf(const rdf::Graph &graph)
/*-------------------------------------------*/
{
  std::map<std::string, AbstractObject *>::iterator rp ;
  AbstractObject::to_rdf(graph) ;
  for (rp = resources_.begin() ;  rp != resources_.end() ;  ++rp) {
    rp->second->to_rdf(graph) ;
    }
  }


/*

  metaclass = BSML.Recording  #: :attr:`.BSML.Recording`

  attributes = [ 'dataset', 'source', 'format', 'comment', 'investigation',
                 'starttime', 'duration', 'timeline', 'generatedBy'
               ]
  '''Generic attributes of a Recording.'''

  mapping = { 'format':        PropertyMap(DCT.format),
              'dataset':       PropertyMap(BSML.dataset),
              'source':        PropertyMap(DCT.source, functional=False),
              'investigation': PropertyMap(DCT.subject),
              'investigator':  PropertyMap(DCT.creator),
              'starttime':     PropertyMap(DCT.created, XSD.dateTime,
                                           utils.datetime_to_isoformat,
                                           utils.isoformat_to_datetime),
              'duration':      PropertyMap(DCT.extent, XSD.dayTimeDuration,
                                           utils.seconds_to_isoduration,
                                           utils.isoduration_to_seconds),
##            'digest':        PropertyMap(BSML.digest),
              'timeline':      PropertyMap(TL.timeline,
                                           to_rdf=PropertyMap.get_uri,
                                           from_rdf=_get_timeline, subelement=True),
              'generatedBy':   PropertyMap(PROV.wasGeneratedBy, to_rdf=PropertyMap.get_uri,
                                           subelement=True),
            }

  SignalClass = Signal       #: The class of Signals in the Recording
  EventClass  = Event        #: The class of Events in the Recording

*/
