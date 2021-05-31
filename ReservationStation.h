#ifndef ReservationStation_h
#define ReservationStation_h
#include "DataBus.h"
#include <vector>
#include <iostream>
#include <iomanip>
using namespace std;

struct ReservationStation
{
    
    //RS Identification
    int ID;
    
    //Systemtic Attributes
    pair <int, int> SupportedTypes;     //Range of supported instructions
    Instruction* currentInstruction;    //Pointer to the instruction currently in the Reservation Station
    
    //Flags
    bool Busy;                          //Flag to show if reservation station is busy

    //Run-time attributes
    int executionCycles;
    int remainingCycles;                //Number of execution cycles left
    
    //Classical Attributes
    int CalculatedValue;
    int Op;
    int Vj;
    int Vk;
    int Qj;
    int Qk;
    int A;
    
    //Utility Functions
    bool isMatch (int instType);                   //Function that returns if this RS matches the type we are looking for
    void clearStation();                           //Function that clears the reservation station
    void clearQueue();                             //Function that clears the queues
    void clearAll()
    {
        clearQueue();
        clearStation();
    }
    void flushQueue();
    
    //Derivative flag
    bool afterBranch;

    
    //Main function
    void play(int Cycle);                          //Function that simulates one cycle


    //Stages
    void issue (Instruction* Instruction, int Cycle);   //Function that simulates the issue stage
    void start_exe (int Cycle);                         //Function that simulates the last stage of the exe stage
    void end_exe   (int Cycle);                         //Function that simulates the last stage of the exe stage
    void wb        (int Cycle);                         //Function that simulates the wb stage

    //Constructor
    ReservationStation();
    
    //Wires connected from the main module
        //Run-time data
        float* PC;
        
        //Memory
        vector <int>*                     DataMemory;

        //Register File
        vector <int16_t>*                 RegisterFile;
        
        //Register Status
        vector <int>*                     RegisterStatus;
             
        //Load-Store Queue
        vector <ReservationStation*>*     LoadStoreQueue;

        //Instruction Queue
        vector <ReservationStation*>*     InstructionQueue;

        //Flags
        bool* issueStall;
        bool* exeStall;
        
        //Branch Statistics
        float* totalBranches;
        float* wrongBranches;
        
        //DataBus Ports
        bool*       DB_Busy;
        int16_t*    DB_Data;
        int *       DB_FUID;
    
};

//Main Function
void ReservationStation::play(int Cycle)
{
    if (!Busy) return;
    //cout<<currentInstruction->Inst<<" is at "<<remainingCycles<<" "<<executionCycles<<" "<<endl;
    if (remainingCycles == executionCycles)                         //Start executing
    {
        //Do not execute if youre a load-store not head of queue, one of the operands are not ready, or there is an execution stall
        if (((Op == 1 || Op == 2) && (LoadStoreQueue->at(0)->ID!= ID)) || (Qj != 0) || (Qk != 0)|| (*exeStall && afterBranch)) return;
        
        start_exe(Cycle);
        if (executionCycles == 1) end_exe(Cycle);
        remainingCycles--;
    }
    else if (remainingCycles == 1)                                  //Finish Executing
    {
        end_exe(Cycle);
        remainingCycles--;
    }
    else if (remainingCycles == 0)                                  //Write Back;
    {
        wb (Cycle);
    }
    else remainingCycles--;                                         //During Actual Execution, just subtract
}



//Stages
void ReservationStation::issue       (Instruction* Instruction, int Cycle)
{
    //General statements
        currentInstruction              = Instruction;
        currentInstruction->issueCycle  = Cycle;

        Busy                            = true;
        Op                              = currentInstruction->Type;
        afterBranch                     = *exeStall;
    
    //Instruction Specific
    switch(Op)
    {
        case 1: //LW rd, imm(rs1)
        {
            //Check RS1
            if (RegisterStatus->at(currentInstruction->rs1) != 0)
                Qj = RegisterStatus->at(currentInstruction->rs1);
            else
                Vj = RegisterFile->at(currentInstruction->rs1);
            
            //Check
            A = currentInstruction->imm;
            
            //Set this unit as the source for the register RD
            RegisterStatus->at(currentInstruction->rd) = ID;

            break;
        }
            
        case 2: //SW rs2, imm(rs1)
        {
            //Check RS1
            if (RegisterStatus->at(currentInstruction->rs1) != 0)
                Qj = RegisterStatus->at(currentInstruction->rs1);
            else
                Vj = RegisterFile->at(currentInstruction->rs1);
            
            
            //Check RS2
              if (RegisterStatus->at(currentInstruction->rs2) != 0)
                  Qk = RegisterStatus->at(currentInstruction->rs2);
              else
                  Vk = RegisterFile->at(currentInstruction->rs2);
            
            //Place imm in A
            A = currentInstruction->imm;
                        
            break;
        }
        
        case 3: //BEQ
        {
            //Check RS1
            if (RegisterStatus->at(currentInstruction->rs1) != 0)
                Qj = RegisterStatus->at(currentInstruction->rs1);
            else
                Vj = RegisterFile->at(currentInstruction->rs1);
            
            //Check RS2
              if (RegisterStatus->at(currentInstruction->rs2) != 0)
                  Qk = RegisterStatus->at(currentInstruction->rs2);
              else
                  Vk = RegisterFile->at(currentInstruction->rs2);
            
            //Place Immediate in A?? where PC 
                A = currentInstruction->imm + *PC;
            
            //Stall execution
                *exeStall = true;
            
                break;
        }
        
        case 4: //JALR
        {
            //Waits for RS1
            if (RegisterStatus->at(currentInstruction->rs1) != 0)
                Qj = RegisterStatus->at(currentInstruction->rs1);
            else
                Vj = RegisterFile->at(currentInstruction->rs1);
            
            A = *PC;


            //Want to store in R1
            RegisterStatus->at(1) = ID;
            
            *issueStall = true;
            break;
        }
        case 5: //RET
        {
            //Check and wait for R1
            if (RegisterStatus->at(1) != 0)
                Qj = RegisterStatus->at(1);
            else
                Vj = RegisterFile->at(1);
            
            *issueStall = true;

            break;
        }
        case 6: //ADD rd, rs1, rs2
        {
            //Check RS1
            if (RegisterStatus->at(currentInstruction->rs1) != 0)
                Qj = RegisterStatus->at(currentInstruction->rs1);
            else
                Vj = RegisterFile->at(currentInstruction->rs1);
            
            //Check RS2
            if (RegisterStatus->at(currentInstruction->rs2) != 0)
                Qk = RegisterStatus->at(currentInstruction->rs2);
            else
                Vk = RegisterFile->at(currentInstruction->rs2);
            
            //Set this unit as the source for the register RD
            RegisterStatus->at(currentInstruction->rd) = ID;
            
            
            break;
        }
        case 7: //NEG rd, rs1
        {
            //Check RS1
            if (RegisterStatus->at(currentInstruction->rs1) != 0)
                Qj = RegisterStatus->at(currentInstruction->rs1);
            else
                Vj = RegisterFile->at(currentInstruction->rs1);
            
            //Set this unit as the source for the register RD
            RegisterStatus->at(currentInstruction->rd) = ID;
            
            break;
            
        }
        case 8: //ADDI rd, rs1, imm
        {
            //Check RS1
            if (RegisterStatus->at(currentInstruction->rs1) != 0)
                Qj = RegisterStatus->at(currentInstruction->rs1);
            else
                Vj = RegisterFile->at(currentInstruction->rs1);
            
            //Check Imm
                Vk = currentInstruction->imm;
                        
            //Set this unit as the source for the register RD
            RegisterStatus->at(currentInstruction->rd) = ID;
                        
            
            break;
        }
        case 9: //DIV rd, rs1, rs2
        {
            //Check RS1
            if (RegisterStatus->at(currentInstruction->rs1) != 0)
                Qj = RegisterStatus->at(currentInstruction->rs1);
            else
                Vj = RegisterFile->at(currentInstruction->rs1);
            
            //Check RS2
            if (RegisterStatus->at(currentInstruction->rs2) != 0)
                Qk = RegisterStatus->at(currentInstruction->rs2);
            else
                Vk = RegisterFile->at(currentInstruction->rs2);
            
            //Set this unit as the source for the register RD
            RegisterStatus->at(currentInstruction->rd) = ID;
            
            
            break;
        }
            
    }
        
}
void ReservationStation::start_exe   (int Cycle)
{
    currentInstruction->startCycle = Cycle;
    switch(Op)
       {
           case 1: //LW rd, imm(rs1)
           {
               //Place address in A
               A = A + Vj;
               
               //Calculated Value = Value from Memory
               CalculatedValue = DataMemory->at(A);
               break;
           }
               
           case 2: //SW rs2, imm(rs1)
           {
               //Place address in A
                A = A + Vj;
               
               //Calculated Value = Value from Register
               CalculatedValue = Vk;
               
               break;
           }
           
           case 3: //BEQ
           {               
               //Value
                    CalculatedValue = (Vj == Vk);
               
                break;
           }
               
           
           case 4: //JALR
           {
               CalculatedValue = A + 1;

               break;
           }
               
           case 5: //RET
           {
               CalculatedValue = Vj;
               break;
           }
               
           case 6: //ADD rd, rs1, rs2
           {



               CalculatedValue = Vj + Vk;
               break;
           }
               
           case 7: //NEG rd, rs1
           {
            

               CalculatedValue = ~Vj;
               break;
               
           }
               
           case 8: //ADDI rd, rs1, imm
           {
               CalculatedValue = Vj + Vk;
               break;
           }
               
           case 9: //DIV rd, rs1, rs2
           {
               
               CalculatedValue = (Vk)? (Vj  / Vk):(0);
               break;
           }
               
       }
}
void ReservationStation::end_exe     (int Cycle)
{
    currentInstruction->endCycle = Cycle;
}
void ReservationStation::wb          (int Cycle)
{
    
    if (Op == 2 || Op == 3 || Op == 5)//If you are function that does not need the common bus (Store and Branch)
    {
        currentInstruction->writeCycle = Cycle;
        cout<<setw(15)<<currentInstruction->Inst<<setw(15)<<currentInstruction->issueCycle<<setw(15)<<currentInstruction->startCycle<<setw(15)<<currentInstruction->endCycle<<setw(15)<<currentInstruction->writeCycle<<endl;
        switch(Op)
        {
                
            case 2: //SW rs2, imm(rs1)
            {
                DataMemory->at(A) = CalculatedValue;
                clearAll();
                break;
            }
            
            case 3: //BEQ
            {
                *totalBranches = *totalBranches + 1;
                if(CalculatedValue)
                {
                    *wrongBranches= *wrongBranches + 1;
                    *PC = A;
                    flushQueue();
                }
                clearAll();
                *exeStall = false;
                break;
            }
                
            case 5: //RET
        {                
                //Set PC to be Equal to R1
                *PC = Vj;

                //Stop stalling the issuing
                clearAll();
                *issueStall = false;

                break;
            }
                   
        }
        
        
    }
    else if (!(*DB_Busy))
    {
        currentInstruction->writeCycle = Cycle;
        cout<<setw(15)<<currentInstruction->Inst<<setw(15)<<currentInstruction->issueCycle<<setw(15)<<currentInstruction->startCycle<<setw(15)<<currentInstruction->endCycle<<setw(15)<<currentInstruction->writeCycle<<endl;
        
        *DB_Busy = true;
        
        switch(Op)
        {
            case 1: //LW rd, imm(rs1)
            {
                *DB_Data = CalculatedValue;
                *DB_FUID = ID;
                break;
            }
                
            case 2: //SW rs2, imm(rs1)
            {
                *DB_FUID = ID;
                DataMemory->at(A) = CalculatedValue;
                break;
            }
            
            case 3: //BEQ
            {
                *DB_FUID = ID;

                *totalBranches++;
                if(CalculatedValue)
                {
                    *wrongBranches++;
                    *PC = A;
                    flushQueue();
                }
                *exeStall = false;
                break;
            }
                
            
            case 4: //JALR
            {
                //Store PC + 1 in R1
                *DB_Data = CalculatedValue;
                *DB_FUID = ID;
                
                //Set PC to be Equal to Register
                *PC = Vj;

                //Stop stalling the issuing
                *issueStall = false;
                break;
            }
                
            case 5: //RET
            {
                *DB_FUID = ID;
                
                //Set PC to be Equal to R1
                *PC = Vj;

                //Stop stalling the issuing
                *issueStall = false;

                break;
            }
                
            case 6: //ADD rd, rs1, rs2
            {

                *DB_Data = CalculatedValue;
                *DB_FUID = ID;
                break;
            }
                
            case 7: //NEG rd, rs1
            {
                *DB_Data = CalculatedValue;
                *DB_FUID = ID;
                break;
                
            }
                
            case 8: //ADDI rd, rs1, imm
            {
                *DB_Data = CalculatedValue;
                *DB_FUID = ID;
                break;
            }
                
            case 9: //DIV rd, rs1, rs2
            {
                *DB_Data = CalculatedValue;
                *DB_FUID = ID;
                break;
            }
                
        }
        
    }
    

}


//Utility Functions
bool ReservationStation::isMatch (int instType)
{
    return (instType >= SupportedTypes.first && instType <= SupportedTypes.second);
}

//Clear
void ReservationStation::clearStation()
{
    //Flags
     Busy = false;                          //Flag to show if reservation station is busy
     afterBranch = false;
    
    //
    //Run-time attributes
     remainingCycles = executionCycles;      //Number of execution cycles left
    
    //Classical Attributes
     CalculatedValue = 0;
     Op = 0;
     Vj = 0;
     Vk = 0;
     Qj = 0;
     Qk = 0;
     A  = 0;
}

void ReservationStation::clearQueue()
{
    //For all instructions, remove from the Instruction Queue
    for (int i = 0; i<InstructionQueue->size(); i++)
        if (InstructionQueue->at(i)->ID == ID)
            InstructionQueue->erase(InstructionQueue->begin()+i);
    
    //For all store-load instructions, remove from the StoreLoad Queue
    if (Op == 1 || Op == 2)
            LoadStoreQueue->erase(LoadStoreQueue->begin());

}


void ReservationStation::flushQueue()
{
    int BranchID = 0;
    
    //Find my (branch) instruction in the instruction queue
    for (int i = 0; i<InstructionQueue->size(); i++)
        if (InstructionQueue->at(i)->ID == ID)
            BranchID = i;
    
    while (InstructionQueue->size() != BranchID + 1)
    {
        InstructionQueue->at(BranchID + 1)->clearAll();
    }
}



//Inherited Classes
struct RS_LW: public ReservationStation
{
    RS_LW
    (
         int iID,
         int iexecutionCycles,
         float* PC,
         vector <int>* DataMemory,
         vector <int16_t>* RegisterFile,
         vector <int>*     RegisterStatus,
         vector <ReservationStation*>*     LoadStoreQueue,
         vector <ReservationStation*>*     InstructionQueue,
         bool* issueStall,
         bool* exeStall,
         float* totalBranches,
         float* wrongBranches,
         bool*    DB_Busy,
         int16_t*  DB_Data,
         int *    DB_FUID
    )
    {
        
        
        SupportedTypes.first = 1;
        SupportedTypes.second = 1;
        
        ID = iID;
        executionCycles = iexecutionCycles;
        
        this->DB_Busy = DB_Busy;
        this->DB_Data = DB_Data;
        this->DB_FUID = DB_FUID;
        this->PC = PC;
        this->DataMemory = DataMemory;
        this->RegisterFile = RegisterFile;
        this->RegisterStatus = RegisterStatus;
        this->LoadStoreQueue = LoadStoreQueue;
        this->InstructionQueue = InstructionQueue;
        this->issueStall = issueStall;
        this->exeStall = exeStall;
        this->totalBranches = totalBranches;
        this->wrongBranches = wrongBranches;
        
        clearStation();

    }
};

struct RS_SW: public ReservationStation
{
    RS_SW
   (
        int iID,
        int iexecutionCycles,
        float* PC,
        vector <int>* DataMemory,
        vector <int16_t>* RegisterFile,
        vector <int>*     RegisterStatus,
        vector <ReservationStation*>*     LoadStoreQueue,
        vector <ReservationStation*>*     InstructionQueue,
        bool* issueStall,
        bool* exeStall,
        float* totalBranches,
        float* wrongBranches,
        bool*    DB_Busy,
        int16_t*  DB_Data,
        int *    DB_FUID
   )
   {
        SupportedTypes.first = 2;
        SupportedTypes.second = 2;
       
       ID = iID;
       executionCycles = iexecutionCycles;
       
       this->DB_Busy = DB_Busy;
       this->DB_Data = DB_Data;
       this->DB_FUID = DB_FUID;
       this->PC = PC;
       this->DataMemory = DataMemory;
       this->RegisterFile = RegisterFile;
       this->RegisterStatus = RegisterStatus;
       this->LoadStoreQueue = LoadStoreQueue;
       this->InstructionQueue = InstructionQueue;
       this->issueStall = issueStall;
       this->exeStall = exeStall;
       this->totalBranches = totalBranches;
       this->wrongBranches = wrongBranches;
       
       clearStation();

    }
};


struct RS_BEQ: public ReservationStation
{
    RS_BEQ
    (
         int iID,
         int iexecutionCycles,
         float* PC,
         vector <int>* DataMemory,
         vector <int16_t>* RegisterFile,
         vector <int>*     RegisterStatus,
         vector <ReservationStation*>*     LoadStoreQueue,
         vector <ReservationStation*>*     InstructionQueue,
         bool* issueStall,
         bool* exeStall,
         float* totalBranches,
         float* wrongBranches,
         bool*    DB_Busy,
         int16_t*  DB_Data,
         int *    DB_FUID
    )
    {
        SupportedTypes.first = 3;
        SupportedTypes.second =3;
        ID = iID;
        executionCycles = iexecutionCycles;
        
        this->DB_Busy = DB_Busy;
        this->DB_Data = DB_Data;
        this->DB_FUID = DB_FUID;
        this->PC = PC;
        this->DataMemory = DataMemory;
        this->RegisterFile = RegisterFile;
        this->RegisterStatus = RegisterStatus;
        this->LoadStoreQueue = LoadStoreQueue;
        this->InstructionQueue = InstructionQueue;
        this->issueStall = issueStall;
        this->exeStall = exeStall;
        this->totalBranches = totalBranches;
        this->wrongBranches = wrongBranches;
        
        clearStation();

    }
};


struct RS_JAL: public ReservationStation
{
    RS_JAL
    (
         int iID,
         int iexecutionCycles,
         float* PC,
         vector <int>* DataMemory,
         vector <int16_t>* RegisterFile,
         vector <int>*     RegisterStatus,
         vector <ReservationStation*>*     LoadStoreQueue,
         vector <ReservationStation*>*     InstructionQueue,
         bool* issueStall,
         bool* exeStall,
         float* totalBranches,
         float* wrongBranches,
         bool*    DB_Busy,
         int16_t*  DB_Data,
         int *    DB_FUID
    )
    {
        SupportedTypes.first = 4;
        SupportedTypes.second = 5;
        ID = iID;
        executionCycles = iexecutionCycles;
        
        this->DB_Busy = DB_Busy;
        this->DB_Data = DB_Data;
        this->DB_FUID = DB_FUID;
        this->PC = PC;
        this->DataMemory = DataMemory;
        this->RegisterFile = RegisterFile;
        this->RegisterStatus = RegisterStatus;
        this->LoadStoreQueue = LoadStoreQueue;
        this->InstructionQueue = InstructionQueue;
        this->issueStall = issueStall;
        this->exeStall = exeStall;
        this->totalBranches = totalBranches;
        this->wrongBranches = wrongBranches;
        
        clearStation();

    }
};

struct RS_ADD: public ReservationStation
{
    RS_ADD
    (
         int iID,
         int iexecutionCycles,
         float* PC,
         vector <int>* DataMemory,
         vector <int16_t>* RegisterFile,
         vector <int>*     RegisterStatus,
         vector <ReservationStation*>*     LoadStoreQueue,
         vector <ReservationStation*>*     InstructionQueue,
         bool* issueStall,
         bool* exeStall,
         float* totalBranches,
         float* wrongBranches,
         bool*    DB_Busy,
         int16_t*  DB_Data,
         int *    DB_FUID
    )
    {
        SupportedTypes.first = 6;
        SupportedTypes.second = 8;
        ID = iID;
        executionCycles = iexecutionCycles;
        
        this->DB_Busy = DB_Busy;
        this->DB_Data = DB_Data;
        this->DB_FUID = DB_FUID;
        this->PC = PC;
        this->DataMemory = DataMemory;
        this->RegisterFile = RegisterFile;
        this->RegisterStatus = RegisterStatus;
        this->LoadStoreQueue = LoadStoreQueue;
        this->InstructionQueue = InstructionQueue;
        this->issueStall = issueStall;
        this->exeStall = exeStall;
        this->totalBranches = totalBranches;
        this->wrongBranches = wrongBranches;
        
        clearStation();

    }
};

struct RS_DIV: public ReservationStation
{
    RS_DIV
    (
         int iID,
         int iexecutionCycles,
         float* PC,
         vector <int>* DataMemory,
         vector <int16_t>* RegisterFile,
         vector <int>*     RegisterStatus,
         vector <ReservationStation*>*     LoadStoreQueue,
         vector <ReservationStation*>*     InstructionQueue,
         bool* issueStall,
         bool* exeStall,
         float* totalBranches,
         float* wrongBranches,
         bool*    DB_Busy,
         int16_t*  DB_Data,
         int *    DB_FUID
    )
    {
        SupportedTypes.first = 9;
        SupportedTypes.second = 9;
        ID = iID;
        executionCycles = iexecutionCycles;
        
        this->DB_Busy = DB_Busy;
        this->DB_Data = DB_Data;
        this->DB_FUID = DB_FUID;
        this->PC = PC;
        this->DataMemory = DataMemory;
        this->RegisterFile = RegisterFile;
        this->RegisterStatus = RegisterStatus;
        this->LoadStoreQueue = LoadStoreQueue;
        this->InstructionQueue = InstructionQueue;
        this->issueStall = issueStall;
        this->exeStall = exeStall;
        this->totalBranches = totalBranches;
        this->wrongBranches = wrongBranches;
        
        clearStation();
    }
};

ReservationStation::ReservationStation()
{
}
#endif /* ReservationStation_h */
