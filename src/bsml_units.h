#ifndef _BSML_UNITS_H
#define _BSML_UNITS_H

typedef enum {

  UNITS_mbar = 1,
  UNITS_mV
  } BSML_UNITS ;

const char *bsml_units_as_string(BSML_UNITS u) ;

#endif
