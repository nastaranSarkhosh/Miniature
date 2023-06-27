#include <stdbool.h>
#include "simulator.h"
int registers[16] = { 3,1,1,3,2,2,3,3,2,3,2,3 ,2,3 ,2,3 };
int memory[16001] = { 0 ,2,2,2,2,2,2,2,2,2,2};
int instruction[10000]={0};
int regUsage[16] = { 0 };
int totalusage = 0,memoryUsege = 0;
//                                   Expansion  MemRead   MemWrite MemToReg ALUSrc  RegDst  RegWrite    PC    ALUControl
const char* Signals[15][9] = { {"0", "0", "0", "00", "0", "1", "1", "00", "0010"},
                                {"0", "0", "0", "00", "0", "1", "1", "00", "0110"},
                                {"0", "0", "0", "00", "0", "1", "1", "00", "0111"},
                                {"0", "0", "0", "00", "0", "1", "1", "00", "0001"},
                                {"0", "0", "0", "00", "0", "1", "1", "00", "0000"},
                                {"1", "0", "0", "00", "1", "0", "1", "00", "0010"},
                                {"1", "0", "0", "00", "1", "0", "1", "00", "0111"},
                                {"0", "0", "0", "00", "1", "0", "1", "00", "0001"},
                                {"0", "0", "0", "10", "0", "0", "1", "00", "0000"},
                                {"1", "1", "0", "01", "1", "0", "1", "00", "0010"},
                                {"1", "0", "1", "00", "1", "0", "0", "00", "0010"},
                                {"1", "0", "0", "00", "0", "0", "0", "01", "0110"},
                                {"0", "0", "0", "11", "0", "0", "1", "10", "0000"},
                                {"0", "0", "0", "00", "0", "0", "0", "11", "0000"},
                                {"0", "0", "0", "00", "0", "0", "0", "00", "0000"} };


void main(int argc, char** argv)
{
    FILE* machp = fopen(argv[1], "r");
    FILE* logFile = fopen(argv[2], "w+");
    struct instruction* currInst;
    currInst = (struct instruction*)malloc(sizeof(struct instruction));
    int  instCount = 0, pc = 0;
    loader(machp);
    if (argc < 3)
    {
        printf("***** Please run this program as follows:\n");
        printf("***** %s assprog.as machprog.m\n", argv[0]);
        printf("***** where machprog.m is your machine code\n");
        printf("***** and log.txt will be your log file.\n");
        exit(1);
    }
    if (!machp)
    {
        printf("%s cannot be openned\n", argv[1]);
        exit(1);
    }
    if (!logFile)
    {
        printf("%s cannot be openned\n", argv[2]);
        exit(1);
    }
    bool halt=false;
    while (!halt)
    {
        currInst->pc = pc;
        instCount++;
        strcpy(currInst->inst, int2bin(instruction[pc], 32));
        currInst->opcode = bin2Int(&currInst->inst[4], 4);
        if (currInst->opcode == 14)
        {
            halt = true;
        }
        if (halt)
        {
            fprintf(logFile, "-----------------------------------\n");
            for (int i = 0; i < 16; i++){
                int percent=(100 * regUsage[i] / totalusage);
                fprintf(logFile, "R%d usage : %%%d \n", i,percent);
            }
            fprintf(logFile, "number of instructions: %d lines\n", instCount);
            fprintf(logFile, "Memory : %d\n", memoryUsege);
            exit(0);
        }
        pc = singleCycle(currInst);
        fprintf(logFile, "(PC: %d,Inst:%s)\n", pc, currInst->inst);
        for (int i = 0; i < 16; i++)
            fprintf(logFile, "$%d= %d\n", i, registers[i]);
    }
}
void loader(FILE* m)
{
    int line, i = 0;
    while (fscanf(m, "%d", &line) != EOF)
    {
        memory[i] = line;
        instruction[i]=memory[i];
        i++;
        memoryUsege++;
    }
    rewind(m);
}
int bin2Int(char* opcode, int numBit)
{
    int result = 0, r = 0;
    for (int i = numBit - 1; i >= 0; i--)
    {
        result += (opcode[i] - '0') * pow(2, r);
        r++;
    }
    if (opcode[0] == '1' && numBit == 32)
        return pow(2, numBit) - result;
    return result;
}
void MuxConstruct(char* f, char* s, int b,struct Mux* mux)
{
    mux->first = (char*)malloc(b + 1);
    mux->second = (char*)malloc(b + 1);
    memcpy(mux->first, &f[0], b);
    mux->first[b] = '\0';
    memcpy(mux->second, &s[0], b);
    mux->second[b] = '\0';
}
void Mux4Construct(char* fi, char* s,char* t, char* fo, int bit,struct Mux4* mux)
{
    mux->first = (char*)malloc(bit + 1);
    mux->second = (char*)malloc(bit + 1);
    mux->third = (char*)malloc(bit + 1);
    mux->fourth = (char*)malloc(bit + 1);
    memcpy(mux->first, &fi[0], bit);
    mux->first[bit] = '\0';
    memcpy(mux->second, &s[0], bit);
    mux->second[bit] = '\0';
    memcpy(mux->third, &t[0], bit);
    mux->third[bit] = '\0';
    memcpy(mux->fourth, &fo[0], bit);
    mux->fourth[bit] = '\0';
}
char* OutPutMux4(const char* signal, struct Mux4* mux4)
{
    if (!strcmp(signal, "00"))
        return mux4->first;
    else if(!strcmp(signal, "01"))
        return mux4->second;
    else if(!strcmp(signal, "10"))
        return mux4->third;
    else
        return mux4->fourth;
}
char* pcOutPutMux4(const char* signal, struct Mux4* mux4)
{
    if (!strcmp(signal, "00"))
        return mux4->first;
    else if(!strcmp(signal, "01"))
        return mux4->second;
    else if(!strcmp(signal, "10"))
        return mux4->third;
    else
        return mux4->fourth;
}
char* OutPutMux(const char* signal, struct Mux* mux)
{
    if (!strcmp(signal, "0"))
        return mux->first;
    else return mux->second;
}
void initSignals(int opcode, struct ControlUnit* controlUnit)
{
    controlUnit->Expansion = Signals[opcode][0];
    controlUnit->MemRead = Signals[opcode][1];
    controlUnit->MemWrite = Signals[opcode][2];
    controlUnit->MemToReg = Signals[opcode][3];
    controlUnit->ALUSrc = Signals[opcode][4];
    controlUnit->RegDst = Signals[opcode][5];
    controlUnit->RegWrite = Signals[opcode][6];
    controlUnit->Pc = Signals[opcode][7];
    controlUnit->ALUControl = Signals[opcode][8];
}
int Result(struct ALU* alu)
{
    if (!strcmp(alu->signal, "0000"))
        return (alu->source1) & (alu->source2);
    if (!strcmp(alu->signal, "0001"))
        return (alu->source1) | (alu->source2);
    if (!strcmp(alu->signal, "0010"))
        return (alu->source1) + (alu->source2);
    if (!strcmp(alu->signal, "0110"))
        return (alu->source1) - (alu->source2);
    if (!strcmp(alu->signal, "0111"))
        return (alu->source1 < alu->source2);
    return -1;
}
const char* Zero(int result )
{
    if (result == 0)
        return "1";
    return "0";
}
int ReadReg(struct RegisterFile* t,int reg)
{
    if(reg==1){
        totalusage++;
        regUsage[t->rs]++;
        return (registers[t->rs]);
    }else{
        totalusage++;
        regUsage[t->rt]++;
        return (registers[t->rt]);
    }
}

void regWrite(int d, struct RegisterFile* t)
{
    if (!strcmp(t->regWrite, "1"))
    {
        registers[t->rd] = d;
        totalusage++;
        regUsage[t->rd]++;

    }
}
int memRead(struct MEM* mem)
{
    if (!strcmp(mem->memRead, "1"))
        if(mem->address >= 0 && mem->address <= 16000)
            return memory[mem->address];
}
void memWrite(int resultAlu, struct MEM* mem,int ReadRt)
{
    if (!strcmp(mem->memWrite, "1"))
    {
        mem->address=resultAlu;
        memory[mem->address] =ReadRt;
        memoryUsege++;
    }
}
int singleCycle(struct instruction* inst)
{
    struct ControlUnit* controlUnit = (struct ControlUnit*)malloc(sizeof(struct ControlUnit));
    initSignals(inst->opcode, controlUnit);
    int pc=  ID(controlUnit, inst);
    return pc;

}
char* int2bin(int n, int numBit)
{
    if (n < 0)
        n += pow(2, numBit);
    char* result = (char*)malloc(numBit + 1);
    for (int i = 0; i < numBit; i++)
        result[i] = '0';
    result[numBit] = '\0';
    int j = numBit - 1;
    while (n > 0)
    {
        result[j] = (n % 2) + '0';
        n /= 2;
        j--;
    }
    return result;
}
void Expansion(char * imm,struct instruction* inst,char* signal){
    for (int i = 0; i < 16; i++)
    {
        if (inst->inst[16] != '0' && !strcmp(signal, "1"))
            imm[i] = '1';
        else imm[i] = '0';
    }
    for (int i = 16; i < 33; i++)
        imm[i] = inst->inst[i];
}
int ID(struct ControlUnit* controlUnit,struct instruction* inst){
    struct Mux* RegDstMux= (struct Mux*)malloc(sizeof(struct Mux));
    MuxConstruct(&(inst->inst[12]), &(inst->inst[16]), 4,RegDstMux);
    struct RegisterFile* registerFile = (struct RegisterFile*)malloc(sizeof(struct RegisterFile));
    registerFile->rs = bin2Int(&(inst->inst[8]), 4);
    registerFile->rt = bin2Int(&(inst->inst[12]), 4);
    registerFile->rd = bin2Int(OutPutMux(controlUnit->RegDst, RegDstMux), 4);
    strcpy(registerFile->regWrite, controlUnit->RegWrite);
    int readRs=ReadReg(registerFile,1);
    int readRt=ReadReg(registerFile,2);
    return Exe(controlUnit,inst,readRs ,readRt,registerFile);
}
int Exe(struct ControlUnit* controlUnit,struct instruction* inst,int readRs,int readRt,struct RegisterFile* registerFile){
    char imm[33];
    Expansion(imm,inst,controlUnit->Expansion);
    struct Mux* AluSrcMux = (struct Mux*)malloc(sizeof(struct Mux));
    MuxConstruct(int2bin(readRt, 32), imm, 32,AluSrcMux);
    struct ALU* alu = (struct ALU*)malloc(sizeof(struct ALU));
    strcpy(alu->signal, controlUnit->ALUControl);
    alu->source1 =readRs;
    alu->source2 = bin2Int(OutPutMux(controlUnit->ALUSrc, AluSrcMux), 32);
    int resultAlu=Result(alu);
    char*zero= Zero(resultAlu);
    return Mem(controlUnit,inst,resultAlu,registerFile,imm,readRs,zero,readRt);
}
int Mem(struct ControlUnit* controlUnit,struct instruction* inst,int resultAlu,struct RegisterFile* registerFile,char* imm,int readRs,char*zero,int readRt){
    struct MEM* mem = (struct MEM*)malloc(sizeof(struct MEM));
    strcpy(mem->memRead, controlUnit->MemRead);
    strcpy(mem->memWrite, controlUnit->MemWrite);
    memWrite(resultAlu, mem,readRt);
    int resultMem=memRead(mem);
    return WB(controlUnit,inst,resultAlu,resultMem,registerFile,imm,readRs,zero);
}
int WB(struct ControlUnit* controlUnit,struct instruction* inst,int resultAlu,int resultMem,struct RegisterFile* registerFile,char* imm,int readRs,char*zero){
    char lui[33];
    for (int i = 0; i <16; i++)
        lui[i] = inst->inst[i+16];
    for (int i = 16; i < 33; i++)
        lui[i] = '0';
    struct Mux4* Mem2RegMux = (struct Mux4*)malloc(sizeof(struct Mux4));
    Mux4Construct(int2bin(resultAlu,32), int2bin(resultMem, 32),lui,int2bin((inst->pc + 1),32),32,Mem2RegMux);
    int outMux=bin2Int(OutPutMux4(controlUnit->MemToReg, Mem2RegMux), 32);
    regWrite(outMux, registerFile);
    return pc(controlUnit,inst,imm,readRs,zero);
}
int pc(struct ControlUnit* controlUnit,struct instruction* inst,char* imm,int readRs,char*zero){
    struct Mux* branch = (struct Mux*)malloc(sizeof(struct Mux));
    MuxConstruct(int2bin((inst->pc + 1), 32), int2bin(( inst->pc+1 ) + bin2Int(imm, 32), 32),32,branch);
    char* output= OutPutMux(zero,branch);;
    struct Mux4* pc = (struct Mux4*)malloc(sizeof(struct Mux4));
    Mux4Construct(int2bin((inst->pc + 1), 32), output, int2bin(readRs,32), imm, 32,pc);
    inst->pc = bin2Int(pcOutPutMux4(controlUnit->Pc, pc), 32);
    return inst->pc;
}


