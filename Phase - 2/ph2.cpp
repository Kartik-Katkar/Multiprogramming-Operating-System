#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <time.h>
using namespace std;

class PCB{
    public:
        int jobID, TTL, TLL;
        int TTC,LLC;
};

class VM
{
private:
    fstream infile;
    ofstream outfile;
    char Memory[300][4], buffer[40], IR[4], R[4];  
    bool C;     
    int IC;     
    int SI, PI, TI;    
    PCB pcb;
    int PTR;
    int PTE;
    int real_address,virtual_address;
    int pageTable[30];
    int pageTablePTR;
    int page_fault_valid = 0;// 1-Valid  0-Invalid 
    bool Terminate;
    int pageNo;

    void init()
    {
        fill(buffer, buffer + sizeof(buffer), '\0');
        fill(&Memory[0][0], &Memory[0][0] + sizeof(Memory), '\0');
        fill(IR, IR + sizeof(IR), '\0');
        fill(R, R + sizeof(R), '\0');
        C = true;
        IC = 0;
        SI = PI = TI= 0;

        pcb.jobID = pcb.TLL = pcb.TTL = pcb.TTC = pcb.LLC = 0;
        PTR=PTE=pageNo=-1;
        fill(pageTable,pageTable + sizeof(pageTable), 0);
        pageTablePTR = 0;
        Terminate=false;
    }

    void restBuffer()
    {
        fill(buffer, buffer + sizeof(buffer), '\0');
    }

    int Allocate(){
        int pageNo;
        bool check=true;
        while(check){
            pageNo = (rand() % 30) ;
            if(pageTable[pageNo]==0){
                pageTable[pageNo] = 1;
                check=false;
            }
        }
        
        return pageNo;
    }

    void MOS()
    {
        //TI & SI
            if (TI == 0 && SI == 1)
        {
            READ();
        }
        else if (TI == 0 && SI == 2)
        {
            WRITE();
        }
        else if (TI == 0 && SI == 3)
        {
            TERMINATE(0);
        }
        else if (TI == 2 && SI == 1)
        {
            TERMINATE(3);
        }
        else if (TI == 2 && SI == 2)
        {
            WRITE();
            TERMINATE(3);
        }
        else if (TI == 2 && SI == 3)
        {
            TERMINATE(0);
        }

        // TI & PI

         else if (TI == 0 && PI == 1)
        {
            TERMINATE(4);
        }
        else if (TI == 0 && PI == 2)
        {
            TERMINATE(5);
        }
        else if (TI == 0 && PI == 3)
        {
            if(page_fault_valid == 1)
            {
                pageNo = Allocate();
                Memory[PTE][2] = (pageNo / 10) + '0';
                Memory[PTE][3] = (pageNo % 10) + '0';
                pageTablePTR++;
                PI =0;
            }
            else 
            {
                TERMINATE(6);
            }
        }
         else if (TI == 2 && PI == 1)
        {
            TERMINATE(7);
        }
        else if (TI == 2 && PI == 2)
        {
            TERMINATE(8);
        }
        else if (TI == 2 && PI == 3)
        {
            TERMINATE(3);
        }
    }

    void LOAD()
    {
        if (infile.is_open())
        {
            string s;
            while (getline(infile, s))
            {
                if (s[0] == '$' && s[1] == 'A' && s[2] == 'M' && s[3] == 'J')
                {
                 
                    init();
                    cout << "New Job started\n";
                    pcb.jobID = (s[4] - '0') * 1000 + (s[5] - '0') * 100 + (s[6] - '0') * 10 + (s[7] - '0');
                    pcb.TTL = (s[8] - '0') * 1000 + (s[9] - '0') * 100 + (s[10] - '0') * 10 + (s[11] - '0');
                    pcb.TLL = (s[12] - '0') * 1000 + (s[13] - '0') * 100 + (s[14] - '0') * 10 + (s[15] - '0');

                    
                    // Get Frame for Page Table
                    PTR = Allocate()*10; 
                    for(int i=PTR;i<PTR+10;i++){
                        for(int j=0;j<4;j++){
                            Memory[i][j]='*';
                        }
                    }
                    cout << "\nAllocated Page is for Page Table: " << PTR / 10 << "\n";
                    cout << "jobID: " << pcb.jobID << "\nTTL: " << pcb.TTL << "\nTLL: " << pcb.TLL << "\n";

        
                }

                else if (s[0] == '$' && s[1] == 'D' && s[2] == 'T' && s[3] == 'A')
                {
                    cout << "Data card loding\n";
                    restBuffer();
                    STARTEXE();
                }

                else if (s[0] == '$' && s[1] == 'E' && s[2] == 'N' && s[3] == 'D')
                {
                    cout << "END of Job\n";
                    for(int i=0;i<300;i++){
                        if(i==PTR){
                            cout<<"---Page Table---\n";
                        }
                        if(i==PTR+10){
                            cout<<"--Page Table End--\n";
                        }
                        cout<<"[ "<<i<<" ] : ";

                        for(int j=0;j<4;j++){
                            cout<<Memory[i][j]<<" ";
                        }
                        cout<<'\n';
                    }
                }

                else
                {
                    restBuffer();
                    // Get Frame for Program Page
                    pageNo = Allocate();
                    // Memory[PTR + pageTablePTR][0] = '0';
                    Memory[PTR + pageTablePTR][0] = (pageNo / 10) + '0';
                    Memory[PTR + pageTablePTR][1] = (pageNo % 10) + '0';
                    pageTablePTR++;

                    cout << "Program Card loding\n";

                    int length = s.size();
                

                    // Buffer <-- Program Card
                    for (int i = 0; i < length; i++)
                    {
                        buffer[i] = s[i];
                    }
                    
                    int buff = 0;
                    IC = pageNo * 10;
                    int end = IC + 10;

                    // Memory <-- Buffer
                    while (buff < 40 && buffer[buff] != '\0' && IC < end)
                    {
                        for (int j = 0; j < 4; j++)
                        {
                            if (buffer[buff] == 'H')
                            {
                                Memory[IC][j] = 'H';
                                buff++;
                                break;
                            }
                            Memory[IC][j] = buffer[buff];
                            buff++;
                        }
                        IC++;
                    }
                }
            }
            infile.close(); // card File closed
        }
    }

    void READ()
    {
        cout<<"Raed Function called\n";

        string data;
        getline(infile,data);
        if(data[0]=='$' && data[1]=='E' && data[2]=='N' && data[3]=='D'){
            TERMINATE(1);
            return;
        }
        int n=data.size();
        for(int i=0; i<n; i++){
            buffer[i]=data[i];
        }
        int buff=0, memorty_ptr=real_address, end=real_address+10;
        while(buff<40 && buffer[buff]!='\0' && memorty_ptr<end){
            for(int i=0; i<4; i++){
                Memory[memorty_ptr][i]=buffer[buff];
                buff++;
            }
            memorty_ptr++;
        }
        restBuffer();
        SI=0;

    }

    void WRITE(){
        cout<<"Write Function called\n";
        pcb.LLC++;
        if(pcb.LLC>pcb.TLL){
            TERMINATE(2);
            return;
        }
        outfile.open("./output.txt",ios::app);
        bool flag=true;
        for(int i=real_address; i<real_address+9; i++){
            for(int j=0; j<4; j++){
                if(Memory[i][j]!='\0'){
                    outfile<<Memory[i][j];
                }
                else{
                    flag=false;
                    break;
                }
            }
            if(!flag){
                break;
            }
        }
        SI=0;
        outfile<<"\n";
        outfile.close();
    }

    void TERMINATE(int EM){
        Terminate=true;
        outfile.open("./output.txt",ios::app);

        switch(EM){

            case 0:
                outfile<<"NO ERROR : resuot stored successfully\n";
                break;
            case 1:
                outfile<<"ERROR : Out of Data error\n";
                break;
            case 2:
                outfile<<"ERROR : Line limit Exceeded\n";
                break;
            case 3:
                outfile<<"ERROR : Time limit exceeded\n";
                break;
            case 4:
                outfile<<"ERROR : Operation code error\n";
                break;
            case 5:
                outfile<<"ERROR : Operand error\n";
                break;
            case 6:
                outfile<<"ERROR : Invalid page fault\n";
                break;
            case 7:
                outfile<<"ERROR : Time Limit exceeded and Operation code error\n";
                break;
            case 8:
                outfile<<"ERROR : Time Limit exceeded and Operand error\n";
                break;
        }
        outfile<<"JobID : "<<pcb.jobID<<" ";
        outfile<<"IC : "<<IC<<" ";
        outfile<<"IR : ";
        for(int i=0; i<4; i++){
            outfile<<IR[i];
        }
        outfile<<" ";
        outfile<<"SI : "<<SI<<" ";
        outfile<<"PI : "<<PI<<" ";
        outfile<<"TI : "<<TI<<" ";
        outfile<<"TTC : "<<pcb.TTC<<" ";
        outfile<<"LLC : "<<pcb.LLC<<" ";
        outfile<<"TTL : "<<pcb.TTL<<" ";
        outfile<<"TLL : "<<pcb.TLL<<" ";
        outfile<<"\n\n\n\n";
        SI=PI=TI=0;
        outfile.close();
    }
    
    int ADDRESSMAP(int virtual_address)
    {
        if (0 <= virtual_address  &&  virtual_address < 100)
        {
            PTE = PTR + (virtual_address / 10);
            if (Memory[PTE][0] == '*')
            {
                PI = 3; // Page fault no such page exist
                MOS();
            }
            else
            {
                string p;
                p = Memory[PTE][0];
                p += Memory[PTE][1];
                int pageNo = stoi(p);
                real_address = pageNo * 10 + (virtual_address % 10);
                return real_address;
            }
        }
        else
        {
            PI = 2; // Operand Error;
            MOS();
        }
        return pageNo * 10;
    }

    void STARTEXE(){
        IC = 0;
        EXECUTEUSERPROGRAM();
    }

    void EXECUTEUSERPROGRAM()
    { // Slave Mode
        while (!Terminate)
        {   
            real_address = ADDRESSMAP(IC);
            if(PI != 0){
                return;
            }

            for (int i = 0; i < 4; i++){
                IR[i] = Memory[real_address][i];
            }

            IC++;

            string op;
            op += IR[2];
            op += IR[3];

            //GD - GET DATA
            if (IR[0] == 'G' && IR[1] == 'D')
            {
                SIMULATION();
                page_fault_valid = 1;
                if (!isdigit(IR[2]) || !isdigit(IR[3]))
                {
                    PI = 2;
                    MOS();
                }
                else{
                    virtual_address = stoi(op);
                    real_address = ADDRESSMAP(virtual_address);

                    SI = 1;
                    MOS();
                }
            }

            // PD - PRINT DATA  
            else if (IR[0] == 'P' && IR[1] == 'D')
            {
                SIMULATION();
                page_fault_valid=0;
                if (!isdigit(IR[2]) || !isdigit(IR[3]))
                {
                    PI = 2;
                    MOS();
                }
                else{
                    virtual_address = stoi(op);
                    real_address = ADDRESSMAP(virtual_address);
                    SI = 2;
                    MOS();
                }
            }

            // H - HALT
            else if (IR[0] == 'H' && IR[1] == '\0')
            {
                SIMULATION();
                SI = 3;
                MOS();
                return;
            }

            // LR - LOAD DATA (Register  <- Memory)
            else if (IR[0] == 'L' && IR[1] == 'R')
            {
                SIMULATION();
                page_fault_valid = 0;
                if (!isdigit(IR[2]) || !isdigit(IR[3]))
                {
                    PI = 2;
                    MOS();
                }
                else{
                    virtual_address = stoi(op);
                    real_address = ADDRESSMAP(virtual_address);
                    for (int i = 0; i < 4; i++)
                    {
                        R[i] = Memory[real_address][i];
                    }
                }
            }

            // SR - STORE (Memory  <-  Register)
            else if (IR[0] == 'S' && IR[1] == 'R')
            {
                SIMULATION();
                page_fault_valid = 1;
                if (!isdigit(IR[2]) || !isdigit(IR[3]))
                {
                    PI = 2;
                    MOS();
                }
                else{
                    virtual_address = stoi(op);
                    real_address = ADDRESSMAP(virtual_address);
                    for (int i = 0; i < 4; i++)
                    {
                        Memory[real_address][i] = R[i];
                    }
                }
            }

            // CR - COMPARE
            else if (IR[0] == 'C' && IR[1] == 'R')
            {
                SIMULATION();
                page_fault_valid=0;
                if (!isdigit(IR[2]) || !isdigit(IR[3]))
                {
                    PI = 2;
                    MOS();
                }
                else{
                    virtual_address = stoi(op);
                    real_address = ADDRESSMAP(virtual_address);
                    string s1,s2;
                    for (int i = 0; i < 4; i++)
                    {
                        s1+=Memory[real_address][i];
                        s2+=R[i];
                    }
                    if (s1 == s2)
                    {
                        C = true;
                    }
                    else
                    {
                        C = false;
                    }
                }
            }

            // BT (JUMP if toogle is T)
            else if (IR[0] == 'B' && IR[1] == 'T')
            {
                SIMULATION();
                page_fault_valid=0;
                if (!isdigit(IR[2]) || !isdigit(IR[3]))
                {
                    PI = 2;
                    MOS();
                }
                else{
                if (C)
                    {   
                        string j;
                        j+=IR[2];
                        j+=IR[3];
                        IC = stoi(j);
                    }
                }
            }
            else{
                PI = 1;
                SI = 0;
                MOS();

            }
        }
    }

    void SIMULATION()
    {
        if (IR[0] == 'G' && IR[1] == 'D'){
            pcb.TTC += 2;
        }

        else if (IR[0] == 'P' && IR[1] == 'D'){
            pcb.TTC += 1;
        }

        else if (IR[0] == 'H'){
            pcb.TTC += 1;
        }

        else if (IR[0] == 'L' && IR[1] == 'R'){
            pcb.TTC += 1;
        }

        else if (IR[0] == 'S' && IR[1] == 'R'){
            pcb.TTC += 2;
        }

        else if (IR[0] == 'C' && IR[1] == 'R'){
            pcb.TTC += 1;
        }

        else if (IR[0] == 'B' && IR[1] == 'T'){
            pcb.TTC += 1;
        }

        if(pcb.TTC >= pcb.TTL){
            TI=2;
            MOS();
        }
    }

public:
    VM()
    {
        // infile.open("C:\\Users\\91766\\OneDrive\\Desktop\\TY\\os\\CP\\phase 2\\input_phase2.txt", ios::in);
        infile.open("./input_phase2.txt", ios::in);
        // infile.open("./input_Phase1.txt", ios::in);
        // infile.open("C:\\Users\\91766\\OneDrive\\Desktop\\TY\\os\\CP\\input_custom.txt", ios::in);
        init();
        LOAD();
    }
};

int main()
{
    VM v;
    return 0;
}
