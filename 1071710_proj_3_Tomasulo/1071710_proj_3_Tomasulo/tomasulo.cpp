#include<iostream>
#include<cstring>
#include<string>
#include<fstream>
#include<iomanip>
#include<math.h>
#include<vector>
#include<map>
#include<sstream>
#include<queue>
using namespace std;
struct ALU {
	float outcome = 0;
	bool is_empty = 1;
	int cycle = 0;
	int RS = 0;
};
ALU add_ALU;
ALU mul_ALU;
float Register[32] = {};
string RAT[32] = {};
int add_sub_rs_num = 0;
int mul_div_rs_num = 0;
vector<vector<string>> RS;
queue<vector<string>> initial_queue;
map<string, int> label; // 用於存放Label之對應要跳的行數
int add_cycle = 0;
int sub_cycle = 0;
int mul_cycle = 0;
int div_cycle = 0;
string deleteComment(string instruction); //把註解除掉
string which_type(vector<string> instruction);
bool issue();
void capture();
void execute();
void write_result();
bool rs_empty_check(vector<vector<string>> RS, int total_rs_num);
queue<vector<string>> read_file();
vector<vector<string>> rs_initialize(vector<vector<string>> rs, int rs_num);
float operation_result(string operation, string rs1, string rs2);
string get_operation(string operation);
void print_cycle();
int main() {
	int cycle = 0; // cycle數
	while (true) {
		cycle = 0;
		initial_queue = read_file(); //get insturction
		for (int i = 0; i < 32; i++) {
			Register[i] = 0;
			RAT[i] = "\0";
		} //Register 初始化
//		Register[1] = 0;
//		Register[2] = 2;
//		Register[3] = 4;
//		Register[4] = 2.5;
//		Register[5] = 8;
		cout << "請設定Reserve station(ADD/SUB)之大小: "<<endl;
		cin >> add_sub_rs_num;
		cout << "請設定Reserve station(MUL/DIV)之大小: " << endl;
		cin >> mul_div_rs_num;
		mul_div_rs_num = add_sub_rs_num + mul_div_rs_num;
		int total_rs_num = add_sub_rs_num + mul_div_rs_num;
		RS = rs_initialize(RS, total_rs_num);
		cout << "請設定ADD(含Immediate)之ALU cycle數: " << endl;
		cin >> add_cycle;
		cout << "請設定SUB之ALU cycle數: " << endl;
		cin >> sub_cycle;
		cout << "請設定MUL之ALU cycle數: " << endl;
		cin >> mul_cycle;
		cout << "請設定DIV之ALU cycle數: " << endl;
		cin >> div_cycle;
		while (!(initial_queue.empty() && rs_empty_check(RS, total_rs_num))) {
			cycle += 1;
			cout << "##################Cycle: " << cycle << "##################" << endl;
			if (!initial_queue.empty())
			{
				issue();
			}
			capture();
			execute();
			write_result();
			//印出Register, RAT, RS, ALU之狀態
			print_cycle();
			cout << "################################################" <<endl<< endl;
		}
	}
	return 0;
}
queue<vector<string>> read_file() {
	ifstream fin1; //讀檔
	string input;
	int index = 0;
	int position = 0;
	vector<string>temp;// 暫存使用
	queue<vector<string>> instruction;//存放instruction的vector，會進行切割處理
	string file_name = "0"; //紀錄檔案位置
	//輸入檔案名稱，若不存在則請使用者重新輸入
	cout << "Welcome! please input the file name to start the program or enter 0 to exit!" << endl;
	while (true) {
		cout << "File name: ";
		cin >> file_name;
		if (file_name == "0") {
			cout << "Thank you for using this program." << endl;
			exit(3);
		}
		fin1.open(file_name); //read files.
		if (!fin1) {
			cout << "The file does not exist, please try again." << endl;
		}
		else
			break;
	}
	while (getline(fin1, input)) {//讀每一個instruction並放入vector中
		if (input[input.size() - 1] == ':') {//遇到Label要如此處理
			while (input.find(':') != -1) {
				input.erase(input.find(':'), 1);
			}
			label.insert(pair<string, int>(input, index));//把對應的Index放入
			index--;
		}
		else {//當作Instruction處理，先將\t處理掉，再根據找到的第一個空格切割，拿到operator
			while (input.find('\t') != -1) {
				input.erase(input.find('\t'), 1);
			}
			position = input.find_first_of(' ');
			temp.push_back(input.substr(0, position));//將operator放入
			input = input.substr(position + 1);
			input = deleteComment(input);//處理註解
			stringstream ss(input);//根據逗號的位置切割字串，並將每次切割出來的字串再存入該instruction的vector中，所以會根據這個順序依序拿到opcode rd rs1 rs2等等...
			while (getline(ss, input, ',')) {
				if (!input.empty()) {
					temp.push_back(input);
				}
			}
			instruction.push(temp);
			temp.clear();
		}
		index++;
	}
	return instruction;
}
string deleteComment(string instruction) {
	int position = 0;
	position = instruction.find(";");
	if (position != -1) {
		instruction.erase(position);
	}
	position = instruction.find("//");
	if (position != -1) {
		instruction.erase(position);
	}
	while (instruction.find(' ') != -1) {
		instruction.erase(instruction.find(' '), 1);
	}
	return instruction;
}
string which_type(vector<string> instruction) {
	if (instruction[0] == "add" || instruction[0] == "sub" || instruction[0] == "addi") {
		return "add";
	}
	else if (instruction[0] == "mul" || instruction[0] == "div") {
		return "mul";
	}
}
string get_operation(string operation) {
	if (operation == "add" || operation == "addi")
		return "+";
	if (operation == "sub")
		return "-";
	if (operation == "mul")
		return "*";
	if (operation == "div")
		return "/";
}
vector<vector<string>> rs_initialize(vector<vector<string>> rs, int rs_num) {
	vector<string> temp;
	for (int i = 0; i < rs_num; i++) {
		temp.push_back("\0");
		temp.push_back("\0");
		temp.push_back("\0");
		rs.push_back(temp);
		temp.clear();
	}
	return rs;
}
bool rs_empty_check(vector<vector<string>> RS, int total_rs_num) {	
	for (int i = 0; i < total_rs_num; i++) {
		if (RS[i][0] != "\0")
			return 0;
	}
	return 1;
}
bool issue() {
	vector<string> temp = initial_queue.front();
	if (which_type(temp) == "add") {
		for (int i = 0; i < add_sub_rs_num; i++) {
			if (RS[i][0] == "\0") {
				RS[i][0] = temp[0];
				string operation = RS[i][0];
				string rd = temp[1].substr(1);
				string rs1 = temp[2].substr(1);
				//1. 本身是數字就直接丟進去
				//2. check RAT, if RAT is empty, go to register else put RAT
				//3. update RAT
				if (RAT[stoi(rs1)] == "\0")
				{
					RS[i][1] = to_string(Register[stoi(rs1)]);
				}
				else {
					RS[i][1] = RAT[stoi(rs1)];
				}
				if (temp[3][0] == 'F') {
					string rs2 = temp[3].substr(1);
					if (RAT[stoi(rs2)] == "\0")
					{
						RS[i][2] = to_string(Register[stoi(rs2)]);
					}
					else {
						RS[i][2] = RAT[stoi(rs2)];
					}
				}
				else {
					RS[i][2] = temp[3];
				}
				//update RAT
				RAT[stoi(rd)] = "RS" + to_string(i);
				initial_queue.pop();
				return 0;
			}
		}
	}
	else if (which_type(temp) == "mul") {
		for (int i = add_sub_rs_num; i < mul_div_rs_num; i++) {
			if (RS[i][0] == "\0") {
				RS[i][0] = temp[0];
				string operation = RS[i][0];
				string rd = temp[1].substr(1);
				string rs1 = temp[2].substr(1);
				//1. 本身是數字就直接丟進去
				//2. check RAT, if RAT is empty, go to register else put RAT
				//3. update RAT
				if (RAT[stoi(rs1)] == "\0")
				{
					RS[i][1] = to_string(Register[stoi(rs1)]);
				}
				else {
					RS[i][1] = RAT[stoi(rs1)];
				}
				if (temp[3][0] == 'F') {
					string rs2 = temp[3].substr(1);
					if (RAT[stoi(rs2)] == "\0")
					{
						RS[i][2] = to_string(Register[stoi(rs2)]);
					}
					else {
						RS[i][2] = RAT[stoi(rs2)];
					}
				}
				else {
					RS[i][2] = temp[3];
				}
				//update RAT
				RAT[stoi(rd)] = "RS" + to_string(i);
				initial_queue.pop();
				return 0;
			}
		}
	}
	return 1; // is_full
}
void capture() {
	//if buffer is ready, fill the rs !!
	int target = 0;
	int total_num = add_sub_rs_num + mul_div_rs_num;
	if (!add_ALU.is_empty) {
		if (add_ALU.cycle == 0) { //buffer is ready!!
			for (int i = 0; i < total_num; i++) {
				if (RS[i][1][0] == 'R') {
					target = stoi(RS[i][1].substr(2));
					if (add_ALU.RS == target) {
						RS[i][1] = to_string(add_ALU.outcome);
					}
				}
				if (RS[i][2][0] == 'R') {
					target = stoi(RS[i][2].substr(2));
					if (add_ALU.RS == target) {
						RS[i][2] = to_string(add_ALU.outcome);
					}
				}
			}
		}
	}
	if (!mul_ALU.is_empty) {
		if (mul_ALU.cycle == 0) {
			for (int i = 0; i < total_num; i++) {
				if (RS[i][1][0] == 'R') {
					target = stoi(RS[i][1].substr(2));
					if (mul_ALU.RS == target) {
						RS[i][1] = to_string(mul_ALU.outcome);
					}
				}
				if (RS[i][2][0] == 'R') {
					target = stoi(RS[i][2].substr(2));
					if (mul_ALU.RS == target) {
						RS[i][2] = to_string(mul_ALU.outcome);
					}
				}
			}
		}
	}
}
void execute() {
	// Random dispatch 
	// 只要滿足條件就直接丟到ALU運算 (必須是空的)
	// ALU，設定Cycle、運算結果、參考的RS、is_empty設為0
	string operation = "\0";
	if (add_ALU.is_empty) {
		for (int i = 0; i < add_sub_rs_num; i++) {
			if (RS[i][1][0] != 'R' && RS[i][2][0] != 'R' && RS[i][0] != "\0") {
				add_ALU.is_empty = 0; // set not empty
				add_ALU.RS = i;// set RS
				operation = get_operation(RS[i][0]); // set ALU cycle
				if (operation == "+")
					add_ALU.cycle = add_cycle;
				else if (operation == "-")
					add_ALU.cycle = sub_cycle;
			
				add_ALU.outcome = operation_result(RS[i][0], RS[i][1], RS[i][2]);
				break;
			}
		}
	}
	else {
		if (add_ALU.cycle > 0) {
			add_ALU.cycle--;
		}
	}
	if (mul_ALU.is_empty) {
		for (int i = add_sub_rs_num; i < mul_div_rs_num; i++) {
			if (RS[i][1][0] != 'R' && RS[i][2][0] != 'R' && RS[i][0] != "\0") {
				mul_ALU.is_empty = 0; // set not empty
				mul_ALU.RS = i;// set RS
				operation = get_operation(RS[i][0]); // set ALU cycle
				if (operation == "*")
					mul_ALU.cycle = mul_cycle;
				else if (operation == "/")
					mul_ALU.cycle = div_cycle;
				mul_ALU.outcome = operation_result(RS[i][0], RS[i][1], RS[i][2]);
				break;
			}
		}
	}
	else {
		if (mul_ALU.cycle > 0) {
			mul_ALU.cycle--;
		}
	}
}
void write_result() {
	//將結果送回Register RAT對應到的RS
	//更新RAT(清空釋出)
	//更新RS(原RS釋出)
	//清空buffer

	if (!add_ALU.is_empty) {
		if (add_ALU.cycle == 0) { //buffer is ready!!
			for (int i = 0; i < 32; i++) {
				if (RAT[i] == "RS" + to_string(add_ALU.RS)) {
					Register[i] = add_ALU.outcome;
					RAT[i] = "\0";
				}
			}
			capture();
			RS[add_ALU.RS][0] = "\0";
			RS[add_ALU.RS][1] = "\0";
			RS[add_ALU.RS][2] = "\0";
			add_ALU.is_empty = 1;
			add_ALU.outcome = 0;
			add_ALU.RS = 0;
		}
	}
	if (!mul_ALU.is_empty) {
		if (mul_ALU.cycle == 0) { //buffer is ready!!
			for (int i = 0; i < 32; i++) {
				if (RAT[i] == "RS" + to_string(mul_ALU.RS)) {
					Register[i] = mul_ALU.outcome;
					RAT[i] = "\0";
				}
			}
			capture();
			RS[mul_ALU.RS][0] = "\0";
			RS[mul_ALU.RS][1] = "\0";
			RS[mul_ALU.RS][2] = "\0";
			mul_ALU.is_empty = 1;
			mul_ALU.outcome = 0;
			mul_ALU.RS = 0;
		}
	}
}
float operation_result(string operation, string rs1, string rs2) {
	if (operation == "add" || operation == "addi") {
		return stof(rs1) + stof(rs2);
	}
	else if (operation == "sub") {
		return stof(rs1) - stof(rs2);
	}
	else if (operation == "mul") {
		return stof(rs1) * stof(rs2);
	}
	else if (operation == "div") {
		if (stof(rs2) == 0)
		{
			cout << "EXCEPTION!!!" << endl;
			//system("PAUSE");
			exit(3);
		}
		return stof(rs1) / stof(rs2);
	}
}

void print_cycle() {
	cout << "________________________________________________" << endl;
	cout << "Registers status: " << endl<<endl; //Register的狀態都印出來～供使用者參考
	for (int i = 0; i < 32; i++) {
		cout << "R" << i << ": " << Register[i] << "\t\t";
		if (i % 2 != 0)
			cout << endl;
	}
	cout << "________________________________________________" << endl;
	cout << "RAT status: " << endl<<endl; //Register的狀態都印出來～供使用者參考
	for (int i = 0; i < 32; i++) {
		cout << "RAT" << i << ": " << setw(3) << RAT[i] << setw(3);
		if (i % 2 != 0)
			cout << endl;
	}
	cout << "________________________________________________" << endl;
	cout << "(ADD) RS status: " << endl<<endl; //Register的狀態都印出來～供使用者參考
	for (int i = 0; i < add_sub_rs_num; i++) {
		cout << "RS" << i << ": " << setw(3) << RS[i][0] << "|" << setw(3) << RS[i][1] << "|" << setw(3) << RS[i][2] << "\t\t";
		if (i % 2 != 0)
			cout << endl;
	}
	if(add_sub_rs_num%2 !=0)
		cout << endl;
	cout << "________________________________________________" << endl;
	cout << "(ADD)BUFFER: ";
	if (add_ALU.is_empty == 1)
		cout << "empty" << endl;
	else
		cout << "(RS" << add_ALU.RS << ")" << RS[add_ALU.RS][1] << get_operation(RS[add_ALU.RS][0]) << RS[add_ALU.RS][2] << endl;
	cout << "________________________________________________" << endl;
	cout << "(MUL) RS status: "<<endl << endl; //Register的狀態都印出來～供使用者參考
	for (int i = add_sub_rs_num; i < mul_div_rs_num; i++) {
		cout << "RS" << i << ": " <<setw(3)<< RS[i][0] << "|"<<setw(3) << RS[i][1] << "|"<<setw(3) << RS[i][2];
		if (i % 2 != 0)
			cout << endl;
	}
	if(mul_div_rs_num %2 != 0)
		cout << endl;
	cout << "________________________________________________" << endl;
	cout << "(MUL)BUFFER: ";
	if (mul_ALU.is_empty == 1)
		cout << "empty" << endl;
	else
		cout << "(RS" << mul_ALU.RS << ")" << RS[mul_ALU.RS][1]<<get_operation(RS[mul_ALU.RS][0])<< RS[mul_ALU.RS][2] << endl;
	cout << "________________________________________________" << endl;

}