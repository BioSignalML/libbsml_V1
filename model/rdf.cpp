#include <string>
#include <redland.h>

#include "rdf.h"

rdf::Node::Node(const std::string &value, const rdf::Resource &datatype)
/*--------------------------------------------------------------------*/
: node(librdf_new_node_from_typed_literal(
         world,
         (const unsigned char *)value.c_str(),
         NULL,
         datatype.get_librdf_uri())) { }

