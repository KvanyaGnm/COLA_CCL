#pragma once

#include <COLA.hh>
#include "factory.hh"
#include "filter.hh"

namespace cola {

  using CCLModule = GenericModule<
    CCLConverterFactory<MSTConverter>,
    CCLConverterFactory<SAConverter>,
    CCLConverterFactory<chainSAConverter>,
    CCLConverterFactory<CoalConverter>
  >;


}