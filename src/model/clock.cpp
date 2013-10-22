#include <string>
#include <vector>
#include <cmath>

#include "model/clock.h"
#include "model/bsml.h"


using namespace bsml ;


Clock::Clock(void)
/*--------------*/
: rate_(0.0), period_(0.0)
{ }

Clock::Clock(const std::string &uri, const Unit &unit, double *times, size_t size)
/*---------------------------------------------------------------------------------*/
: AbstractObject(BSML::SampleClock, uri), unit_(unit), rate_(0.0), period_(0.0), times_(times, times+size)
{
  }

size_t Clock::size(void)
/*--------------------*/
{
  return times_.size() ;
  }

double Clock::time(size_t pos)
/*--------------------------*/
{
  if (period_ != 0.0) return (double)pos*period_ ;
  else if (rate_ != 0.0) return (double)pos/rate_ ;
  else return times_.at(pos) ;
  }


/**
  Find the index of a time in the clock.

  :param float time: The time to lookup, in seconds.
  :return: The greatest index such that ``self.time(index) <= t``.
    -1 is returned if ``t`` is before ``self.time(0)``.
  :rtype: int
**/
size_t Clock::index(double time)
/*----------------------------*/
{
  if (period_ != 0.0) return (size_t)floor(time/period_) ;
  else if (rate_ != 0.0) return (size_t)floor(time*rate_) ;
  else {
    size_t i = 0 ;
    size_t j = times_.size() ;
    while (i < j) {
      size_t m = (i + j)/2 ;
      if (times_.at(m) <= time) i = m + 1 ;
      else                      j = m ;
      }
    return i - 1 ;
    }
  }

/*
  def extend(self, times):
  #-----------------------
    """
    Add times to a clock.

    :param np.array times: Array of sample times, in seconds.
    """
    if self.times[-1] >= times[0]:
      raise DataError('Times must be increasing')
    self.times = np.append(self.times, times)

*/
