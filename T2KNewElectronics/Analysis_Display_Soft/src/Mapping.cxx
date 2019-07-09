#include "T2KConstants.h"
#include "Mapping.h"

#include <fstream>
#include <string>
#include <iostream>

using namespace std;

void Mapping::loadMapping()
{
    // Load in the correspondance betwwen pad coordinates and pin connectors
            /*
                    --> Cards
                ---  ---  ---  ---
    C          | A || C || A || C |
    h          | A || C || A || C |
    i           ---  ---  ---  ---
    p          | B || D || B || D |
    s          | B || D || B || D |
                ---  ---  ---  ---



        */

    int i, j, connector;

    std::cout << "Reading ChipA.txt" << std::endl;

    ifstream A((loc::mapping + "ChipA.txt").c_str());
    while (!A.eof())
    {
        A >> j >> i >> connector >> ws;
        //int index = channel - 3;
        //if (channel >=43) index -= 6;
        int index = connector;
        m_ichip[0][0][index] = i ;
        m_jchip[0][0][index] = j ;
        m_ichip[0][1][index] = i ;
        m_jchip[0][1][index] = j ;
        m_ichip[2][0][index] = i ;
        m_jchip[2][0][index] = j ;
        m_ichip[2][1][index] = i ;
        m_jchip[2][1][index] = j ;
        // Coord to channel
        m_connector[0][0][i][j] = connector ;
        m_connector[0][1][i][j] = connector ;
        m_connector[2][0][i][j] = connector ;
        m_connector[2][1][i][j] = connector ;


    }
    A.close();

    std::cout << "Reading ChipB.txt" << std::endl;
    ifstream B((loc::mapping + "ChipB.txt").c_str());
    while(!B.eof())
    {
        B >> j >> i >> connector >> ws;
        //int index = channel - 3;
        //if (channel >=43) index -= 6;
        int index = connector;
        m_ichip[0][2][index] = i ;
        m_jchip[0][2][index] = j ;
        m_ichip[0][3][index] = i ;
        m_jchip[0][3][index] = j ;
        m_ichip[2][2][index] = i ;
        m_jchip[2][2][index] = j ;
        m_ichip[2][3][index] = i ;
        m_jchip[2][3][index] = j ;
        // Coord to channel
        m_connector[0][2][i][j] = connector ;
        m_connector[0][3][i][j] = connector ;
        m_connector[2][2][i][j] = connector ;
        m_connector[2][3][i][j] = connector ;
    }
    B.close();

    std::cout << "Reading ChipC.txt" << std::endl;
    ifstream C((loc::mapping + "ChipC.txt").c_str());
    while(!C.eof())
    {
        C >> j >> i >> connector >> ws;;
        //int index = channel - 3;
        //if (channel >=43) index -= 6;
        int index = connector;
        m_ichip[1][3][index] = i ;
        m_jchip[1][3][index] = j ;
        m_ichip[1][2][index] = i ;
        m_jchip[1][2][index] = j ;
        m_ichip[3][3][index] = i ;
        m_jchip[3][3][index] = j ;
        m_ichip[3][2][index] = i ;
        m_jchip[3][2][index] = j ;
        // Coord to channel
        m_connector[1][3][i][j] = connector ;
        m_connector[1][2][i][j] = connector ;
        m_connector[3][3][i][j] = connector ;
        m_connector[3][2][i][j] = connector ;
    }
    C.close();

    std::cout << "Reading ChipD.txt" << std::endl;
    ifstream D((loc::mapping + "ChipD.txt").c_str());
    while(!D.eof())
    {
        D >> j >> i >> connector >> ws; 
        //int index = channel - 3;
        //if (channel >=43) index -= 6;
        int index = connector;
        m_ichip[1][1][index] = i ;
        m_jchip[1][1][index] = j ;
        m_ichip[1][0][index] = i ;
        m_jchip[1][0][index] = j ;
        m_ichip[3][1][index] = i ;
        m_jchip[3][1][index] = j ;
        m_ichip[3][0][index] = i ;
        m_jchip[3][0][index] = j ;
        // Coord to channel
        m_connector[1][1][i][j] = connector ;
        m_connector[1][0][i][j] = connector ;
        m_connector[3][1][i][j] = connector ;
        m_connector[3][0][i][j] = connector ;

    }
    D.close();


}


int Mapping::i(const int& card, const int& chip, const int& bin)
{
    return (m_ichip[card][chip][bin] + card*geom::padOnchipx);
}

int Mapping::j(const int& card, const int& chip, const int& bin)
{
    int result;
    if (card%2==0){result=m_jchip[card][chip][bin] + (n::chips-1-chip)*geom::padOnchipy;}
    else if (card%2==1){result=m_jchip[card][chip][bin] + chip*geom::padOnchipy;}
    return result;
}
