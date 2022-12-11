#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>

using namespace std;

#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
	bitset<32>  PC;
	bool        nop;
};

struct IDStruct {
	bitset<32>  Instr;
	bool        nop;
	IDStruct();
};

IDStruct::IDStruct(void) {
	nop = 1;
}

struct EXStruct {
	bitset<32>  Read_data1;
	bitset<32>  Read_data2;
	bitset<16>  Imm;
	bitset<5>   Rs;
	bitset<5>   Rt;
	bitset<5>   Wrt_reg_addr;
	bool        is_I_type;
	bool        rd_mem;
	bool        wrt_mem;
	bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
	bool        wrt_enable;
	bool        nop;
	EXStruct();
};

EXStruct::EXStruct(void) {
	is_I_type = 0;
	rd_mem = 0;
	wrt_mem = 0;
	alu_op = 0;
	wrt_enable = 0;
	nop = 1;
}


struct MEMStruct {
	bitset<32>  ALUresult;
	bitset<32>  Store_data;
	bitset<5>   Rs;
	bitset<5>   Rt;
	bitset<5>   Wrt_reg_addr;
	bool        rd_mem;
	bool        wrt_mem;
	bool        wrt_enable;
	bool        nop;
	MEMStruct();
};

MEMStruct::MEMStruct(void) {
	rd_mem = 0;
	wrt_mem = 0;
	wrt_enable = 0;
	nop = 1;
}

struct WBStruct {
	bitset<32>  Wrt_data;
	bitset<5>   Rs;
	bitset<5>   Rt;
	bitset<5>   Wrt_reg_addr;
	bool        wrt_enable;
	bool        nop;
	WBStruct();
};

WBStruct::WBStruct(void) {
	wrt_enable = 0;
	nop = 1;
}

struct stateStruct {
	IFStruct    IF;
	IDStruct    ID;
	EXStruct    EX;
	MEMStruct   MEM;
	WBStruct    WB;
};

class InsMem
{
public:
	bitset<32> Instruction;
	string id, ioDir;
	InsMem(string name, string ioDir) {
		id = name;
		IMem.resize(MemSize);
		ifstream imem;
		string line;
		int i = 0;
		imem.open(ioDir + "\\imem.txt");
		if (imem.is_open())
		{
			while (getline(imem, line))
			{
				IMem[i] = bitset<8>(line);
				i++;
			}
		}
		else cout << "Unable to open IMEM input file.";
		imem.close();
	}

	bitset<32> readInstr(bitset<32> ReadAddress) {
		// read instruction memory
		// return bitset<32> val
		string insmem;
		insmem.append(IMem[ReadAddress.to_ulong()].to_string());
		insmem.append(IMem[ReadAddress.to_ulong() + 1].to_string());
		insmem.append(IMem[ReadAddress.to_ulong() + 2].to_string());
		insmem.append(IMem[ReadAddress.to_ulong() + 3].to_string());
		Instruction = bitset<32>(insmem);		//read instruction memory
		return Instruction;
	}

private:
	vector<bitset<8> > IMem;
};

class DataMem
{
public:
	bitset<32> ReadData;
	string id, opFilePath, ioDir;
	DataMem(string name, string ioDir) : id{ name }, ioDir{ ioDir } {
		DMem.resize(MemSize);
		opFilePath = ioDir + "\\" + name + "_DMEMResult.txt";
		ifstream dmem;
		string line;
		int i = 0;
		dmem.open(ioDir + "\\dmem.txt");
		if (dmem.is_open())
		{
			while (getline(dmem, line))
			{
				DMem[i] = bitset<8>(line);
				i++;
			}
		}
		else cout << "Unable to open DMEM input file.";
		dmem.close();
	}

	bitset<32> readDataMem(bitset<32> Address) {
		// read data memory
		// return bitset<32> val
		string datamem;
		datamem.append(DMem[Address.to_ulong()].to_string());
		datamem.append(DMem[Address.to_ulong() + 1].to_string());
		datamem.append(DMem[Address.to_ulong() + 2].to_string());
		datamem.append(DMem[Address.to_ulong() + 3].to_string());
		ReadData = bitset<32>(datamem);		//read data memory
		return ReadData;
	}

	void writeDataMem(bitset<32> Address, bitset<32> WriteData) {
		// write into memory
		DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
		DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
		DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
		DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
	}

	void outputDataMem() {
		ofstream dmemout;
		dmemout.open(opFilePath, std::ios_base::trunc);
		if (dmemout.is_open()) {
			for (int j = 0; j < 1000; j++)
			{
				dmemout << DMem[j] << endl;
			}

		}
		else cout << "Unable to open " << id << " DMEM result file." << endl;
		dmemout.close();
	}
	vector<bitset<8> > DMem;

private:
};

class RegisterFile
{
public:
	bitset<32> Reg_data;
	string outputFile;
	RegisterFile(string ioDir) : outputFile{ ioDir + "RFResult.txt" } {
		Registers.resize(32);
		Registers[0] = bitset<32>(0);
	}

	bitset<32> readRF(bitset<5> Reg_addr) {
		// Fill in
		Reg_data = Registers[Reg_addr.to_ulong()];
		return Reg_data;
	}

	void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data) {
		// Fill in
		Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
	}

	void outputRF(int cycle) {
		ofstream rfout;
		if (cycle == 0)
			rfout.open(outputFile, std::ios_base::trunc);
		else
			rfout.open(outputFile, std::ios_base::app);
		if (rfout.is_open())
		{
			rfout << "State of RF after executing cycle:\t" << cycle << endl;
			for (int j = 0; j < 32; j++)
			{
				rfout << Registers[j] << endl;
			}
		}
		else cout << "Unable to open RF output file." << endl;
		rfout.close();
	}

private:
	vector<bitset<32> >Registers;
};

class Core {
public:
	RegisterFile myRF;
	uint32_t cycle = 0;
	bool halted = false;
	string ioDir;
	struct stateStruct state, nextState;
	InsMem ext_imem;
	DataMem ext_dmem;

	Core(string ioDir, InsMem& imem, DataMem& dmem) : myRF(ioDir), ioDir{ ioDir }, ext_imem{ imem }, ext_dmem{ dmem } {}

	virtual void step() {}

	virtual void printState() {}
};

class SingleStageCore : public Core {
public:
	SingleStageCore(string ioDir, InsMem& imem, DataMem& dmem) : Core(ioDir + "\\SS_", imem, dmem), opFilePath(ioDir + "\\StateResult_SS.txt") {}

	void step() {
		/* Your implementation*/
				// Fetch
		bitset<32> insturciton = ext_imem.readInstr(state.IF.PC);
		bitset<32> Rs, Rt, Rd;
		//bitset<32> Rs_value, Rt_value;
		bitset<6> funct;
		bitset<7> funct7;
		bitset<3> funct3;
		bitset<16> imm;
		bitset<26> address;
		bitset<32>temp;
		bitset<32> Offset;
		string immPart;

		// If current insturciton is "11111111111111111111111111111111", then break;
		if (insturciton == bitset<32>(0xFFFFFFFF))
		{
			nextState.IF.nop = 1;
		}

		// decode(Read RF)
		int Op_code = stoi(insturciton.to_string().substr(25, 7), 0, 2);
		switch (Op_code)
		{
		case 0x00:
			state.EX.Rs = bitset<5>(insturciton.to_string(), 6, 5);
			state.EX.Rt = bitset<5>(insturciton.to_string(), 11, 5);
			state.EX.Wrt_reg_addr = bitset<5>(insturciton.to_string(), 16, 5);
			funct = bitset<6>(insturciton.to_string(), 26, 6);

			myRF.readRF(state.EX.Rs);
			state.EX.Read_data1 = myRF.Reg_data;
			myRF.readRF(state.EX.Rt);
			state.EX.Read_data2 = myRF.Reg_data;
			myRF.writeRF(0, 0);
			// myRF.ReadWrite(Rs, Rt, 0, 0, 0);
			//Rs_value = myRF.ReadData1;
			//Rt_value = myRF.ReadData2;

			switch (funct.to_ulong())
			{
			case 0x21:		// addu
				nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong());
				myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
				nextState.EX.alu_op = 1;
				//myRF.ReadWrite(0, 0, Rd, myALU.ALUOperation(ADDU, Rs_value, Rt_value), 1);
				break;
			case 0x23:		// subu
				nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());
				myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
				nextState.EX.alu_op = 0;
				//myRF.ReadWrite(0, 0, Rd, myALU.ALUOperation(SUBU, Rs_value, Rt_value), 1);
				break;
			case 0x24:		// and
				nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() & state.EX.Read_data2.to_ulong());
				myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
				nextState.EX.alu_op = 0;
				break;
			case 0x25:		// or
				nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() | state.EX.Read_data2.to_ulong());
				myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
				nextState.EX.alu_op = 0;
				break;
			case 0x26:		// nor
				nextState.MEM.ALUresult = bitset<32>(~(state.EX.Read_data1.to_ulong() | state.EX.Read_data2.to_ulong()));
				myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
				nextState.EX.alu_op = 0;
				break;
			default:
				break;
			}
			nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
			break;

		case 0x03:	// lw
			// imm = bitset<16>(insturciton.to_string(), 0, 12);
			state.EX.Rs = bitset<5>(insturciton.to_string(), 12, 5);	// address of Rs1
			state.EX.Wrt_reg_addr = bitset<5>(insturciton.to_string(), 20, 5); //address of Rd
			Offset = bitset<32>(insturciton.to_string(), 0, 12);
			Rs = bitset<32>(state.EX.Rs.to_ulong() + bitset<32>(Offset.to_ulong()).to_ulong());
			// ext_dmem.readDataMem(bitset<32>(state.EX.Rs.to_ulong()));
			ext_dmem.readDataMem(Rs);	// data of Rs1
			myRF.writeRF(state.EX.Wrt_reg_addr, ext_dmem.ReadData);	// write data of Rs1 to Rd
			nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
			break;

		case 0x04:
			state.EX.Rs = bitset<5>(insturciton.to_string(), 6, 5);
			state.EX.Rt = bitset<5>(insturciton.to_string(), 11, 5);
			imm = bitset<16>(insturciton.to_string(), 16, 16);
			myRF.readRF(state.EX.Rs);
			state.EX.Read_data1 = myRF.Reg_data;
			myRF.readRF(state.EX.Rt);
			state.EX.Read_data2 = myRF.Reg_data;
			// myRF.ReadWrite(Rs, Rt, 0, 0, 0);

			//Rs_value = myRF.ReadData1;
			//Rt_value = myRF.ReadData2;
			if (state.EX.Read_data1 == state.EX.Read_data2)
			{
				temp = bitset<32>(imm.to_ulong()) << 2;
				nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + temp.to_ulong());
				// pc = myALU.ALUOperation(ADDU, pc, bitset<32>(imm.to_ulong()) << 2);
			}

			nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
			break;

		case 0x09:
			state.EX.Rs = bitset<5>(insturciton.to_string(), 6, 5);
			state.EX.Rt = bitset<5>(insturciton.to_string(), 11, 5);
			imm = bitset<16>(insturciton.to_string(), 16, 16);

			myRF.readRF(state.EX.Rs);
			// myRF.ReadWrite(Rs, 0, 0, 0, 0);
			state.EX.Read_data1 = myRF.Reg_data;
			// Rs_value = myRF.ReadData1;
			nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + (imm.to_ulong()));
			myRF.writeRF(state.EX.Rt, nextState.MEM.ALUresult);
			// myRF.ReadWrite(0, 0, Rt, myALU.ALUOperation(ADDU, Rs_value, bitset<32>(imm.to_ulong())), 1);
			nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);
			// pc = myALU.ALUOperation(ADDU, pc, bitset<32>(4));
			break;

		case 0x0D:	// I-type
			funct3 = bitset<3>(insturciton.to_string(), 17, 3);
			switch (funct3.to_ulong())
			{
			case 0x00:	// addi
				imm = bitset<16>(insturciton.to_string(), 0, 12);
				state.EX.Rs = bitset<5>(insturciton.to_string(), 12, 5);	// address of Rs1
				state.EX.Wrt_reg_addr = bitset<5>(insturciton.to_string(), 20, 5); //address of Rd
				myRF.readRF(state.EX.Rs);
				state.EX.Read_data1 = myRF.Reg_data;						// data of Rs1
				nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + (imm.to_ulong()));
				myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
				nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
				break;

			case 0x04:	// XORI
				imm = bitset<16>(insturciton.to_string(), 0, 12);
				state.EX.Rs = bitset<5>(insturciton.to_string(), 12, 5);	// address of Rs1
				state.EX.Wrt_reg_addr = bitset<5>(insturciton.to_string(), 20, 5); //address of Rd
				myRF.readRF(state.EX.Rs);
				state.EX.Read_data1 = myRF.Reg_data;						// data of Rs1
				nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() ^ (imm.to_ulong()));
				myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
				nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
				break;

			case 0x06:	// ORI
				imm = bitset<16>(insturciton.to_string(), 0, 12);
				state.EX.Rs = bitset<5>(insturciton.to_string(), 12, 5);	// address of Rs1
				state.EX.Wrt_reg_addr = bitset<5>(insturciton.to_string(), 20, 5); //address of Rd
				myRF.readRF(state.EX.Rs);
				state.EX.Read_data1 = myRF.Reg_data;						// data of Rs1
				nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() | (imm.to_ulong()));
				myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
				nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
				break;

			case 0X07:	// ANDI
				imm = bitset<16>(insturciton.to_string(), 0, 12);
				state.EX.Rs = bitset<5>(insturciton.to_string(), 12, 5);	// address of Rs1
				state.EX.Wrt_reg_addr = bitset<5>(insturciton.to_string(), 20, 5); //address of Rd
				myRF.readRF(state.EX.Rs);
				state.EX.Read_data1 = myRF.Reg_data;						// data of Rs1
				nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() & (imm.to_ulong()));
				myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
				nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
				break;

			default:
				break;
			}
			break;

		case 0x23:		// sw
			state.EX.Rs = bitset<5>(insturciton.to_string(), 12, 5);		// Rs1
			state.EX.Rt = bitset<5>(insturciton.to_string(), 7, 5);			// Rs2
			immPart = insturciton.to_string().substr(0, 7) + insturciton.to_string().substr(20, 5);
			imm = stoi(immPart, 0, 2);
			Rs = bitset<32>(state.EX.Rs.to_ulong() + imm.to_ulong());
			myRF.readRF(state.EX.Rt);	// data of Rs2
			ext_dmem.writeDataMem(Rs, myRF.Reg_data);
			// myRF.writeRF(Rs, myRF.Reg_data);
			//// myRF.ReadWrite(Rs, 0, 0, 0, 0);
			//state.EX.Read_data1 = myRF.Reg_data;
			//// Rs_value = myRF.ReadData1;
			//nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + (imm.to_ulong()));
			//ext_dmem.readDataMem(nextState.MEM.ALUresult);
			//myRF.writeRF(state.EX.Rt, ext_dmem.ReadData);
			//state.EX.alu_op = 1;
			//// myRF.ReadWrite(0, 0, Rt, myDataMem.MemoryAccess(myALU.ALUOperation(ADDU, Rs_value, bitset<32>(imm.to_ulong())), 0, 1, 0), 1);
			nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
			break;

		case 0x33:	// R-typr
			state.EX.Rs = bitset<5>(insturciton.to_string(), 12, 5);		// address of Rs1
			state.EX.Rt = bitset<5>(insturciton.to_string(), 7, 5);	// address of Rs2
			state.EX.Wrt_reg_addr = bitset<5>(insturciton.to_string(), 20, 5); //address of Rd
			funct7 = bitset<7>(insturciton.to_string(), 0, 7);
			funct3 = bitset<3>(insturciton.to_string(), 17, 3);
			switch (funct7.to_ulong())
			{
			case 0x00:	// funct7:000
				switch (funct3.to_ulong())
				{
				case 0x00:	// add
					state.EX.Rs = bitset<5>(insturciton.to_string(), 12, 5);		// address of Rs1
					myRF.readRF(state.EX.Rs);
					state.EX.Read_data1 = myRF.Reg_data;
					state.EX.Rt = bitset<5>(insturciton.to_string(), 7, 5);			// address of Rs2
					myRF.readRF(state.EX.Rt);
					state.EX.Read_data2 = myRF.Reg_data;							//data of Rs2
					state.EX.Wrt_reg_addr = bitset<5>(insturciton.to_string(), 20, 5);	// address Rd
					nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong());
					myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
					nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
					break;

				case 0x04:	//xor
					state.EX.Rs = bitset<5>(insturciton.to_string(), 12, 5);		// address of Rs1
					myRF.readRF(state.EX.Rs);
					state.EX.Read_data1 = myRF.Reg_data;
					state.EX.Rt = bitset<5>(insturciton.to_string(), 7, 5);			// address of Rs2
					myRF.readRF(state.EX.Rt);
					state.EX.Read_data2 = myRF.Reg_data;							//data of Rs2
					state.EX.Wrt_reg_addr = bitset<5>(insturciton.to_string(), 20, 5);	// address Rd
					nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() ^ state.EX.Read_data2.to_ulong());
					myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
					nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
					break;

				case 0x06:	// or
					state.EX.Rs = bitset<5>(insturciton.to_string(), 12, 5);		// address of Rs1
					myRF.readRF(state.EX.Rs);
					state.EX.Read_data1 = myRF.Reg_data;
					state.EX.Rt = bitset<5>(insturciton.to_string(), 7, 5);			// address of Rs2
					myRF.readRF(state.EX.Rt);
					state.EX.Read_data2 = myRF.Reg_data;							//data of Rs2
					state.EX.Wrt_reg_addr = bitset<5>(insturciton.to_string(), 20, 5);	// address Rd
					nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() | state.EX.Read_data2.to_ulong());
					myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
					nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
					break;

				case 0x07:	// and
					state.EX.Rs = bitset<5>(insturciton.to_string(), 12, 5);		// address of Rs1
					myRF.readRF(state.EX.Rs);
					state.EX.Read_data1 = myRF.Reg_data;
					state.EX.Rt = bitset<5>(insturciton.to_string(), 7, 5);			// address of Rs2
					myRF.readRF(state.EX.Rt);
					state.EX.Read_data2 = myRF.Reg_data;							//data of Rs2
					state.EX.Wrt_reg_addr = bitset<5>(insturciton.to_string(), 20, 5);	// address Rd
					nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() & state.EX.Read_data2.to_ulong());
					myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);
					nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
					break;

				default:
					break;
				}
				break;
			case 0x20:	// sub
				state.EX.Rs = bitset<5>(insturciton.to_string(), 12, 5);		// Rs1
				myRF.readRF(state.EX.Rs);
				state.EX.Read_data1 = myRF.Reg_data;							// data stored in Rs1
				state.EX.Rt = bitset<5>(insturciton.to_string(), 7, 5);			// Rs2
				myRF.readRF(state.EX.Rt);
				state.EX.Read_data2 = myRF.Reg_data;							// data stored in Rs2
				state.EX.Wrt_reg_addr = bitset<5>(insturciton.to_string(), 20, 5);	// address Rd
				nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());	// Rs1-Rs2
				myRF.writeRF(state.EX.Wrt_reg_addr, nextState.MEM.ALUresult);	// store in Rd
				nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
				break;

			default:
				break;
			}
			break;
		case 0x2B:
			state.EX.Rs = bitset<5>(insturciton.to_string(), 6, 5);
			state.EX.Rt = bitset<5>(insturciton.to_string(), 11, 5);
			imm = bitset<16>(insturciton.to_string(), 16, 16);
			myRF.readRF(state.EX.Rs);
			state.EX.Read_data1 = myRF.Reg_data; // Rs
			myRF.readRF(state.EX.Rt);
			state.EX.Read_data2 = myRF.Reg_data; // Rt
			// myRF.ReadWrite(Rs, Rt, 0, 0, 0);
			//Rs_value = myRF.ReadData1;
			//Rt_value = myRF.ReadData2;
			state.MEM.ALUresult = bitset<32>(imm.to_ulong() + state.EX.Read_data1.to_ulong());
			ext_dmem.writeDataMem(state.MEM.ALUresult, state.EX.Read_data2);
			// myDataMem.MemoryAccess(myALU.ALUOperation(ADDU, Rs_value, bitset<32>(imm.to_ulong())), Rt_value, 0, 1);
			nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
			break;
		case 0x02:
			address = bitset<26>(insturciton.to_string(), 6, 26);
			state.MEM.ALUresult = bitset<32>(state.IF.PC.to_ulong() + 4); // pc+4
			state.MEM.ALUresult = bitset<32>(state.MEM.ALUresult.to_ulong() & 0xF0000000);
			temp = bitset<32>(address.to_ulong()) << 2;
			state.MEM.ALUresult = bitset<32>(state.MEM.ALUresult.to_ulong() | temp.to_ulong());
			// pc = myALU.ALUOperation(OR, bitset<32>(address.to_ulong()) << 2, myALU.ALUOperation(AND, myALU.ALUOperation(ADDU, pc, bitset<32>(4)), bitset<32>(0xF0000000)));
			break;
		case 0x3F:
			break;

		default:
			break;
		}

		if (state.IF.nop)
			halted = true;

		myRF.outputRF(cycle); // dump RF
		printState(nextState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 

		state = nextState; // The end of the cycle and updates the current state with the values calculated in this cycle
		cycle++;
	}

	void printState(stateStruct state, int cycle) {
		ofstream printstate;
		if (cycle == 0)
			printstate.open(opFilePath, std::ios_base::trunc);
		else
			printstate.open(opFilePath, std::ios_base::app);
		if (printstate.is_open()) {
			printstate << "State after executing cycle:\t" << cycle << endl;

			printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
			printstate << "IF.nop:\t" << state.IF.nop << endl;
		}
		else cout << "Unable to open SS StateResult output file." << endl;
		printstate.close();
	}
private:
	string opFilePath;
};

class FiveStageCore : public Core {
public:

	FiveStageCore(string ioDir, InsMem& imem, DataMem& dmem) : Core(ioDir + "\\FS_", imem, dmem), opFilePath(ioDir + "\\StateResult_FS.txt") {}

	void step() {
		/* Your implementation */
		/* --------------------- WB stage --------------------- */
		if (!state.WB.nop)
		{
			if (state.WB.wrt_enable)
			{
				myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
			}
			if (state.MEM.nop)
			{
				nextState.WB.nop = state.MEM.nop;
			}
		}


		/* --------------------- MEM stage -------------------- */
		if (!state.MEM.nop)
		{
			nextState.WB.Rs = state.MEM.Rs;
			nextState.WB.Rt = state.MEM.Rt;
			nextState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
			nextState.WB.wrt_enable = state.MEM.wrt_enable;

			if (state.MEM.rd_mem)           // lw
			{
				nextState.WB.Wrt_data = ext_dmem.readDataMem(state.MEM.ALUresult);
				nextState.WB.nop = 0;
			}
			else if (state.MEM.wrt_mem)     // sw
			{
				// RAW Hazard: MEM-MEM
				if (state.WB.wrt_enable)
				{
					if (state.WB.Wrt_reg_addr == state.MEM.Rt)
					{
						state.MEM.Store_data = state.WB.Wrt_data;
					}
				}

				ext_dmem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
				// TODO: check WB.Wrt_data
				nextState.WB.Wrt_data = ext_dmem.readDataMem(state.MEM.ALUresult);
				nextState.WB.nop = 0;
			}
			else                            // addu & subu & beq
			{
				nextState.WB.Wrt_data = state.MEM.ALUresult;
				nextState.WB.nop = 0;
			}

			if (state.EX.nop)
			{
				nextState.MEM.nop = state.EX.nop;
			}
		}


		/* --------------------- EX stage --------------------- */
		if (!state.EX.nop)
		{
			nextState.MEM.Rs = state.EX.Rs;
			nextState.MEM.Rt = state.EX.Rt;
			nextState.MEM.rd_mem = state.EX.rd_mem;
			nextState.MEM.wrt_mem = state.EX.wrt_mem;
			nextState.MEM.Store_data = state.EX.Read_data2;
			nextState.MEM.wrt_enable = state.EX.wrt_enable;
			nextState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;

			// RAW Hazards: MEM-EX
			if (state.MEM.wrt_enable)
			{
				if (state.MEM.Wrt_reg_addr == state.EX.Rs)
				{
					state.EX.Read_data1 = nextState.WB.Wrt_data;
				}
				if (state.MEM.Wrt_reg_addr == state.EX.Rt)
				{
					state.EX.Read_data2 = nextState.WB.Wrt_data;
					nextState.MEM.Store_data = state.EX.Read_data2;
				}
			}

			// RAW Hazards: MEM-EX
			if (state.WB.wrt_enable)
			{
				if (state.WB.Wrt_reg_addr == state.EX.Rs)
				{
					state.EX.Read_data1 = state.WB.Wrt_data;
				}
				if (state.WB.Wrt_reg_addr == state.EX.Rt)
				{
					state.EX.Read_data2 = state.WB.Wrt_data;
					nextState.MEM.Store_data = state.EX.Read_data2;
				}
			}

			if (!state.EX.is_I_type)
			{
				if (state.EX.alu_op)    // addu
				{
					nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong());
				}
				else                    // subs
				{
					nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());
				}
			}
			else
			{
				bitset<32> Offset = bitset<32>(state.EX.Imm.to_ulong());
				if (state.EX.alu_op)
				{
					if (state.EX.Imm[15] == 1)
					{
						for (int i = 31; i > 15; i--)
						{
							Offset[i] = 1;
						}
					}

					if (state.EX.rd_mem)    // lw
					{
						nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + bitset<32>(Offset.to_ulong()).to_ulong());
					}
					else                    // sw
					{
						nextState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + bitset<32>(Offset.to_ulong()).to_ulong());
					}
				}
			}
			nextState.MEM.nop = 0;

			if (state.ID.nop)
			{
				nextState.EX.nop = state.ID.nop;
			}
		}


		/* --------------------- ID stage --------------------- */
		if (!state.ID.nop)
		{
			nextState.EX.Rs = bitset<5>(state.ID.Instr.to_string(), 6, 5);
			nextState.EX.Rt = bitset<5>(state.ID.Instr.to_string(), 11, 5);
			nextState.EX.Imm = bitset<16>(state.ID.Instr.to_string(), 16, 16);
			nextState.EX.Read_data1 = myRF.readRF(nextState.EX.Rs);
			nextState.EX.Read_data2 = myRF.readRF(nextState.EX.Rt);

			unsigned Op_code = (state.ID.Instr >> 26).to_ulong();
			switch (Op_code)
			{
			case 0x00:
				nextState.EX.Wrt_reg_addr = bitset<5>(state.ID.Instr.to_string(), 16, 5);
				nextState.EX.is_I_type = 0;
				nextState.EX.wrt_enable = 1;
				nextState.EX.rd_mem = 0;
				nextState.EX.wrt_mem = 0;
				switch (bitset<6>(state.ID.Instr.to_string(), 26, 6).to_ulong())
				{
				case 0x21:
					nextState.EX.alu_op = 1;
					nextState.EX.nop = 0;
					break;

				case 0x23:
					nextState.EX.alu_op = 0;
					nextState.EX.nop = 0;
					break;

				default:
					break;
				}
				break;
			case 0x23:              // lw
				nextState.EX.Wrt_reg_addr = nextState.EX.Rt;
				nextState.EX.alu_op = 1;
				nextState.EX.is_I_type = 1;
				nextState.EX.wrt_enable = 1;
				nextState.EX.rd_mem = 1;
				nextState.EX.wrt_mem = 0;
				nextState.EX.nop = 0;
				break;

			case 0x2B:              // sw
				nextState.EX.Wrt_reg_addr = nextState.EX.Rt;
				nextState.EX.alu_op = 1;
				nextState.EX.is_I_type = 1;
				nextState.EX.wrt_enable = 0;
				nextState.EX.rd_mem = 0;
				nextState.EX.wrt_mem = 1;
				nextState.EX.nop = 0;
				break;

			case 0x04:              // beq
				nextState.EX.Wrt_reg_addr = nextState.EX.Rt;
				nextState.EX.alu_op = 0;
				nextState.EX.is_I_type = 1;
				nextState.EX.wrt_enable = 0;
				nextState.EX.rd_mem = 0;
				nextState.EX.wrt_mem = 0;

				if (nextState.EX.Read_data1 != nextState.EX.Read_data2)
				{
					bitset<32> BranchAddr;

					// SignExtend
					if (nextState.EX.Imm[15] == 1)
					{
						BranchAddr = bitset<32>(nextState.EX.Imm.to_ulong()) << 2;
						for (int i = 31; i > 17; i--)
						{
							BranchAddr[i] = 1;
						}
					}
					else
					{
						BranchAddr = bitset<32>(nextState.EX.Imm.to_ulong()) << 2;
					}

					state.IF.PC = bitset<32>(state.IF.PC.to_ulong() + BranchAddr.to_ulong());
				}
				nextState.EX.nop = 0;
				break;

			default:
				break;
			}

			if (nextState.MEM.rd_mem && !nextState.EX.is_I_type)
			{
				if (halted)
				{
					if (nextState.MEM.Wrt_reg_addr == nextState.EX.Rs || nextState.MEM.Wrt_reg_addr == nextState.EX.Rt)
					{
						nextState.EX.nop = 1;
						state.IF.PC = bitset<32>(state.IF.PC.to_ulong() - 4);
						halted = 0;
					}
				}
				else
				{
					halted = 1;
				}
			}
		}


		/* --------------------- IF stage --------------------- */
		if (!state.IF.nop)
		{
			nextState.ID.Instr = ext_imem.readInstr(state.IF.PC);
			if (nextState.ID.Instr == bitset<32>(0xFFFFFFFF))
			{
				nextState.IF.nop = 1;
				nextState.ID.nop = 1;
			}
			else
			{
				nextState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);
				nextState.IF.nop = 0;
				nextState.ID.nop = 0;
			}
		}

		if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
			halted = true;

		myRF.outputRF(cycle); // dump RF
		printState(nextState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 

		state = nextState; //The end of the cycle and updates the current state with the values calculated in this cycle
		cycle++;
	}

	void printState(stateStruct state, int cycle) {
		ofstream printstate;
		if (cycle == 0)
			printstate.open(opFilePath, std::ios_base::trunc);
		else
			printstate.open(opFilePath, std::ios_base::app);
		if (printstate.is_open()) {
			printstate << "State after executing cycle:\t" << cycle << endl;

			printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
			printstate << "IF.nop:\t" << state.IF.nop << endl;

			printstate << "ID.Instr:\t" << state.ID.Instr << endl;
			printstate << "ID.nop:\t" << state.ID.nop << endl;

			printstate << "EX.Read_data1:\t" << state.EX.Read_data1 << endl;
			printstate << "EX.Read_data2:\t" << state.EX.Read_data2 << endl;
			printstate << "EX.Imm:\t" << state.EX.Imm << endl;
			printstate << "EX.Rs:\t" << state.EX.Rs << endl;
			printstate << "EX.Rt:\t" << state.EX.Rt << endl;
			printstate << "EX.Wrt_reg_addr:\t" << state.EX.Wrt_reg_addr << endl;
			printstate << "EX.is_I_type:\t" << state.EX.is_I_type << endl;
			printstate << "EX.rd_mem:\t" << state.EX.rd_mem << endl;
			printstate << "EX.wrt_mem:\t" << state.EX.wrt_mem << endl;
			printstate << "EX.alu_op:\t" << state.EX.alu_op << endl;
			printstate << "EX.wrt_enable:\t" << state.EX.wrt_enable << endl;
			printstate << "EX.nop:\t" << state.EX.nop << endl;

			printstate << "MEM.ALUresult:\t" << state.MEM.ALUresult << endl;
			printstate << "MEM.Store_data:\t" << state.MEM.Store_data << endl;
			printstate << "MEM.Rs:\t" << state.MEM.Rs << endl;
			printstate << "MEM.Rt:\t" << state.MEM.Rt << endl;
			printstate << "MEM.Wrt_reg_addr:\t" << state.MEM.Wrt_reg_addr << endl;
			printstate << "MEM.rd_mem:\t" << state.MEM.rd_mem << endl;
			printstate << "MEM.wrt_mem:\t" << state.MEM.wrt_mem << endl;
			printstate << "MEM.wrt_enable:\t" << state.MEM.wrt_enable << endl;
			printstate << "MEM.nop:\t" << state.MEM.nop << endl;

			printstate << "WB.Wrt_data:\t" << state.WB.Wrt_data << endl;
			printstate << "WB.Rs:\t" << state.WB.Rs << endl;
			printstate << "WB.Rt:\t" << state.WB.Rt << endl;
			printstate << "WB.Wrt_reg_addr:\t" << state.WB.Wrt_reg_addr << endl;
			printstate << "WB.wrt_enable:\t" << state.WB.wrt_enable << endl;
			printstate << "WB.nop:\t" << state.WB.nop << endl;
		}
		else cout << "Unable to open FS StateResult output file." << endl;
		printstate.close();
	}
private:
	string opFilePath;
};

int main(int argc, char* argv[]) {

	string ioDir = "";
	if (argc == 1) {
		cout << "Enter path containing the memory files: ";
		cin >> ioDir;
	}
	else if (argc > 2) {
		cout << "Invalid number of arguments. Machine stopped." << endl;
		return -1;
	}
	else {
		ioDir = argv[1];
		cout << "IO Directory: " << ioDir << endl;
	}

	InsMem imem = InsMem("Imem", ioDir);
	DataMem dmem_ss = DataMem("SS", ioDir);
	DataMem dmem_fs = DataMem("FS", ioDir);

	SingleStageCore SSCore(ioDir, imem, dmem_ss);
	FiveStageCore FSCore(ioDir, imem, dmem_fs);

	while (1) {
		if (!SSCore.halted)
			SSCore.step();

		if (!FSCore.halted)
			FSCore.step();

		if (SSCore.halted && FSCore.halted)
			break;
	}

	// dump SS and FS data mem.
	SSCore.ext_dmem.outputDataMem();
	FSCore.ext_dmem.outputDataMem();

	return 0;
}