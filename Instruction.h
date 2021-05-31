#ifndef Instruction_h
#define Instruction_h
#include <string>
using namespace std;

struct Instruction
{
    int Type;
    
    string Inst;

    int issueCycle;         //Cycle when instruction was issued
    int startCycle;         //Cycle when instruction started execution
    int endCycle;           //Cycle when instruction ended execution
    int writeCycle;         //Cycle when instruction finished writing
    
    int rd;
    int rs1;
    int imm;
    int rs2;

    
    Instruction()
    {
        issueCycle = -1;
        startCycle = -1;
        endCycle   = -1;
        writeCycle = -1;

    };
};


struct LW: public Instruction
{

    LW (string i_Inst, int i_rd, int i_rs1, int i_imm)
    {
        Inst = i_Inst;
        Type = 1;
        rd = i_rd;
        rs1 = i_rs1;
        imm = i_imm;
    }
};

struct SW: public Instruction
{

    SW (string i_Inst, int i_rs1, int i_rs2, int i_imm)
    {
        Inst = i_Inst;
        Type = 2;
        rs1 = i_rs1;
        rs2 = i_rs2;
        imm = i_imm;
    }
};

struct BEQ: public Instruction
{

    BEQ (string i_Inst, int i_rs1, int i_rs2, int i_imm)
    {
        Inst = i_Inst;
        Type = 3;
        rs1 = i_rs1;
        rs2 = i_rs2;
        imm = i_imm;
    }
};

struct JALR: public Instruction
{
    JALR (string i_Inst, int i_rs1)
    {
        Inst = i_Inst;
        Type = 4;
        rs1 = i_rs1;
    }
};

struct RET: public Instruction
{
    RET (string i_Inst)
    {
        Inst = i_Inst;
        Type = 5;
    }
};

struct ADD: public Instruction
{

    ADD (string i_Inst, int i_rd, int i_rs1, int i_rs2)
    {
        Inst = i_Inst;
        Type = 6;
        rs1 = i_rs1;
        rs2 = i_rs2;
        rd = i_rd;
    }
};

struct NEG: public Instruction
{

    NEG (string i_Inst, int i_rd, int i_rs1)
    {
        Inst = i_Inst;
        Type = 7;
        rd = i_rd;
        rs1 = i_rs1;
    }
};

struct ADDI: public Instruction
{

    ADDI (string i_Inst, int i_rd, int i_rs1, int i_imm)
    {
        Inst = i_Inst;
        Type = 8;
        rd = i_rd;
        rs1 = i_rs1;
        imm = i_imm;
    }
};


struct DIV: public Instruction
{

    DIV (string i_Inst, int i_rd, int i_rs1, int i_rs2)
    {
        Inst = i_Inst;
        Type = 9;
        rd = i_rd;
        rs1 = i_rs1;
        rs2 = i_rs2;
    }
};



#endif /* Instruction_h */
