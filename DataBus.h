
#ifndef DataBus_h
#define DataBus_h
#include "ReservationStation.h"

#include <vector>
#include <utility>
using namespace std;

struct DataBus
{
    //Attributes
        bool*    Busy;
        int16_t* Data;
        int*     FUID;
    
    //Connections
        vector <ReservationStation*>*     ReservationStations;
        vector <int16_t>            *     RegisterFile;
        vector <int>*                     RegisterStatus;
    
    //Methods
        void    reflect();
        void    clear();
        bool    reserve(int16_t iData, int iFUID);
    
    //Constructor
        DataBus (bool* Busy, int16_t* Data, int* FUID, vector <ReservationStation*>* iReservationStations,vector <int16_t>* iRegisterFile,vector <int>* iRegisterStatus);
};

//Methods Declaration
bool DataBus::reserve(int16_t iData, int iFUID)
{
    //Redundant
    if (Busy) return false;
    else
    {
        *Busy = true;
        *Data = iData;
        *FUID = iFUID;
        return true;
    }
}

void DataBus::reflect()
{
    if (!Busy || *FUID == 0) return;
    //Loop over all the reservation stations and check if any of them needs the value
    for (int i = 0; i<ReservationStations->size(); i++)
    {
        if (ReservationStations->at(i)->Qj == *FUID)
        {
            ReservationStations->at(i)->Qj = 0;
            ReservationStations->at(i)->Vj = *Data;
        }
        
        if (ReservationStations->at(i)->Qk == *FUID)
        {
            ReservationStations->at(i)->Qk = 0;
            ReservationStations->at(i)->Vk = *Data;
        }
    }
    
    //Loop over all the registers and check if any of them needs the value (except R0)
    for (int i = 1; i<RegisterStatus->size(); i++)
    {
        
        if (RegisterStatus->at(i) != 0 && RegisterStatus->at(i) == *FUID)
        {
            RegisterStatus->at(i) = 0;
            RegisterFile->at(i) = *Data;
        }
        
    }
    
    //Clear the station that requested the BUS
    
    for (int i = 0; i<ReservationStations->size(); i++)
    {
        if (ReservationStations->at(i)->ID == *FUID)
        {
            ReservationStations->at(i)->clearQueue();
            ReservationStations->at(i)->clearStation();
        }
    }
        
    //Clear the BUS
    clear();
}

void DataBus::clear()
{
    *Busy = false;
    *Data = 0;
    *FUID = 0;
}

//Constructor Declaration
DataBus::DataBus (bool* Busy, int16_t* Data, int* FUID, vector <ReservationStation*>* iReservationStations,vector <int16_t>* iRegisterFile,vector <int>* iRegisterStatus)
{
    this->Busy = Busy;
    this->Data = Data;
    this->FUID = FUID;
    
    ReservationStations = iReservationStations;
    RegisterFile        = iRegisterFile;
    RegisterStatus      = iRegisterStatus;
    
    *Busy = false;
    *Data = 0;
    *FUID = 0;
}

#endif /* DataBus_h */


