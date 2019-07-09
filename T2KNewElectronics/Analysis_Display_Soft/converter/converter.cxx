/*******************************************************************************
                         PandaX-III / T2K-II
                         ___________________

File:        fdecoder.c

Description: This program decodes the binary data files recorded by the TDCM.


Author:      D. Calvet,        denis.calvetATcea.fr


History:
April 2019    : created from mreader.c

*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <fstream>

#include "../src/fdecoder.h"
#include "../src/datum_decoder.h"
#include "../src/platform_spec.h"
#include "../src/frame.h"

// Marion's classes
#include "../src/Mapping.h"
#include "../src/DAQ.h"
#include "../src/Pads.h"
#include "../src/Pixel.h"
#include "../src/T2KConstants.h"

// ROOT
#include "TH1D.h"
#include "TH2F.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH3.h"

typedef struct _Param
{
    char inp_dir[120];
    char inp_file[120];
    FILE *fsrc;
    int has_no_run;
    int show_run;
    unsigned int vflag;
    int sample_index_offset_zs;
} Param;

/*******************************************************************************
Global Variables
*******************************************************************************/
Param param;
Features fea;
DatumContext dc;
int verbose;
TH1I *hADCvsTIME[n::pads];
std::vector<long int> eventPos;
int firstEv;
Pixel P;

/**********************************************************************
Useful functions
**********************************************************************/
void scan()
{
    // Reset eventPos vector
    eventPos.clear();

    // Scan the file
    DatumContext_Init(&dc, param.sample_index_offset_zs);
    unsigned short datum;
    int err;
    bool done = true;
    int prevEvnum = -1;
    firstEv = -1;
    int evnum;
    while (done)
    {
        // Read one short word
        if (fread(&datum, sizeof(unsigned short), 1, param.fsrc) != 1)
        {
            done = false;
        }
        else
        {
            fea.tot_file_rd += sizeof(unsigned short);
            // Interpret datum
            if ((err = Datum_Decode(&dc, datum)) < 0)
            {
                printf("%d Datum_Decode: %s\n", err, &dc.ErrorString[0]);
                done = true;
            }
            else
            {
                if (dc.isItemComplete)
                {
                    if (dc.ItemType == IT_START_OF_EVENT)
                    {
                        //printf("Type : 0x%x \n", dc.ItemType);
                        evnum = (int)dc.EventNumber;
                        if (firstEv < 0)
                        {
                            printf("Is first event type : 0x%x  and number %u \n", dc.ItemType, dc.EventNumber);
                            eventPos.push_back(fea.tot_file_rd - sizeof(unsigned short));
                            //cout << dc.EventNumber << endl;
                            firstEv = evnum;
                            prevEvnum = evnum;
                        }
                        else if (evnum != prevEvnum)
                        {
                            eventPos.push_back(fea.tot_file_rd - sizeof(unsigned short));
                            prevEvnum = evnum;
                        }
                    }
                    else if (dc.ItemType == IT_ADC_SAMPLE)
                    {
                    }
                    else if (dc.ItemType == IT_DATA_FRAME)
                    {
                    }
                    else if (dc.ItemType == IT_END_OF_FRAME)
                    {
                    }
                    else if (dc.ItemType == IT_MONITORING_FRAME)
                    {
                    }
                    else if (dc.ItemType == IT_CONFIGURATION_FRAME)
                    {
                    }
                    else if (dc.ItemType == IT_SHORT_MESSAGE)
                    {
                    }
                    else if (dc.ItemType == IT_LONG_MESSAGE)
                    {
                    }
                    else if (dc.ItemType == IT_TIME_BIN_INDEX)
                    {
                    }
                    else if (dc.ItemType == IT_CHANNEL_HIT_HEADER)
                    {
                    }
                    else if (dc.ItemType == IT_DATA_FRAME)
                    {
                    }
                    else if (dc.ItemType == IT_NULL_DATUM)
                    {
                    }
                    else if (dc.ItemType == IT_CHANNEL_HIT_COUNT)
                    {
                    }
                    else if (dc.ItemType == IT_LAST_CELL_READ)
                    {
                    }
                    else if (dc.ItemType == IT_END_OF_EVENT)
                    {
                    }
                    else if (dc.ItemType == IT_PED_HISTO_MD)
                    {
                    }
                    else if (dc.ItemType == IT_UNKNOWN)
                    {
                    }
                    else if (dc.ItemType == IT_CHAN_PED_CORRECTION)
                    {
                    } //printf("Type : 0x%x \n", dc.ItemType);}
                    else if (dc.ItemType == IT_CHAN_ZERO_SUPPRESS_THRESHOLD)
                    {
                    } //printf("Type : 0x%x \n", dc.ItemType);}
                    else
                    {
                        printf("Unknow Item Type : 0x%04x\n", dc.ItemType);
                    }
                }
            }
        }
    }
    cout << "First event : " << firstEv << endl;
    cout << eventPos.size() << " events in the file..." << endl;
}

void clearPadMaxAmpl(int array[geom::nPadx][geom::nPady])
{
    for (int i = 0; i < geom::nPadx; i++)
    {
        for (int j = 0; j < geom::nPady; j++)
        {
            array[i][j] = 0;
        }
    }
}

void clearADCAmpl(int array[n::cards][n::chips][n::bins][n::samples])
{
    for (int i = 0; i < n::cards; i++)
    {
        for (int j = 0; j < n::chips; j++)
        {
            for (int k = 0; k < n::bins; k++)
            {
                for (int l = 0; l < n::samples; l++)
                {
                    array[i][j][k][l] = 0;
                }
            }
        }
    }
}

void clearMaxAmpl(int array[n::cards][n::chips][n::bins])
{
    for (int i = 0; i < n::cards; i++)
    {
        for (int j = 0; j < n::chips; j++)
        {
            for (int k = 0; k < n::bins; k++)
            {
                array[i][j][k] = 0;
            }
        }
    }
}

void clearPadAmpl(int array[geom::nPadx][geom::nPady][n::samples])
{
    for (int i = 0; i < geom::nPadx; i++)
    {
        for (int j = 0; j < geom::nPady; j++)
        {
            for (int k = 0; k < n::samples; k++)
            {
                array[i][j][k] = 0;
            }
        }
    }
}

int padNum(const int &i, const int &j) { return (j * geom::nPadx + i); }
int iFrompad(const int &padnum) { return (padnum % geom::nPadx); }
int jFrompad(const int &padnum) { return (padnum / geom::nPadx); }

void histoEventInit()
{
    gStyle->SetOptStat(0);

    for (int k = 0; k < n::pads; k++)
    {
        char histName[40];
        sprintf(histName, "hADCvsTIME %d", k);
        hADCvsTIME[k] = new TH1I(histName, histName, n::samples, 0, n::samples);
        hADCvsTIME[k]->SetMinimum(-1);
        hADCvsTIME[k]->Reset("ICESM");
        for (int i = 0; i < n::samples; i++)
        {
            hADCvsTIME[k]->Fill(i, 0);
        }
    }
}

/*******************************************************************************
help() to display usage
*******************************************************************************/
void help()
{
    printf("fdecoder <options>\n");
    printf("   -h                   : print this message help\n");
    printf("   -i <file>            : input file name\n");
    printf("   -zs_preamble <Value> : number of pre-samples below threshold in zero-suppressed mode\n");
    printf("   -v <level>           : verbose\n");
    printf("   -vflag <0xFlags>     : flags to determine printed items\n");
}

/*******************************************************************************
parse_cmd_args() to parse command line arguments
*******************************************************************************/
int parse_cmd_args(int argc, char **argv, Param *p)
{
    int i;
    int match = 0;
    int err = 0;

    for (i = 1; i < argc; i++)
    {

        // Input file name
        if (strncmp(argv[i], "-i", 2) == 0)
        {
            match = 1;
            if ((i + 2) < argc)
            {
                i++;
                strcpy(&(p->inp_dir[0]), argv[i]);
                strcpy(&(p->inp_file[0]), argv[i + 1]);
                printf("Input directory : %s", p->inp_dir);
                printf("\n");
                printf("Input file : %s", p->inp_file);
                printf("\n");
            }
            else
            {
                printf("mising argument after %s\n", argv[i]);
                return (-1);
            }
        }
    }
    return (0);
}

/*******************************************************************************
Main
*******************************************************************************/
int main(int argc, char **argv)
{
    unsigned short vdatum[10];
    int i;
    int err;
    bool done;
    int ntot;
    int timebin;
    double amp;

    string inputdir(argv[2]);
    string inputfile(argv[3]);

    string input_file_name = inputfile.substr(0, inputfile.size() - 4);

    DAQ daq;
    daq.loadDAQ();
    cout << "... DAQ loaded successfully" << endl;

    Mapping T2K;
    T2K.loadMapping();
    cout << "...Mapping loaded succesfully." << endl;

    Pads padPlane;
    padPlane.loadPadPlane(daq, T2K);
    cout << "...Pads loaded succesfully." << endl;

    // Default parameters
    //sprintf(param.inp_file, "C:\\users\\calvet\\projects\\bin\\pandax\\data\\R2018_11_27-15_24_07-000.aqs"); //Change default file path
    param.sample_index_offset_zs = 4;
    param.vflag = 0;
    param.has_no_run = 0;
    param.show_run = 0;
    verbose = 1;

    // Parse command line arguments
    if (parse_cmd_args(argc, argv, &param) < 0)
    {
        return (-1);
    }

    // Open input file
    char filename[300];
    strcpy(filename, param.inp_dir);
    strcat(filename, param.inp_file);
    printf("%s", filename);

    if (!(param.fsrc = fopen(filename, "rb")))
    {
        printf("could not open file %s.\n", param.inp_file);
        return (-1);
    }

    scan();
    ntot = eventPos.size();

    TFile *output_file = new TFile((loc::rootfiles + input_file_name + ".root").c_str(), "RECREATE");
    TTree *tree = new TTree("tree", "tree");

    Int_t eventNumber;
    Int_t ADCAmpl[n::cards][n::chips][n::bins][n::samples] = {};
    Int_t MaxAmpl[n::cards][n::chips][n::bins] = {};
    Int_t MaxAmplTimeSample[n::cards][n::chips][n::bins] = {};
    Int_t PadAmpl[geom::nPadx][geom::nPady][n::samples] = {};
    Int_t PadMaxAmpl[geom::nPadx][geom::nPady] = {};
    Int_t PadMaxAmplTimeSample[geom::nPadx][geom::nPady] = {};

    TBranch *b0 = tree->Branch("eventNumber", &eventNumber, "eventNumber/I");
    TBranch *b1 = tree->Branch("ADCAmpl", &ADCAmpl, TString::Format("ADCAmpl[%i][%i][%i][%i]/I", n::cards, n::chips, n::bins, n::samples));
    //TBranch *b2 = tree->Branch("MaxAmpl", &MaxAmpl, Form("MaxAmpl[%i][%i][%i]/D", n::cards, n::chips, n::bins));
    //TBranch *b3 = tree->Branch("MaxAmplTimeSample", &MaxAmplTimeSample, Form("MaxAmplTimeSample[%i][%i][%i]/I", n::cards, n::chips, n::bins));
    TBranch *b4 = tree->Branch("PadAmpl", &PadAmpl, Form("PadAmpl[%i][%i][%i]/I", geom::nPadx, geom::nPady, n::samples));
    //TBranch *b5 = tree->Branch("PadMaxAmpl", &PadMaxAmpl, Form("PadMaxAmpl[%i][%i]/I", geom::nPadx, geom::nPady));
    //TBranch *b6 = tree->Branch("PadMaxAmplTimeSample", &PadMaxAmplTimeSample, Form("PadMaxAmplTimeSample[%i][%i]/I", geom::nPadx, geom::nPady));

    if (verbose)
    {
        printf("Decoder    : Version %d.%d Compiled %s at %s\n", dc.DecoderMajorVersion, dc.DecoderMinorVersion, dc.DecoderCompilationDate, dc.DecoderCompilationTime);
        printf("Input file : %s\n", param.inp_file);
    }

    //for (int i = firstEv; i < firstEv + ntot; i++)
    for (int i = firstEv; i < firstEv + 30; i++) // test
    {
        if (i % 100 == 0)
        {
            cout << "\r"
                 << "\t"
                 << "... Processing event " << i << " ..." << flush;
        }

        // Clear & init
        histoEventInit();
        //clearPadMaxAmpl(PadMaxAmpl);
        //clearPadMaxAmpl(PadMaxAmplTimeSample);
        //clearMaxAmpl(MaxAmpl);
        //clearMaxAmpl(MaxAmplTimeSample);
        //clearPadAmpl(PadAmpl);
        //clearADCAmpl(ADCAmpl);
        amp = 0;
        timebin = 0;
        // Initialize the data interpretation context
        DatumContext_Init(&dc, param.sample_index_offset_zs);

        fseek(param.fsrc, eventPos[i - firstEv], SEEK_SET);
        // Scan the file
        int prevEvnum = -1;
        done = true;

        while (done)
        {
            // Read one short word
            //if (fread(&datum, sizeof(unsigned short), 1, param.fsrc) != 1)
            // clock_t start = clock();
            if (fread(&vdatum, sizeof(unsigned short), 10, param.fsrc) != 1)
            {
                //printf("\n");
                //printf("*** End of file reached ***\n");
                done = false;
            }
            else
            {
                for (int idatum = 0; idatum < 10; ++idatum)
                {
                    unsigned short datum = vdatum[idatum];
                    fea.tot_file_rd += sizeof(unsigned short);
                    // Interpret datum
                    if ((err = Datum_Decode(&dc, datum)) < 0)
                    {
                        printf("%d Datum_Decode: %s\n", err, &dc.ErrorString[0]);
                        done = true;
                    }
                    // Decode
                    if (dc.isItemComplete)
                    {
                        if (dc.ItemType == IT_START_OF_EVENT && dc.EventNumber != prevEvnum && dc.EventNumber == i)
                        {
                            eventNumber = (int)dc.EventNumber;
                            prevEvnum = eventNumber;
                            cout << eventNumber << endl;
                        }
                        else if (eventNumber == i && dc.ItemType == IT_ADC_SAMPLE) //dc.AbsoluteSampleIndex!=timesampleprev)
                        {

                            if (dc.ChannelIndex != 15 && dc.ChannelIndex != 28 && dc.ChannelIndex != 53 && dc.ChannelIndex != 66 && dc.ChannelIndex > 2 && dc.ChannelIndex < 79)
                            {
                                int a = 0;
                                int b = 0;
                                // histo and display
                                int x = T2K.i(dc.CardIndex, dc.ChipIndex, daq.connector(dc.ChannelIndex));
                                int y = T2K.j(dc.CardIndex, dc.ChipIndex, daq.connector(dc.ChannelIndex));
                                int k = padNum(x, y);
                                a = (int)dc.AbsoluteSampleIndex;
                                b = (int)dc.AdcSample;
                                if (b < 4096)
                                {
                                    hADCvsTIME[k]->Fill(a, b);
                                }
                                // Fill branches
                                int card = (int)dc.CardIndex;
                                int chip = (int)dc.ChipIndex;
                                int channel = (int)dc.ChannelIndex;
                                ADCAmpl[card][chip][channel][a] = b;
                                PadAmpl[x][y][a] = b;
                            }
                            else
                            {
                            }
                        }
                        else if (dc.ItemType == IT_DATA_FRAME)
                        {
                        }
                        else if (dc.ItemType == IT_END_OF_FRAME)
                        {
                        }
                        else if (dc.ItemType == IT_MONITORING_FRAME)
                        {
                        }
                        else if (dc.ItemType == IT_CONFIGURATION_FRAME)
                        {
                        }
                        else if (dc.ItemType == IT_SHORT_MESSAGE)
                        {
                        }
                        else if (dc.ItemType == IT_LONG_MESSAGE)
                        {
                        }
                        else if (eventNumber == i && dc.ItemType == IT_TIME_BIN_INDEX)
                        {
                        }
                        else if (eventNumber == i && dc.ItemType == IT_CHANNEL_HIT_HEADER)
                        {
                        }
                        else if (dc.ItemType == IT_DATA_FRAME)
                        {
                        }
                        else if (dc.ItemType == IT_NULL_DATUM)
                        {
                        }
                        else if (dc.ItemType == IT_CHANNEL_HIT_COUNT)
                        {
                        }
                        else if (dc.ItemType == IT_LAST_CELL_READ)
                        {
                        }
                        else if (dc.ItemType == IT_END_OF_EVENT)
                        {
                        }
                        else if (dc.ItemType == IT_PED_HISTO_MD)
                        {
                        }
                        else if (dc.ItemType == IT_UNKNOWN)
                        {
                        }
                        else if (dc.ItemType == IT_CHAN_PED_CORRECTION)
                        {
                        } //printf("Type : 0x%x \n", dc.ItemType);}
                        else if (dc.ItemType == IT_CHAN_ZERO_SUPPRESS_THRESHOLD)
                        {
                        } //printf("Type : 0x%x \n", dc.ItemType);}
                        else if (eventNumber > i)
                        {
                            done = false;
                        } //}&& dc.ItemType==IT_START_OF_EVENT
                        else
                        {
                        }
                    }
                }
            }
        }
        /*
			for (int q=0; q<n::pads; q++)
			{
				P = padPlane.pad(iFrompad(q),jFrompad(q));
				//amp = hADCvsTIME[q]->GetMaximum();
				//cout << "Max amplitude : " << amp << endl;
				//cout << "PadMaxAmp before filling : " << PadMaxAmpl[iFrompad(q)][jFrompad(q)] << endl;
					hADCvsTIME[q]->GetXaxis()->SetRange(1, 511);
					timebin = hADCvsTIME[q]->GetMaximumBin();
					amp = double(hADCvsTIME[q]->GetMaximum());
					//if (amp >4096) {cout << "amp :" << amp << "\t timebin: " << timebin << endl;}

				// Fill branches
				//cout << "Carte : " << P.card() << " Chip : " << P.chip() << " Channel : " << P.channel() << endl;
				MaxAmpl[P.card()][P.chip()][P.channel()] = amp; // channel 0,1,2,15,28,53,66,79,80,81 will stay at 0
				PadMaxAmpl[iFrompad(q)][jFrompad(q)] = amp;
				//cout << "PadMaxAmp after filling : " << PadMaxAmpl[iFrompad(q)][jFrompad(q)] << endl;
				MaxAmplTimeSample[P.card()][P.chip()][P.channel()] = timebin;
				PadMaxAmplTimeSample[iFrompad(q)][jFrompad(q)] = timebin;
			}
*/
        // Fill in the tree
        tree->Fill();

        // Delete histos
        for (int p = 0; p < n::pads; p++)
        {
            delete hADCvsTIME[p];
        }

        // Empty all branches
        // Elec
        for (int i = 0; i < n::cards; i++)
        {
            for (int j = 0; j < n::chips; j++)
            {
                for (int k = 0; k < n::bins; k++)
                {
                    MaxAmpl[i][j][k] = 0;
                    MaxAmplTimeSample[i][j][k] = 0;
                    for (int l = 0; l < n::samples; l++)
                    {
                        ADCAmpl[i][j][k][l] = 0;
                    }
                }
            }
        }
        // Pads
        for (int i = 0; i < geom::nPadx; i++)
        {
            for (int j = 0; j < geom::nPady; j++)
            {
                PadMaxAmpl[i][j] = 0;
                PadMaxAmplTimeSample[i][j] = 0;
                for (int k = 0; k < n::samples; k++)
                {
                    PadAmpl[i][j][k] = 0;
                }
            }
        }
    }

    	tree->Write("", TObject::kOverwrite);

    output_file->Write();
    output_file->Close();

    // Close file if it has been opened
    if (param.fsrc)
    {
        fclose(param.fsrc);
    }

    std::cout << "Finished" << std::endl;

    return (0);
}
