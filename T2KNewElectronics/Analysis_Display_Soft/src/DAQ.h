#ifndef DAQ_h
#define DAQ_h

#include "T2KConstants.h"

class DAQ
{
    public :
        // Constructors
        void loadDAQ();

        // Getters
        int DAQchannel(int detector)
        {
          int res;
          res = arc2daq[detector2arc[detector]];
          if (res==-99 && detector==65){res=49;}
          return(res);
        }
        int connector(const int& daqchannel){return(arc2detector[daq2arc[daqchannel]]);}

        // Other

    private :
        int detector2arc[n::bins];
        int arc2daq[n::bins];
        int daq2arc[n::bins];
        int arc2detector[n::bins];
};

#endif
