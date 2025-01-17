#ifndef Pixel_h
#define Pixel_h

#include "T2KConstants.h"
#include "Mapping.h"
#include "DAQ.h"
#include <iostream>
#include "TPolyLine.h"
#include "RQ_OBJECT.h"
#include "TQObject.h"

using namespace std;

class Pixel
{
    //RQ_OBJECT("Pixel")

    public :
        // Constructors
        Pixel();
        Pixel(DAQ& daq, Mapping& map, int i, int j, int amp);

        // Setters different
        void setPixel(int card, int chip, int channel, int ichip, int jchip);
        void setAmp(int amp);

        // Getters
        int coordi();
        int coordj();
        int card();
        int chip();
        int channel();
        int connector();
        int ampl();
        float coordx();
        float coordy();
        int id();
        TPolyLine* line();

        // Display cell address content.
	    friend ostream& operator<<(ostream& os, const Pixel& P)
	    {
            os << "Pixel" << endl;
            os << "{" << endl;
            printf("\tConnections : #Card : %i   #Chip : %i   #Channel : %i\n", P.m_card, P.m_chip, P.m_channel);
            os << "\tCoordinates : " << endl;
            printf("\t\tOn chip : (%i, %i)\n", P.m_ichip, P.m_jchip);
            printf("\t\tOn board (%i, %i)\n", P.m_i, P.m_j);
            printf("\t\tOn board in mm (%.1f, %.1f)\n", P.m_x*1000, P.m_y*1000);
            os << "}" << endl;
		    return os;
        }

    private :
        int m_card;
        int m_chip;
        int m_channel;
        int m_connector;
        int m_i;
        int m_j;
        int m_ichip;
        int m_jchip;
        float m_x;
        float m_y;
        int m_amp;
        int m_id;
        TPolyLine* m_line;

};

#endif
