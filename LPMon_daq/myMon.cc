//
// Display hit pattern in real geometry with JTPC's display feature.
//
//  - Waveform window for the pad on which cursor is located.
//     <-- Update only when cursor enters new pad area
//  
//  - Open another window to keep waveform of the current pad.
//
//  - Show track with color for max ADC value
//     <-- How: scan all the row,col and get max adc value from attached hist
//
//  - Use real geomerical shape: key stone
//
// NEED:
//  - Map from (row,col) to hist id (which is a channel/pad id)
//
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <cstdio>
#include <unistd.h>     // sleep()
//#include <arpa/inet.h>  // ntohs()
#include "byteswap.h"


#include "TH1F.h"
#include "TPad.h"
#include "TCanvas.h"
//#include "TApplication.h"
#include "TGFrame.h"
#include "TPolyLine.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TROOT.h"

using namespace std;

//-----------------------------------------------------------------------------
// Structures :
//-------------
struct EventHeader {
  int eventSize;
  int eventNumb;
};

struct DataPacketHeader {
  unsigned short size;
  unsigned short hdr;
  unsigned short args;
  unsigned short ts_h;
  unsigned short ts_l;
  unsigned short ecnt;
  unsigned short scnt;
};

struct DataPacketData {
  unsigned short data[550];
};

struct DataPacketEnd {
  unsigned short crc1;
  unsigned short crc2;
};

//-----------------------------------------------------------------------------
// Macros :
//---------
// Macros to interpret DataPacket header
#define GET_FEC_ERROR(word)  (((word) & 0x03F0)>>4)
#define GET_LOS_FLAG(word)   (((word) & 0x0400)>>10)
#define GET_SYNCH_FAIL(word) (((word) & 0x0800)>>11)

// Macros to interpret DataPacket read back arguments
#define GET_RB_MODE(word)     (((word) & 0x4000)>>14)
#define GET_RB_COMPRESS(word) (((word) & 0x2000)>>13)
#define GET_RB_ARG2(word)     (((word) & 0x1E00)>>9)
#define GET_RB_ARG1(word)     (((word) & 0x01FF))

// Macros to interpret DataPacket event type / count
#define GET_EVENT_TYPE(word)  (((word) & 0xC000)>>14)
#define GET_EVENT_COUNT(word) (((word) & 0x3FFF))

// Macros to interpret DataPacket Samples
#define CELL_INDEX_FLAG      0x1000
//#define GET_CELL_INDEX(word) (((word) & 0x0FFF))
#define GET_CELL_INDEX(word) (((word) & 0x01FF))

#define ntohs(x)  __bswap_16 (x)
#define ntohl(x)  __bswap_32 (x)


//-----------------------------------------------------------------------------
// Class for KeyStone Pad :
//-------------------------
class KeyStonePad
{
public:
  KeyStonePad(int id=0, double *x=0, double *y=0);
  ~KeyStonePad() { delete fPolyLine; }
  
  int GetID()     const { return fId; }
  
  void SetID(int id) { fId = id; }
  
  void SetFillColor(int colorId);
  
  //void Modified() { fPolyLine->Modified(); }
  void Modified() {} 
  void Draw();
private:
  int fId;      // it's own id 
  TPolyLine *fPolyLine;
};


KeyStonePad::KeyStonePad(int id, double *x, double *y)
{
  fId = id;

  const int kNPoints = 4;
  double xCorner[kNPoints+1];  // +1 to close
  double yCorner[kNPoints+1];
  if ( x!=0 && y!=0 ) {
    for (int i=0; i<kNPoints; i++) {
      xCorner[i] = x[i];
      yCorner[i] = y[i];
    }
    xCorner[kNPoints] = x[0];
    yCorner[kNPoints] = y[0];
    
    fPolyLine = new TPolyLine(kNPoints+1, xCorner, yCorner);
    fPolyLine->SetLineColor(1);
    fPolyLine->SetLineWidth(2);
    fPolyLine->Draw("f");
  }
  else {
    fPolyLine = 0;
  }
}


inline void KeyStonePad::SetFillColor( int color )
{
  fPolyLine->SetFillColor( color );
}


inline void KeyStonePad::Draw()
{
  if ( fPolyLine != 0 ) {
    fPolyLine->Draw();
  }
}


//-----------------------------------------------------------------------------
// Global variabes :
//------------------
const int kNumRows     = 24;
const int kNumCols     = 72;
const int kNumPads     = kNumRows * kNumCols;
const int kNumTimeBins = 512;

int channelMap[kNumPads];          // to get channel # from seq. pad id

vector<TH1F*> hADC(kNumPads);    // initialize them to null
//vector<TPad*> tpcPad(kNumPads);
vector<KeyStonePad*> tpcPad(kNumPads);

double tpcPanelWidth  = 0.0;
double tpcPanelHeight = 0.0;

FILE *acqFile = 0;
//ifstream acqFile;

const int kNPoints = 4;
double x[kNumPads][kNPoints];
double y[kNumPads][kNPoints];
//

//-----------------------------------------------------------------------------
// INIT :
//-------
//
void ReadMap() 
{
  //
  // Read Two Maps : 
  //
  //  1. For Channel/Histo :  fill channelMap[]
  //  2. For Coordinates of the corners
  //
  ifstream map4DispFile("Map4Disp.txt");
  for (int i=0; i<kNumPads; i++) {
    int irow, icol, padId, chanId;
    map4DispFile >> irow >> icol >> padId >> chanId;
    channelMap[padId] = chanId;
  }
  map4DispFile.close();


  ifstream padGeomFile("keystonePadGeom.txt");
  //ifstream padGeomFile("rectCorners.txt");
  
  char   varName[20];
  double width;
  double ymin, ymax;
  
  padGeomFile >> varName >> width;  // half width of the pad panel
  padGeomFile >> varName >> ymin;
  padGeomFile >> varName >> ymax;

  tpcPanelWidth  = 2. * width;         // total width  of the pad array
  tpcPanelHeight = ymax - ymin;        // total height of the pad array

  cout << width << ", " << ymin << ", " << ymax << endl;

  for (int i=0; i<kNumPads; i++) {
    int id, row, col;
    double x0, y0, x1, y1, x2, y2, x3, y3;
    //padGeomFile >> id >> row >> col 
    padGeomFile >> row >> col 
                >> x0 >> y0 >> x1 >> y1 >> x2 >> y2 >> x3 >> y3;
    id = i;
    x[id][0] = x0;    y[id][0] = y0;
    x[id][1] = x1;    y[id][1] = y1;
    x[id][2] = x2;    y[id][2] = y2;
    x[id][3] = x3;    y[id][3] = y3;
  }

  padGeomFile.close();

}


//-------------------------------------------
void Init()
{
  //
  // Create histograms for ADC  : hID = channel ID
  //
  for (int i=0; i<kNumPads; i++) {
    char hname[20];
    sprintf(hname, "h%04d", i);
    hADC[i] = new TH1F(hname, hname, kNumTimeBins, -0.5, kNumTimeBins-0.5);
  } 
  
  //
  // Create Key Stone Pad objects
  //
  //vector<KeyStone*> tpcPad(kNumPads);

  //
  // Assign its id to corresponding histogram ID
  //
  // Algo : (row,col) --> keyStone ID = col + row*NCOLS
  //                  ==> chanID[keyID] = Map(row,col) 
  //
  const double scale  = 0.95;          // scale down further for padding 
  const double xscale = scale * 1./tpcPanelWidth;
  const double yscale = scale * 1./tpcPanelHeight;

  const double yoffset = 0.02;

  for (int ipad=0; ipad<kNumPads; ipad++) {

    // Transform pad geom to (0,0) ~ (1,1)
    double xCorner[kNPoints];
    double yCorner[kNPoints];
    
    for (int ic=0; ic<kNPoints; ic++) {
      xCorner[ic] = x[ipad][ic] * xscale + 0.5;
      yCorner[ic] = y[ipad][ic] * yscale + 0.5; // + yoffset;
    }
    
    char pName[20];
    sprintf( pName, "p%4d", ipad);
    //tpcPad[ipad] = new TPad(pName, pName, x0,y0, x1,y1);
    tpcPad[ipad] = new KeyStonePad(ipad, xCorner, yCorner);
    tpcPad[ipad]->Draw();
  }
}

//-----------------------------------------------------------------------------
// Draw Pad :
//-----------
//
//               gray, b,  lb,  lg, g,  ly, y,  lr, r
//int colorCode[] = {19, 4, 154, 153, 3, 155, 5, 152, 2}; 
int   colorCode[] = {19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 };
int   colorLength = 11;
const double threshold = 40;
//const double threshold = 15;

void FillPadWithColor( )
{
  // Calculate color index
  double MAXADC = -1000.;
  for (int hid=0; hid<kNumPads; hid++)
    if ( hADC[hid]->GetMaximum() > MAXADC )
      MAXADC = hADC[hid]->GetMaximum();
  MAXADC -= 40;
  //cout << "MAXADC = " << MAXADC << endl;

  if ( MAXADC <= 0 ) 
    return;

  int    nSysColors = gStyle->GetNumberOfColors();
  int    nColors    = 10;
  double scale      = nColors / MAXADC;
  
  for (int ipad=0; ipad<kNumPads; ipad++) {
    // Get max adc of the pads
    int    hId    = channelMap[ ipad ];
    double adcMax = hADC[hId]->GetMaximum() - threshold;

    /*
    int color = 19;
    if ( adcMax >0 ) {
      color = (int)(0.01 + adcMax*scale);
      color = (int)( (color+0.99) * nSysColors / nColors);
    }
    */

    int colorIndex = 0;
    
    /*
    if ( MAXADC > threshold ) {
    */
    if ( adcMax == MAXADC ) {
      colorIndex = 10;
    }
    else if ( adcMax >= threshold ) {
      colorIndex = (int)((adcMax/MAXADC) * 10.);
      //cout << "cId=" << colorIndex << " : " 
      //     << MAXADC << ", " << adcMax << endl;    }
    }
    tpcPad[ipad]->SetFillColor( colorCode[colorIndex] );
    //tpcPad[ipad]->SetFillColor( color );
    tpcPad[ipad]->Modified();  // Tell Canvas that I've some changed attributes
  }
}


//
//-----------------------------------------------------------------------------
// EVENT :
//--------
//
//  Reset All Histograms
//  Read ACQ file for an event
//  Decode it and fill the histograms
//  Itreate over KeyStoneID and get max adc value of the attached hist
//  Fill them with color according to the max adc value
//
//----------------------------------------------------------------------
int DecodeNextEvent(void) 
{
  EventHeader      head;
  DataPacketHeader pHeader;
  DataPacketData   pData;
  DataPacketEnd    pEnd;

  // Read next header or quit of end of file
  if(fread(&head, sizeof(EventHeader), 1, acqFile) != 1) {
  //acqFile.read((char*)(&head), sizeof(EventHeader));
  //if ( acqFile.gcount() != sizeof(EventHeader) ) {
    //fclose(acqFile);
    return 1; // End of program
  }
	
  //cout << "EventSize = 0x" << hex << head.eventSize << dec; // << endl;
  head.eventSize = ntohl(head.eventSize);
  head.eventNumb = ntohl(head.eventNumb);
  //head.eventTime = ntohl(head.eventTime);
  //cout << " --> 0x" << hex << head.eventSize << dec << endl;
	
  int rawDataSize = head.eventSize - sizeof(head);
  // 	cout << "Event number from header --> " << head.eventNumb << endl;
  // 	cout << " total rawdata size " << head.eventSize << endl ;
  // 	cout << "Rawdata size " << rawDataSize << endl ;
  // 	iFirstPacket=0;
  int nByteTot = rawDataSize ;
  int nByteRead=0;

  while (nByteRead < nByteTot ) {

    // now read data packet header
    fread(&pHeader, sizeof(DataPacketHeader), 1, acqFile);
    nByteRead += sizeof(DataPacketHeader);

    // check sync fail
    if (GET_LOS_FLAG(ntohs(pHeader.hdr) )) cout << " LOS FLAG SET " << endl  ;
    if (GET_SYNCH_FAIL(ntohs(pHeader.hdr))) cout << " SYNC FAIL FLAG SET " << endl  ;

    // compute asic#
    int tempAsic1 = GET_RB_ARG1(ntohs(pHeader.args));
    int tempAsic2 = GET_RB_ARG2(ntohs(pHeader.args));
    int channel = tempAsic1/6;
    int asicN = 10*(tempAsic1%6)/2 + tempAsic2;
    // 		cout << " channel " << channel << " asic " << asicN << endl;

    //get sample count
    int sampleCount = ntohs(pHeader.scnt);

    // take into account padding short int to get even total
    int sampleCountRead = sampleCount;
    if (sampleCount%2==1)  sampleCountRead++;

    // now read the data 
    fread(&pData, sizeof(unsigned short int), sampleCountRead, acqFile);
    nByteRead+=sampleCountRead*sizeof(unsigned short int);
    
    // read end (crc stuff)
    fread(&pEnd,sizeof(DataPacketEnd), 1, acqFile);
    nByteRead+=sizeof(DataPacketEnd);

    // compute physical channel (0-71)
    int physChannel=-10; 
    if (channel> 2 && channel < 15 ) {
      physChannel= channel -3; 
    } else if (channel> 15 && channel < 28 ) {
      physChannel= channel -4; 
    } else if (channel> 28 && channel < 53 ) {
      physChannel= channel -5; 
    } else if (channel> 53 && channel < 66 ) {
      physChannel= channel -6; 
    } else if (channel> 66  ) {
      physChannel= channel -7; 
    }
        
    int thisPhysChannel = asicN*72+physChannel;
    //int tempCharge = 0; 
    //     		int tempNHit = 0;
    int thisCell = -999;
    //int maxcharge = -50;
    // 			int maxTime = 510; 
 
    int timeBin = 0;

    //only physical channels
    if (physChannel>-1 && physChannel<72 && sampleCount>9) { 
      for (int ip=0; ip<sampleCount-2; ip++) {
        unsigned short int datum = ntohs(pData.data[ip]);
        if (datum & CELL_INDEX_FLAG) {                           // <-- (3)
          //cout << "new packet found cell " << (datum&0xFFF) << endl;//4095
          thisCell = GET_CELL_INDEX(datum);
          if ( thisCell != 0x1FF )   // is this last bin ?
            timeBin  = TMath::Max(thisCell-10, 0);        // reset bin #
          else
            thisCell  = -999;        // last bin is not for ADC but for chip 
        } 
        else if ( thisCell != -999 ) {
          int datumC = datum - 250; 
          hADC[thisPhysChannel]->Fill( timeBin++, datumC );
        }
      } //end of for ip 
    } //end of if physchanel
  } //end of while nByteRead

  return 0;
}


//-----------------------------------------------------------------------------
// MAIN :
//-------
//

//int main(int argc, char **argv)
int monitor( string acqFileName="RUN_00146.00000.acq")
{
  //
  // Open data file
  // 
  //ifstream acqFile("RUN_00130.00000.acq",ios::binary);
  //acqFile = fopen("RUN_00130.00000.acq", "rb");
  acqFile = fopen(acqFileName.c_str(), "rb");

  // Exit as soon as possible before doing any serious works
  if ( ! acqFile ) {
    cout << "Can't open ACQ file: " << acqFileName << endl;
    return -1;
  }

  char runUID[21];
  //acqFile.read( runUID, 20 );
  int nRead = fread( runUID, sizeof(char), 20, acqFile );
  runUID[20] = '\0';
  //int status = DecodeNextEvent( acqFile );
  //cout << "Run UID : " << runUID  << ": nRead=" << acqFile.gcount() << endl;
  cout << "Run UID : " << runUID  << ": nRead=" << nRead << endl;

  //
  //
  //
  //TApplication theApp( "Monitoring", &argc, argv );

  ReadMap();

  //
  // Create canvas
  //
  // NOTE: This SHOULD be done before create TPad's
  //
  double aspectRatio = tpcPanelHeight / tpcPanelWidth; // to get H from W
  int canvasWidth    = 1024;
  int canvasHeight   = static_cast<int>(aspectRatio * canvasWidth);

  cout << tpcPanelHeight << ", " << tpcPanelWidth <<  ", " << aspectRatio
       << ", " << canvasWidth << ", " << canvasHeight << endl;

  TCanvas *c = new TCanvas("c1", "LP-TPC Display", canvasWidth, canvasHeight);

  //
  // Initialize histograms, tpc pad's, etc.
  //
  Init();

  // Add tpcPads to the canvas
  for (int ipad=0; ipad<kNumPads; ipad++) {
    tpcPad[ipad]->Draw();                   // Draw() adds it to current canvas
  }

  // Draw Canvas
  c->Draw();
  c->SetEditable(false);

  // Event Loop
  while (true)
  {
    // Reset event
    for (int ipad=0; ipad<kNumPads; ipad++) {
      hADC[ipad]->Reset();
    }

    // Decode event data, and fill hist
    //int status = DecodeNextEvent( acqFile );
    int status = DecodeNextEvent();
    if ( status != 0 )
      break;

    FillPadWithColor();

    // Update Canvas
    c->Update();
    c->Draw();

    gSystem->ProcessEvents();
    if ( gROOT->IsInterrupted()) {
      char ch;
      cout << "Ctrl-C is pressed. Do you want to Quit? [y]/n" << endl;
      cin >> ch;
      if ( ch == 'y' || ch == 'Y' )
	break;
      else {
        c->Draw();  // redraw
        sleep(3);
      }
    }

    
    // Need some time to fetch next event
    usleep(100);   // microsec
  }

  // Clean up and prepare to exit
  //acqFile.close();
  fclose(acqFile);
  
  for (int i=0; i<kNumPads; i++) {
    delete hADC[i];
    delete tpcPad[i];
  }

  delete c;

  //theApp.Run();

  return 0;
}
