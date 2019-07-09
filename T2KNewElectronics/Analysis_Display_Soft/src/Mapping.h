#ifndef Mapping_h
#define Mapping_h

#include "T2KConstants.h"

class Mapping
{
    public :
        // Constructors
        void loadMapping();

        // Getters
        int ichip(const int& card, const int& chip, const int& bin){return m_ichip[card][chip][bin];}
        int jchip(const int& card, const int& chip, const int& bin){return m_jchip[card][chip][bin];}
        int connector(const int& card, const int& chip, const int& ichip, const int& jchip){return m_connector[card][chip][ichip][jchip];}
        int i(const int& card, const int& chip, const int& bin);
        int j(const int& card, const int& chip, const int& bin);

        // Other

    private :
        int m_ichip[n::cards][n::chips][n::bins];
        int m_jchip[n::cards][n::chips][n::bins];
        int m_connector[n::cards][n::chips][geom::padOnchipx][geom::padOnchipy];
};

#endif
