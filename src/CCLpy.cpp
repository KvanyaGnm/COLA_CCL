#include "CCLpy.hh"
#include <COLA.hh>

extern "C" cola::VModule* LoadCOLAModule() { return new cola::CCLModule(); }