#include "T2KConstants.h"
#include "DAQ.h"

#include <fstream>
#include <string>
#include <iostream>

using namespace std;

void DAQ::loadDAQ()
{
    int det, arc;

    ifstream det2arc((loc::daq + "detector2arc.txt").c_str());
    while (!det2arc.eof())
    {
        det2arc >> det >> arc >> ws;
        detector2arc[det]=arc;
        arc2detector[arc]=det;
    }
    det2arc.close();

    int daq;
    ifstream farc2daq((loc::daq + "arc2daq.txt").c_str());
    while (!farc2daq.eof())
    {
        farc2daq >> arc >> daq >> ws;
        arc2daq[arc]=daq;
        daq2arc[daq]=arc;
    }
    farc2daq.close();

}
