#include <string>
#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include <ctype.h>

#include "utility.h"

#include "units.h"
//
//
using namespace bsml ;

Unit::Unit(void)
/*------------*/
: rdf::Resource()
{
  }


Unit::Unit(const std::string &unit)
/*-------------------------------*/
: rdf::Resource(unit)
{
  }


static dict direct_ = {
  { "Bpm",    "BeatsPerMinute" },
  { "bpm",    "BeatsPerMinute" },
  { "cc",     "CubicCentimetre" },
  { "pm",     "PerMinute" },
  { "1/min",  "PerMinute" },
  { "Lpm",    "LitrePerMinute" },
  { "lpm",    "LitrePerMinute" },
  { "mv",     "Millivolt" },
  { "uV-mrs", "Microvolt" },
  { "annotation", "AnnotationData" } } ;

static dict units_ = {
  { "%",     "Percent" },
  { "A",     "ampere" },
  { "deg",   "DegreeOfArc" },
  { "degC",  "DegreeCelsius" },
  { "mHg",   "metresOfMercury" },
  { "mH2O",  "metresOfWater" },
  { "g",     "gram" },
  { "J",     "joule" },
  { "K",     "Kelvin" },
  { "l",     "litre" },
  { "L",     "litre" },
  { "m",     "metre" },
  { "min",   "minute" },
  { "s",     "second" },
  { "V",     "volt" },
  { "W",     "watt" },
  { "bar",   "bar" },
  { "BPM",   "BeatsPerMinute" },
  { "bpm",   "BeatsPerMinute" },
  { "M",     "molar" },
  { "mol",   "mole" } } ;

static dict powers_prefix_ = {
  { "2", "Square" },
  { "3", "Cubic" } } ;

static dict powers_suffix_ = {
  { "2", "Squared" },
  { "3", "Cubed" } } ;

static dict prefixes_ = {
  { "Y", "Yotta" },  { "Z", "Zetta" },  { "E", "Exa" },
  { "P", "Peta" },   { "T", "Tera" },   { "G", "Giga" },
  { "M", "Mega" },   { "K", "Kilo" },   { "H", "Hecto" },
  { "D", "Deca" },
  { "d", "Deci" },   { "c", "Centi" },  { "m", "Milli" },
  { "u", "Micro" },  { "n", "Nano" },   { "p", "Pico" },
  { "f", "Femto" },  { "a", "Atto" },   { "z", "Zepto" },
  { "y", "Yocto" } } ;
//  { "\u00b5", "Micro" } } ;  // C++ and Unicode don't play...



static std::string titlecase(const std::string &s)
/*----------------------------------------*/
{
  return std::string(1, toupper(s[0])) + s.substr(1) ;
  }

static std::string name(const std::string &u)
/*----------------------------------*/
{
  try {
    return titlecase(units_.at(u)) ;
    }
  catch (const std::out_of_range &e) {
    return prefixes_.at(u.substr(0, 1)) + units_.at(u.substr(1)) ;
    }
  }

static std::string power(const std::string &u)
/*-----------------------------------*/
{
  size_t p = u.find("^") ;
  if (p == std::string::npos) return name(u) ;
  else {
    std::string nm = name(u.substr(0, p)) ;
    size_t l = nm.length() ;
    std::string suffix = nm.substr(l - 6, std::string::npos) ;
    boost::to_lower(suffix) ;
    if (suffix == "second")
      return nm + powers_suffix_.at(u.substr(p+1)) ;
    else
      return powers_prefix_.at(u.substr(p+1)) + nm ;
    }
  }

static std::string mult(const std::string &u)
/*----------------------------------*/
{
  return strlist(strlist(u, "*"), power).join("") ;
  }


Unit bsml::get_unit(const std::string &u)
/*=====================================*/
{
  /**
   * Convert an abbreviated unit-of-measure into a URI from
   * a unit"s ontology.
   */
  std::string unit = "" ;
  if (u != "") {
    try {
      unit = direct_.at(u) ;
      }
    catch (const std::out_of_range &e) {
      try {
        unit = strlist(strlist(u, "/"), mult).join("Per") ;
        }
      catch (const std::out_of_range &e) {
        if (u.substr(0, 4) == "per_")
          unit = "Per" + titlecase(u.substr(4)) ;
        else
          unit = titlecase(u) ;
        }
      }
    if (bsml::unit::list.contains(unit)) return Unit(bsml::unit::NS + unit) ;
    }
//   raise ValueError("Unknown units abbreviation, %s" % s)
  return Unit() ;
  }



#ifdef TEST_UNITS


#include <iostream>
#include <exception>


static void test(const std::string &u)
/*----------------------------------*/
{
  try {
    std::string o = get_units(u) ;
    if (o != "") std::cout << u << " --> " << o << std::endl ;
    else         std::cout << "Cannot convert: '" << u << "'" << std::endl ;
    }
  catch (const std::exception &e) {
    std::cout << "Error converting: " << u << "(" << e.what() << ")" << std::endl ;
    }
  }


int main(void)
/*=========*/
{
  test("Km") ;
  test("Kg*m/s^2/W*m") ;
  test("mm^2") ;
  test("Mm^3") ;
  test("ms") ;
  test("Mm/s") ;
  test("mg") ;
  test("deg") ;
  test("%") ;
  test("uV") ;
  test("mV") ;
  test("bar") ;
  test("mbar") ;
  test("degC") ;
  test("mmHg") ;
  test("cmH2O") ;
  test("bpm") ;
  test("clpm") ;
  test("") ;
//  test("\u00b5V") ;
  }

#endif
