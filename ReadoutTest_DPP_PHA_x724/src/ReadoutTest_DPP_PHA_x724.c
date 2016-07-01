/******************************************************************************
*
* CAEN SpA - Front End Division
* Via Vetraia, 11 - 55049 - Viareggio ITALY
* +390594388398 - www.caen.it
*
***************************************************************************//**
* \note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the
* software, documentation and results solely at his own risk.
******************************************************************************/

//#include <iostream>
#include <CAENDigitizer.h>

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include <bitset>

#include <stdexcept>
#include <string>


//#define INDIVIDUAL_TRIGGER_INPUTS
// The following define must be set to the actual number of connected boards
#define MAXNB   1
// NB: the following define MUST specify the ACTUAL max allowed number of board's channels
// it is needed for consistency inside the CAENDigitizer's functions used to allocate the memory
#define MaxNChannels 4

#define MAXNBITS 14

/* include some useful functions from file Functions.c
you can find this file in the src directory */
#include "Functions.h"

/* ###########################################################################
*  Functions
*  ########################################################################### */

/* --------------------------------------------------------------------------------------------------------- */
/*! \fn      int ProgramDigitizer(int handle, DigitizerParams_t Params, CAEN_DGTZ_DPPParamsPHA_t DPPParams)
*   \brief   Program the registers of the digitizer with the relevant parameters
*   \return  0=success; -1=error */
/* --------------------------------------------------------------------------------------------------------- */
int ProgramDigitizer(int handle, DigitizerParams_t Params, CAEN_DGTZ_DPP_PHA_Params_t DPPParams)
{
    /* This function uses the CAENDigitizer API functions to perform the digitizer's initial configuration */
    int i, ret = 0;

    /* Reset the digitizer */
    ret |= CAEN_DGTZ_Reset(handle);

    if (ret) {
        printf("ERROR: can't reset the digitizer.\n");
        return -1;
    }
    ret |= CAEN_DGTZ_WriteRegister(handle, 0x8000, 0x01000114);  // Channel Control Reg (indiv trg, seq readout) ??

    /* Set the DPP acquisition mode
    This setting affects the modes Mixed and List (see CAEN_DGTZ_DPP_AcqMode_t definition for details)
    CAEN_DGTZ_DPP_SAVE_PARAM_EnergyOnly        Only energy (DPP-PHA) or charge (DPP-PSD/DPP-CI v2) is returned
    CAEN_DGTZ_DPP_SAVE_PARAM_TimeOnly        Only time is returned
    CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime    Both energy/charge and time are returned
    CAEN_DGTZ_DPP_SAVE_PARAM_None            No histogram data is returned */
    ret |= CAEN_DGTZ_SetDPPAcquisitionMode(handle, Params.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    
    // Set the digitizer acquisition mode (CAEN_DGTZ_SW_CONTROLLED or CAEN_DGTZ_S_IN_CONTROLLED)
    ret |= CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
    
    // Set the number of samples for each waveform
    ret |= CAEN_DGTZ_SetRecordLength(handle, Params.RecordLength);

    // Set the I/O level (CAEN_DGTZ_IOLevel_NIM or CAEN_DGTZ_IOLevel_TTL)
    ret |= CAEN_DGTZ_SetIOLevel(handle, Params.IOlev);

    /* Set the digitizer's behaviour when an external trigger arrives:

    CAEN_DGTZ_TRGMODE_DISABLED: do nothing
    CAEN_DGTZ_TRGMODE_EXTOUT_ONLY: generate the Trigger Output signal
    CAEN_DGTZ_TRGMODE_ACQ_ONLY = generate acquisition trigger
    CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT = generate both Trigger Output and acquisition trigger

    see CAENDigitizer user manual, chapter "Trigger configuration" for details */
    ret |= CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);

    // Set the enabled channels
    ret |= CAEN_DGTZ_SetChannelEnableMask(handle, Params.ChannelMask);

    // Set how many events to accumulate in the board memory before being available for readout
    ret |= CAEN_DGTZ_SetDPPEventAggregation(handle, Params.EventAggr, 0);
    
    /* Set the mode used to syncronize the acquisition between different boards.
    In this example the sync is disabled */
    ret |= CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled);
    
    // Set the DPP specific parameters for the channels in the given channelMask
    ret |= CAEN_DGTZ_SetDPPParameters(handle, Params.ChannelMask, &DPPParams);
    
    for(i=0; i<MaxNChannels; i++) {
        if (Params.ChannelMask & (1<<i)) {
            // Set a DC offset to the input signal to adapt it to digitizer's dynamic range
            ret |= CAEN_DGTZ_SetChannelDCOffset(handle, i, 40000);
            
            // Set the Pre-Trigger size (in samples)
            ret |= CAEN_DGTZ_SetDPPPreTriggerSize(handle, i, 100);
            
            // Set the polarity for the given channel (CAEN_DGTZ_PulsePolarityPositive or CAEN_DGTZ_PulsePolarityNegative)
            ret |= CAEN_DGTZ_SetChannelPulsePolarity(handle, i, Params.PulsePolarity);
        }
    }

  
  
    ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, ANALOG_TRACE_1, CAEN_DGTZ_DPP_VIRTUALPROBE_Delta2);
    ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_None);
    ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, DIGITAL_TRACE_1, CAEN_DGTZ_DPP_DIGITALPROBE_Peaking);

    if (ret) {
        printf("Warning: errors found during the programming of the digitizer.\nSome settings may not be executed\n");
        return ret;
    } else {
        return 0;
    }
}

std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}


/* ########################################################################### */
/* MAIN                                                                        */
/* ########################################################################### */
int main(int argc, char *argv[])
{

  std::string outputname(argv[1]);
  std::ofstream outputfile;
  outputfile.open(outputname.c_str());

    /* The following variable is the type returned from most of CAENDigitizer
    library functions and is used to check if there was an error in function
    execution. For example:
    ret = CAEN_DGTZ_some_function(some_args);
    if(ret) printf("Some error"); */
    //CAEN_DGTZ_ErrorCode ret;
    int ret;

    /* Buffers to store the data. The memory must be allocated using the appropriate
    CAENDigitizer API functions (see below), so they must not be initialized here
    NB: you must use the right type for different DPP analysis (in this case PHA) */
    char *buffer = NULL;                                 // readout buffer
    CAEN_DGTZ_DPP_PHA_Event_t       *Events[MaxNChannels];  // events buffer

    /* The following variables will store the digitizer configuration parameters */
    CAEN_DGTZ_DPP_PHA_Params_t DPPParams[MAXNB];
    DigitizerParams_t Params[MAXNB];

    /* Arrays for data analysis */
    uint64_t PrevTime[MAXNB][MaxNChannels];
    uint64_t ExtendedTT[MAXNB][MaxNChannels];
    int ECnt[MAXNB][MaxNChannels];
    int TrgCnt[MAXNB][MaxNChannels];
    int PurCnt[MAXNB][MaxNChannels];

    /* The following variable will be used to get an handler for the digitizer. The
    handler will be used for most of CAENDigitizer functions to identify the board */
    int handle[MAXNB];

    /* Other variables */
    int i, b, ch, ev;
    int Quit=0;
    int AcqRun = 0;
    uint32_t AllocatedSize, BufferSize;
    int Nb=0;
    int MajorNumber;
    int BitMask = 0;
    uint64_t CurrentTime, PrevRateTime, ElapsedTime, StartingTime;
    uint32_t NumEvents[MaxNChannels];
    CAEN_DGTZ_BoardInfo_t           BoardInfo; // Needed to check firmware release

    for (i = 0; i < MAXNBITS; i++)
        BitMask |= 1<<i; /* Create a bit mask based on number of bits of the board */

    /* *************************************************************************************** */
    /* Set Parameters                                                                          */
    /* *************************************************************************************** */
    memset(&Params, 0, MAXNB * sizeof(DigitizerParams_t));
    memset(&DPPParams, 0, MAXNB * sizeof(CAEN_DGTZ_DPP_PHA_Params_t));
    for (b = 0; b < MAXNB; b++) {

        /****************************\
        * Communication Parameters   *
        \****************************/
        // Direct USB connection
        Params[b].LinkType = CAEN_DGTZ_USB;  // Link Type
        Params[b].VMEBaseAddress = 0;  // For direct USB connection, VMEBaseAddress must be 0        
        Params[b].IOlev = CAEN_DGTZ_IOLevel_NIM;
        /****************************\
        *  Acquisition parameters    *
        \****************************/
        Params[b].AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_List;          // CAEN_DGTZ_DPP_ACQ_MODE_List or CAEN_DGTZ_DPP_ACQ_MODE_Oscilloscope
        Params[b].RecordLength = 5000;                              // Num of samples of the waveforms (only for Oscilloscope mode)
        Params[b].ChannelMask = 0x5;                               // Channel enable mask
        Params[b].EventAggr = 1;                                   // number of events in one aggregate (0=automatic)
        Params[b].PulsePolarity = CAEN_DGTZ_PulsePolarityPositive; // Pulse Polarity (this parameter can be individual)

        /****************************\
        *      DPP parameters        *
        \****************************/
        for(ch=0; ch<MaxNChannels; ch++) {
            DPPParams[b].thr[ch] = 100;   // Trigger Threshold
            DPPParams[b].k[ch] = 3000;     // Trapezoid Rise Time (N*10ns)
            DPPParams[b].m[ch] = 2000;      // Trapezoid Flat Top  (N*10ns)
            DPPParams[b].M[ch] = 300000;      // Decay Time Constant (N*10ns) HACK-FPEP the one expected from fitting algorithm?
            DPPParams[b].ftd[ch] = 1600;    // Flat top delay (peaking time) (N*10ns) ??
            DPPParams[b].a[ch] = 4;       // Trigger Filter smoothing factor
            DPPParams[b].b[ch] = 300;     // Input Signal Rise time (N*10ns)
            DPPParams[b].trgho[ch] = 1300;  // Trigger Hold Off
            DPPParams[b].nsbl[ch] = 4; // 3 = bx10 = 64 samples
            DPPParams[b].nspk[ch] = 1;
            DPPParams[b].pkho[ch] = 2000;   // peak hold off
            DPPParams[b].blho[ch] = 1000;   // baseline hold off
            DPPParams[b].enf[ch] = 1.0; // Energy Normalization Factor
            DPPParams[b].decimation[ch] = 0;
            DPPParams[b].dgain[ch] = 0;
            DPPParams[b].otrej[ch] = 0;
            DPPParams[b].trgwin[ch] = 0;
            DPPParams[b].twwdt[ch] = 0;
            //DPPParams[b].tsampl[ch] = 10;
            //DPPParams[b].dgain[ch] = 1;
	    
	    if(ch == 1){
	      DPPParams[b].k[ch] = 1500;
	      DPPParams[b].M[ch] = 5200;
	      DPPParams[b].m[ch] = 3000;
	      
	    }

	    
        }
    }


    /* *************************************************************************************** */
    /* Open the digitizer and read board information                                           */
    /* *************************************************************************************** */
    /* The following function is used to open the digitizer with the given connection parameters
    and get the handler to it */
    for(b=0; b<MAXNB; b++) {
        /* IMPORTANT: The following function identifies the different boards with a system which may change
        for different connection methods (USB, Conet, ecc). Refer to CAENDigitizer user manual for more info.
        Some examples below */
        
        /* The following is for b boards connected via b USB direct links
        in this case you must set Params[b].LinkType = CAEN_DGTZ_USB and Params[b].VMEBaseAddress = 0 */
        ret = CAEN_DGTZ_OpenDigitizer(Params[b].LinkType, b, 0, Params[b].VMEBaseAddress, &handle[b]);

        if (ret) {
	  std::cout << "Can't open digitizer " << ret << std::endl;
	  std::cout << Params[b].LinkType << " " << b << " " << Params[b].VMEBaseAddress << std::endl;
	  goto QuitProgram;    
        }
        
        /* Once we have the handler to the digitizer, we use it to call the other functions */
        ret = CAEN_DGTZ_GetInfo(handle[b], &BoardInfo);
        if (ret) {
            printf("Can't read board info\n");
            goto QuitProgram;
        }
        printf("\nConnected to CAEN Digitizer Model %s, recognized as board %d\n", BoardInfo.ModelName, b);
        printf("ROC FPGA Release is %s\n", BoardInfo.ROC_FirmwareRel);
        printf("AMC FPGA Release is %s\n", BoardInfo.AMC_FirmwareRel);

        /* Check firmware revision (only DPP firmwares can be used with this Demo) */
        sscanf(BoardInfo.AMC_FirmwareRel, "%d", &MajorNumber);
        if (MajorNumber != V1724_DPP_PHA_CODE &&
            MajorNumber != V1730_DPP_PHA_CODE) {
            printf("This digitizer has not a DPP-PHA firmware\n");
            goto QuitProgram;
        }
    }

    /* *************************************************************************************** */
    /* Program the digitizer (see function ProgramDigitizer)                                   */
    /* *************************************************************************************** */
    for (b = 0; b < MAXNB; b++) {
        ret = ProgramDigitizer(handle[b], Params[b], DPPParams[b]);
        if (ret) {
            printf("Failed to program the digitizer\n");
            goto QuitProgram;
        }
    }

    /* WARNING: The mallocs MUST be done after the digitizer programming,
    because the following functions needs to know the digitizer configuration
    to allocate the right memory amount */
    /* Allocate memory for the readout buffer */
    ret = CAEN_DGTZ_MallocReadoutBuffer(handle[0], &buffer, &AllocatedSize);
    /* Allocate memory for the events */
    ret |= CAEN_DGTZ_MallocDPPEvents(handle[0], (void**)(Events), &AllocatedSize); 
    if (ret) {
        printf("Can't allocate memory buffers\n");
        goto QuitProgram;    
    }

        
    /* *************************************************************************************** */
    /* Readout Loop                                                                            */
    /* *************************************************************************************** */
    // Clear counters
    for (b = 0; b < MAXNB; b++) {
        for (ch = 0; ch < MaxNChannels; ch++) {
            TrgCnt[b][ch] = 0;
            ECnt[b][ch] = 0;
            PrevTime[b][ch] = 0;
            ExtendedTT[b][ch] = 0;
            PurCnt[b][ch] = 0;
        }
    }
    PrevRateTime = get_time();
    AcqRun = 0;
    PrintInterface();
    printf("Type a command: ");
    while(!Quit) {
        // Check keyboard
        if(kbhit()) {
            char c;
            c = getch();
            if (c == 'q')  Quit = 1;
            if (c == 't')
                for (b = 0; b < MAXNB; b++)
                    CAEN_DGTZ_SendSWtrigger(handle[b]); // Send a software trigger to each board
            if (c == 'r')  {
                for (b = 0; b < MAXNB; b++) {
                    CAEN_DGTZ_SWStopAcquisition(handle[b]); 
                    printf("Restarted\n");
                    CAEN_DGTZ_ClearData(handle[b]);
                    CAEN_DGTZ_SWStartAcquisition(handle[b]);
                }
            }
            if (c == 's')  {
                for (b = 0; b < MAXNB; b++) {
                    // Start Acquisition
                    // NB: the acquisition for each board starts when the following line is executed
                    // so in general the acquisition does NOT starts syncronously for different boards
                    CAEN_DGTZ_SWStartAcquisition(handle[b]);
                    printf("Acquisition Started for Board %d\n", b);
                }
		StartingTime = get_time();
                AcqRun = 1;
            }
            if (c == 'S')  {
                for (b = 0; b < MAXNB; b++) {
                    // Stop Acquisition
                    CAEN_DGTZ_SWStopAcquisition(handle[b]); 
                    printf("Acquisition Stopped for Board %d\n", b);
                }
                AcqRun = 0;
            }
        }
        if (!AcqRun) {
            Sleep(10);
            continue;
        }
    
        /* Calculate throughput and trigger rate (every second) */
        CurrentTime = get_time();
	uint64_t TotalTime = CurrentTime - StartingTime;
		if(TotalTime > 300000) goto QuitProgram; // Uncomment it!!
        ElapsedTime = CurrentTime - PrevRateTime; /* milliseconds */
        if (ElapsedTime > 1000) {
            system(CLEARSCR);
            PrintInterface();
	    std::cout << "Elapsed time: " << TotalTime/1000. << " s" << std::endl;
            printf("Readout Rate=%.2f MB\n", (float)Nb/((float)ElapsedTime*1048.576f));
            for(b=0; b<MAXNB; b++) {
                printf("\nBoard %d:\n",b);
                for(i=0; i<MaxNChannels; i++) {
                    if (TrgCnt[b][i]>0)
                        printf("\tCh %d:\tTrgRate=%.2f KHz\tPileUpRate=%.2f%%\n", i, (float)TrgCnt[b][i]/(float)ElapsedTime, (float)PurCnt[b][i]*100/(float)TrgCnt[b][i]);
                    else
                        printf("\tCh %d:\tNo Data\n", i);
                    TrgCnt[b][i]=0;
                    PurCnt[b][i]=0;
                }
            }
            Nb = 0;
            PrevRateTime = CurrentTime;
            printf("\n\n");
        }
        
        /* Read data from the boards */
        for (b = 0; b < MAXNB; b++) {
            /* Read data from the board */
            ret = CAEN_DGTZ_ReadData(handle[b], CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize);
            if (ret) {
	      std::cout << "Readout Error: " << ret << std::endl;
                goto QuitProgram;    
            }
            if (BufferSize == 0)
                continue;
	    
            Nb += BufferSize;
            //ret = DataConsistencyCheck((uint32_t *)buffer, BufferSize/4);
            ret |= CAEN_DGTZ_GetDPPEvents(handle[b], buffer, BufferSize, (void**)(Events), NumEvents);
            if (ret) {
                printf("Data Error: %d\n", ret);
                goto QuitProgram;
            }

            /* Analyze data */
            //for(b=0; b<MAXNB; b++) printf("%d now: %d\n", b, Params[b].ChannelMask);
            for (ch = 0; ch < MaxNChannels; ch++) {
                if (!(Params[b].ChannelMask & (1<<ch)))
                    continue;
		//if(NumEvents[ch] != 0) outputfile << "### " << ch << " " << NumEvents[ch] << " " << Events[ch][0].TimeTag << " " << Events[ch][NumEvents[ch]-1].TimeTag << std::endl;
		//Time Tag: this is the trigger time stamp, i.e. the time of arrival of the pulse. It is espressed in sampling clock unit,
		//therefore the user has to multiply for the corresponding smapling clock to get the value in ns. Sampling clock
		//values are: 10 ns for x724


                /* Update output file */
                for (ev = 0; ev < NumEvents[ch]; ev++) {
                    TrgCnt[b][ch]++;
                    /* Time Tag */
                    if (Events[ch][ev].TimeTag < PrevTime[b][ch]) 
                        ExtendedTT[b][ch]++;
                    PrevTime[b][ch] = Events[ch][ev].TimeTag;
                    /* Energy */
		    if (Events[ch][ev].Energy > 0){
		      //outputfile << Events[ch][ev].Energy << " " << std::bitset<16>(Events[ch][ev].Energy).to_string() << std::endl;
		      outputfile << ch << " " << Events[ch][ev].TimeTag << " " << ((Events[ch][ev].Energy)&BitMask) 
				 << " " << Events[ch][ev].Extras << " " << Events[ch][ev].Extras2 << std::endl;
                        ECnt[b][ch]++;
                    } else {  /* PileUp */
                        PurCnt[b][ch]++;
                    }
                } // loop on events
            } // loop on channels
        } // loop on boards
    } // End of readout loop


QuitProgram:
    /* stop the acquisition, close the device and free the buffers */
    for (b =0 ; b < MAXNB; b++) {
        CAEN_DGTZ_SWStopAcquisition(handle[b]);
        CAEN_DGTZ_CloseDigitizer(handle[b]);
    }
    CAEN_DGTZ_FreeReadoutBuffer(&buffer);
    CAEN_DGTZ_FreeDPPEvents(handle[0], (void**)(Events));

    outputfile.close();
    exec("paplay /usr/share/sounds/ubuntu/ringtones/Bliss.ogg");
    return ret;
}
    
