#ifndef Simulator_h
#define Simulator_h

#include "Instruction.h"
#include "DataBus.h"
#include "ReservationStation.h"


#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

using namespace std;


struct Simulator
{
    //Run-time data
    float PC;
    float Cycle;
    
    //Memories
    vector <Instruction*>            InstructionMemory;
    vector <int>                     DataMemory;

    //Register File
    vector <int16_t>                 RegisterFile;
    
    //Register Status
    vector <int>                      RegisterStatus;
    
    //Reservation Stations
    vector <ReservationStation*>     ReservationStations;
    
    //Load-Store Queue
    vector <ReservationStation*>     LoadStoreQueue;

    //Instruction Queue
    vector <ReservationStation*>     InstructionQueue;
    
    //Data Bus
    DataBus* DataBus;

    //Data Bus Ports
    bool    DB_Busy;
    int16_t DB_Data;
    int     DB_FUID;
    
    //Flags
    bool issueStall;
    bool exeStall;
    
    
    //Branch Statistics
    float totalBranches;
    float wrongBranches;
    
    //File Streams
    fstream         InstructionFileStream;
    stringstream    InstructionStream;
    
    //Run Time Function
    void Run();
    
    //Starting Function
    void startVar();
    void startParse();
    void startOrg();
    
    //utility functions
    void debug();
    void issue ();
    void play();

    //Constructor & Destructor
    Simulator();
    ~Simulator();
};

void Simulator:: play()
{
    
    
    for (auto i = 0; i<InstructionQueue.size(); i++)
        InstructionQueue[i]->play(Cycle);
}

void Simulator::debug()
{
    cout<<"-----------------------"<<endl;

    cout<<"Debugging at Cycle"<<Cycle<<endl;
    
    cout<<endl<<"Register File"<<endl;
    cout<<"Register"<<setw(15)<<"Value"<<endl;
    for (int i = 0; i<7; i++)
        cout<<i<<setw(15)<<RegisterFile[i]<<endl;
    
    cout<<endl<<"Register Status"<<endl;
    cout<<"Register"<<setw(15)<<"Qi"<<endl;
    for (int i = 0; i<7; i++)
        cout<<i<<setw(15)<<RegisterStatus[i]<<endl;
    
    cout<<endl<<"Reservation Stations"<<endl;
    cout<<"ID"<<setw(15)<<"Calc."<<setw(15)<<"Op"<<setw(15)<<"Vj"<<setw(15)<<"Vk"<<setw(15)<<"Qj"<<setw(15)<<"Qk"<<setw(15)<<"A"<<endl;
    for (int i = 0; i<ReservationStations.size(); i++)
        cout<<setw(15)<<ReservationStations[i]->ID<<setw(15)<<ReservationStations[i]->CalculatedValue<<setw(15)<<ReservationStations[i]->Op<<setw(15)<<ReservationStations[i]->Vj<<setw(15)<<ReservationStations[i]->Vk<<setw(15)<<ReservationStations[i]->Qj<<setw(15)<<ReservationStations[i]->Qk<<setw(15)<<ReservationStations[i]->A<<endl;

        
    
    cout<<"-----------------------"<<endl;
}

void Simulator:: issue()
{
    if (issueStall) return;

    for (int i = 0; i<ReservationStations.size(); i++)                                                                                                          //Check all data buses
    {
        if (ReservationStations[i]->isMatch(InstructionMemory[PC]->Type) && !ReservationStations[i]->Busy)                                                      //Check if said data bus is free and of the same type
        {
            //Issue this
                ReservationStations[i]->issue(InstructionMemory[PC], Cycle);

            //Place it in the Instruction Queue
                InstructionQueue.push_back (ReservationStations[i]);
            
            //Check if it is a store or a load. If it is, place it inside
                if (InstructionMemory[PC]->Type == 1 || InstructionMemory[PC]->Type == 2) LoadStoreQueue.push_back (ReservationStations[i]);

            //Increment PC to next instruction
                PC++;
                
            //Exit loop
                return;
        }
    }
}


void Simulator:: Run()
{

   while (PC < InstructionMemory.size() || !InstructionQueue.empty())
   {
       

       
       //Play the next action in the instructions
            play ();
       
       //Issue the next instruction
            if (PC < InstructionMemory.size()) issue ();
       
       //Debug
        //debug();
       //Handle the Data Bus
            DataBus->reflect();
            DataBus->clear();

       //Increment to next cycle
            Cycle++;


   }
    
}



Simulator:: Simulator()
{
    //Initalize the Instruction Memory
    InstructionFileStream.open("Inst.txt");
    if (InstructionFileStream.fail())
    {
        cout<<"Failed to load instruction file"<<endl<<"Program Terminated."<<endl;
        return;
    }
    
    //Run-time data
    PC = 0;
    Cycle = 0;

    //Initalize the Instruction Memory
    startParse();

    //Initialize the Hardware Organization
    startOrg();
    
    //Initailize the rest of the variables
    startVar();
    
    //Instructions Table
    cout<<endl<<"Instructions Executed (sorted according to WB cycle)"<<endl;
    cout<<setw(15)<<"Instruction"<<setw(15)<<"Issued"<<setw(15)<<"Start. Exe"<<setw(15)<<"End. Exe"<<setw(15)<<"Wrote back"<<endl;
    
    //Run Simulation;
    Run();
    
}

Simulator:: ~Simulator()
{
    cout<<endl<<"Register File"<<endl;
    for (int i = 0; i<8; i++)
        cout<<i<<setw(15)<<RegisterFile[i]<<endl;
        
    //Number of cycles
    cout<<endl<<"Total Number of Cycles: "<<Cycle<<endl;
    
    //IPC
    cout<<endl<<"IPC: "<<float(InstructionMemory.size()/Cycle)<<endl;

    //Branch Misprediction Percentage
    cout<<endl<<"Branch Misprediction Percentage: "<<float(wrongBranches/totalBranches)*100<<"%"<<endl;
}


void Simulator:: startParse()
{
    string Instruction, InstructionType, InstructionOperands [3];
    while (InstructionFileStream.good())
    {
        //Get whole line
        getline (InstructionFileStream, Instruction);


        //Create stream from the instruction line
        InstructionStream.clear();
        InstructionStream.str(Instruction);

        //Get instruction type in the line
        getline (InstructionStream, InstructionType, ' ');
        

        if (InstructionType == "LW") //LW rd, imm(rs1)
        {
            getline (InstructionStream, InstructionOperands[0], ','); //rd
            getline (InstructionStream, InstructionOperands[1], '('); //imm
            getline (InstructionStream, InstructionOperands[2], ')'); //rs1
            
            InstructionOperands[0].erase(0,1);                         //Normalize rd
            InstructionOperands[2].erase(0,1);                         //Normalize rs1

            
            InstructionMemory.push_back(new LW (Instruction, stoi(InstructionOperands[0]), stoi(InstructionOperands[2]), stoi(InstructionOperands[1])));
        }
        else if (InstructionType == "SW") //SW rs2, imm(rs1)
        {
            getline (InstructionStream, InstructionOperands[0], ','); //rs2
            getline (InstructionStream, InstructionOperands[1], '('); //imm
            getline (InstructionStream, InstructionOperands[2], ')'); //rs1
            
            InstructionOperands[0].erase(0,1);                         //Normalize rs2
            InstructionOperands[2].erase(0,1);                         //Normalize rs1

            
            InstructionMemory.push_back(new SW (Instruction, stoi(InstructionOperands[2]), stoi(InstructionOperands[0]), stoi(InstructionOperands[1])));
            
        }
        else if (InstructionType == "BEQ") //BEQ rs1, rs2, imm
        {
            getline (InstructionStream, InstructionOperands[0], ','); //rs1
            getline (InstructionStream, InstructionOperands[1], ','); //rs2
            getline (InstructionStream, InstructionOperands[2], '\n'); //imm
            
            InstructionOperands[0].erase(0,1);                         //Normalize rd
            InstructionOperands[1].erase(0,1);                         //Normalize rs1

            InstructionMemory.push_back(new BEQ (Instruction, stoi(InstructionOperands[0]), stoi(InstructionOperands[1]), stoi(InstructionOperands[2])));
            
        }
        else if (InstructionType == "JALR") //JALR rs1
        {
            getline (InstructionStream, InstructionOperands[0], '\n'); //rs1

            InstructionOperands[0].erase(0,1);                         //Normalize rs1

            InstructionMemory.push_back(new JALR (Instruction, stoi(InstructionOperands[0])));

        }
        else if (InstructionType == "RET") //
        {
            InstructionMemory.push_back(new RET(Instruction));
        }
        else if (InstructionType == "ADD") //ADD rd, rs1, rs2
        {
                
            getline (InstructionStream, InstructionOperands[0], ','); //rd
            getline (InstructionStream, InstructionOperands[1], ','); //rs1
            getline (InstructionStream, InstructionOperands[2], '\n'); //rs2
            
            InstructionOperands[0].erase(0,1);                         //Normalize rd
            InstructionOperands[1].erase(0,1);                         //Normalize rs1
            InstructionOperands[2].erase(0,1);                         //Normalize rs2


            
            InstructionMemory.push_back(new ADD (Instruction, stoi(InstructionOperands[0]), stoi(InstructionOperands[1]), stoi(InstructionOperands[2])));
                
        }
        else if (InstructionType == "NEG") //NEG rd, rs1
        {
            getline (InstructionStream, InstructionOperands[0], ','); //rd
            getline (InstructionStream, InstructionOperands[1], '\n'); //rs1
            
            InstructionOperands[0].erase(0,1);                         //Normalize rd
            InstructionOperands[1].erase(0,1);                         //Normalize rs1

            InstructionMemory.push_back(new NEG (Instruction, stoi(InstructionOperands[0]), stoi(InstructionOperands[1])));
        }
        
        else if (InstructionType == "ADDI") //ADDI rd, rs1, imm
        {
            getline (InstructionStream, InstructionOperands[0], ','); //rd
            getline (InstructionStream, InstructionOperands[1], ','); //rs1
            getline (InstructionStream, InstructionOperands[2], '\n'); //imm
            
            InstructionOperands[0].erase(0,1);                         //Normalize rd
            InstructionOperands[1].erase(0,1);                         //Normalize rs1
            

            InstructionMemory.push_back(new ADDI (Instruction, stoi(InstructionOperands[0]), stoi(InstructionOperands[1]), stoi(InstructionOperands[2])));

        }
        else if (InstructionType == "DIV") //DIV rd, rs1, rs2
        {
            getline (InstructionStream, InstructionOperands[0], ','); //rd
            getline (InstructionStream, InstructionOperands[1], ','); //rs1
            getline (InstructionStream, InstructionOperands[2], '\n'); //rs2
            
            InstructionOperands[0].erase(0,1);                         //Normalize rd
            InstructionOperands[1].erase(0,1);                         //Normalize rs1
            InstructionOperands[2].erase(0,1);                         //Normalize rs2

            
            InstructionMemory.push_back(new DIV (Instruction, stoi(InstructionOperands[0]), stoi(InstructionOperands[1]), stoi(InstructionOperands[2])));
        }

    }
    
}

void Simulator:: startOrg()
{
    int number;
    int cycles;
    int RSID = 0;
    
    cout<<"----------Hardware Organization Initialization--------"<<endl;
    cout<<"How many LW reservation stations do you want: ";
    cin>> number;
    cout<<"How many cycles do you want the LW FU to run in: ";
    cin>>cycles;
    while (number>0 && number--)
        ReservationStations.push_back (new RS_LW(++RSID, cycles, &PC, &DataMemory, &RegisterFile, &RegisterStatus, &LoadStoreQueue, &InstructionQueue, &issueStall, &exeStall, &totalBranches, &wrongBranches, &DB_Busy, &DB_Data, &DB_FUID));

    cout<<"How many SW reservation stations do you want: ";
    cin>> number;
    cout<<"How many cycles do you want the SW FU to run in: ";
    cin>>cycles;
    while (number>0 && number--)
        ReservationStations.push_back (new RS_SW(++RSID, cycles, &PC, &DataMemory, &RegisterFile, &RegisterStatus, &LoadStoreQueue, &InstructionQueue, &issueStall, &exeStall, &totalBranches, &wrongBranches, &DB_Busy, &DB_Data, &DB_FUID));

    cout<<"How many BEQ reservation stations do you want: ";
    cin>> number;
    cout<<"How many cycles do you want the BEQ FU to run in: ";
    cin>>cycles;
    while (number>0 && number--)
        ReservationStations.push_back (new RS_BEQ(++RSID, cycles, &PC, &DataMemory, &RegisterFile, &RegisterStatus, &LoadStoreQueue, &InstructionQueue, &issueStall, &exeStall, &totalBranches, &wrongBranches, &DB_Busy, &DB_Data, &DB_FUID));

    
    cout<<"How many JAL/RET reservation stations do you want: ";
    cin>> number;
    cout<<"How many cycles do you want the JAL/RET FU to run in: ";
    cin>>cycles;
    while (number>0 && number--)
        ReservationStations.push_back (new RS_JAL(++RSID, cycles, &PC, &DataMemory, &RegisterFile, &RegisterStatus, &LoadStoreQueue, &InstructionQueue, &issueStall, &exeStall, &totalBranches, &wrongBranches, &DB_Busy, &DB_Data, &DB_FUID));

    
    cout<<"How many ADD/NEG/ADDI reservation stations do you want: ";
    cin>> number;
    cout<<"How many cycles do you want the ADD/NEG/ADDI FU to run in: ";
    cin>>cycles;
    while (number>0 && number--)
        ReservationStations.push_back (new RS_ADD(++RSID, cycles, &PC, &DataMemory, &RegisterFile, &RegisterStatus, &LoadStoreQueue, &InstructionQueue, &issueStall, &exeStall, &totalBranches, &wrongBranches, &DB_Busy, &DB_Data, &DB_FUID));

    
    cout<<"How many DIV reservation stations do you want: ";
    cin>> number;
    cout<<"How many cycles do you want the DIV FU to run in: ";
    cin>>cycles;
    while (number>0 && number--)
        ReservationStations.push_back (new RS_DIV(++RSID, cycles, &PC, &DataMemory, &RegisterFile, &RegisterStatus, &LoadStoreQueue, &InstructionQueue, &issueStall, &exeStall, &totalBranches, &wrongBranches, &DB_Busy, &DB_Data, &DB_FUID));

    cout<<"------------------------------------------------"<<endl;

}

void Simulator:: startVar()
{

    
    DataBus = new struct DataBus(&DB_Busy, &DB_Data, &DB_FUID, &ReservationStations, &RegisterFile, &RegisterStatus);
    DataMemory.resize(1024);
    
    DataMemory[10] = 5;

    //Register File
    RegisterFile.resize(8);
    
    //Register Status
    RegisterStatus.resize(8);

    //Flags
    issueStall = false;
    exeStall = false;
    
    //Branch Statistics
    totalBranches = 0;
    wrongBranches = 0;
    
    
}

#endif
