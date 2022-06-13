# 1071710_proj_3
---
## 此次project包含內容如下 
- [x] 程式碼 - 1071710_proj_3_Tomasulo.cpp 
- [x] 輸入輸出範例數組 - Input: input.txt, input2.txt, input3.txt || 輸出範例：1071710_proj_3輸入輸出範例.docx
- [x] 說明文件 - README.md
- [ ] 1071710_proj_3_DEMO影片網址
---
## 程式流程說明
1. 輸入測資檔名，若該檔不存在將會要求使用者重新輸入，若是輸入0則離開程式
2. 各別add/addi/sub與mul/div分別有一個Reserve Station(RS)，依照提示進行輸入設定大小(共兩個RS)
3. 各別輸入add(addi)/sub/mul/div在ALU中運算的cycle數
4. 進入核心流程，對instruction做Issue(若RS滿則不做、若Initial_Queue(IQ)空則不做)、Dispatch(Capture & Execute，採Random dispatch制)、Write Result，當中在ALU運算結果若有Exception會直接顯示錯誤並中止程式
5. 將每個Cycle的Register、RS、RAT、BUFFER印出
6. 不斷進行4與5，直到IQ、RS、ALU皆為Empty時中斷迴圈，回到1
---
## 部分程式碼說明
## 結構與變數說明
```
struct ALU {
	float outcome = 0;
	bool is_empty = 1;
	int cycle = 0;
	int RS = 0;
};
```
**ALU的結構，存放計算結果、是否為空、運行cycle數、以及是哪一個RS的結果紀錄**
'''
float Register[32] = {};
string RAT[32] = {};
'''
**由於課程中的運算結果都有浮點數，故Register使用存放float的陣列；而RAT則是存放string紀錄著要哪一個RS的值
```
queue<vector<string>> initial_queue;
```
**以queue存放instruction，issue時就很好做，並直接命名為initial_queue**
'''
ALU add_ALU;
ALU mul_ALU;
'''
**本程式支援的如同課程上給的Example，有可供add/sub和mul/div運算的兩個ALU**
## 核心Function說明(Issue、Capture、Execute、Write Result)
---
### Issue  
```
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
```
**如註解所示!**
1. 本身是數字就直接丟進去(addi的狀況)
2. check RAT, if RAT is empty, go to register else put RAT
3. update RAT
**兩個add/mul分開區塊處理**
**若RS皆滿了就不會做動作!!**
---
### Capture  
```
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
```
**最核心的概念就是去看ALU裡面有沒有東西，若有的話且Cycle為0，就會去每個RS裡找對應的RS tag並將運算結果填上!!**
---
### Execute  
```
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
```
**以Random的方式做dispatch，只要滿足條件(意即RS tag都填上，都是數字，已經準備好可以開始運算)，就可以丟到"空"以及對應正確型態的ALU進行運算**
**需要設定ALU的cycle、運算結果、RS tag、empty設定為0代表ALU不可進入**
---
### Write Result  
```
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
```
**如註解所示**
1. 將結果送回Register RAT對應到的RS
2. 更新RAT(清空釋出)
3. 更新RS(將結果送到對應的tag以及該RS進行釋出)
4. 清空buffer

