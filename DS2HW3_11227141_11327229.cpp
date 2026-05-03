// 11227141 鍾博竣
// 11327229 游啓揚

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// 定義學生資料結構，以利讀取與寫入二進位檔 (任務零)
// 確保大小嚴格對齊以符合題目「檔案大小必須一致」的要求
#pragma pack(1)
struct Student {
  char sid[10];           // 學號，10個字元
  char sname[10];         // 姓名，10個字元
  unsigned char score[6]; // 6個分數欄位，儲存為整數
  float average;          // 平均分數，儲存為浮點數
};
#pragma pack()

// 定義雜湊表內的節點結構
struct HashNode {
  int hvalue;   // 儲存雜湊值
  string sid;   // 儲存學號字串
  string sname; // 儲存姓名字串
  float mean;   // 儲存平均分數
  bool isEmpty; // 標記該位址是否為空位

  // 初始化建構子，預設為空位，雜湊值初始化為-1
  HashNode() : hvalue(-1), sid(""), sname(""), mean(0.0f), isEmpty(true) {}
};

// 將相關函式與資料整合為一個 HashTableManager 類別
class HashTableManager {
private:
  vector<Student>
      dataList; // 儲存從檔案讀取的資料，使用 vector 滿足動態陣列規範

  // 判斷一個整數 n 是否為質數
  bool isPrime(int n) {
    if (n <= 1)
      return false; // 1以下不是質數
    if (n <= 3)
      return true; // 2 和 3 是質數
    if (n % 2 == 0 || n % 3 == 0)
      return false; // 排除 2 和 3 的倍數

    // 使用 6k ± 1 原則來檢查質數，以提升檢查效率
    for (int i = 5; i * i <= n; i += 6) {
      if (n % i == 0 || n % (i + 2) == 0)
        return false;
    }
    return true; // 若都無法整除，則為質數
  }

  // 尋找大於 n 的最小質數
  int getSmallestPrime(int n) {
    int prime = n + 1; // 從 n 的下一個整數開始尋找
    while (!isPrime(prime)) {
      prime++; // 不斷遞增直到找到質數為止
    }
    return prime; // 回傳找到的最小質數
  }

  // 計算學號每個字元 ASCII 編碼的乘積
  unsigned long long getAsciiProduct(const string &sid) {
    unsigned long long product = 1; // 初始化乘積為 1
    for (char c : sid) {
      // 過濾可能的空字元或其他非預期結尾符號，確保只乘上有效字元
      if (c != '\0' && c != '\r' && c != '\n') {
        product *= (unsigned long long)(unsigned char)c; // 轉為無號數避免溢位成為負數
      }
    }
    return product; // 回傳算出的 ASCII 乘積
  }

public:
  // 清空現有讀取的資料，供切換檔案時使用
  void clearData() {
    dataList.clear(); // 清空 vector 內容
  }

  // 取得目前讀取的資料筆數
  int getDataSize() const {
    return dataList.size(); // 回傳 vector 大小
  }

  // 任務零：讀取 txt 轉 bin，若 bin 已存在則直接讀取 bin
  bool loadFile(const string &fileNum) {
    string binName = "input" + fileNum + ".bin"; // 組合出二進位檔名
    string txtName = "input" + fileNum + ".txt"; // 組合出文字檔名

    // 第一步：優先嘗試開啟 .bin 二進位檔案
    ifstream binFile(binName, ios::binary);
    if (binFile.is_open()) {
      Student temp;
      // 逐筆讀取結構大小的資料，直到檔尾
      while (binFile.read((char *)&temp, sizeof(Student))) {
        dataList.push_back(temp); // 將讀到的資料放入記憶體
      }
      binFile.close(); // 關閉檔案
      return true;     // 回傳讀取成功
    }

    // 若 .bin 不存在，根據 DEMO 必須顯示不存在的錯誤訊息
    cout << "\n### " << binName << " does not exist! ###\n";

    // 第二步：嘗試讀取 .txt 文字檔進行轉換
    ifstream txtFile(txtName);
    if (!txtFile.is_open()) {
      cout << "\n### " << txtName << " does not exist! ###\n\n";
      return false; // 若連 txt 都不存在則回傳讀取失敗
    }

    // 準備將讀取的 txt 寫入全新的 bin 檔
    ofstream outBin(binName, ios::binary);
    string line; // 用來暫存每一行文字

    // 逐行讀取 txt 檔
    while (getline(txtFile, line)) {
      if (line.empty() || line == "\r")
        continue; // 略過空行或純換行

      stringstream ss(line); // 將字串轉為字串串流以利分割
      string token;          // 用來暫存被 tab 切割出來的字串
      Student stu = {};      // 初始化學生結構並將所有欄位歸零

      // 讀取學號欄位
      if (getline(ss, token, '\t')) {
        for (int i = 0; i < 10 && i < token.length(); ++i) {
          stu.sid[i] = token[i]; // 逐字元複製到結構中
        }
      }
      // 讀取姓名欄位
      if (getline(ss, token, '\t')) {
        for (int i = 0; i < 10 && i < token.length(); ++i) {
          stu.sname[i] = token[i]; // 逐字元複製到結構中
        }
      }
      // 讀取 6 個分數欄位
      for (int i = 0; i < 6; ++i) {
        if (getline(ss, token, '\t')) {
          try {
            stu.score[i] = (unsigned char)stoi(token); // 轉型成無號字元
          } catch (...) {
            stu.score[i] = 0; // 防呆：若該欄位損毀或為空，則設為0避免拋出異常(RE)
          }
        }
      }
      // 讀取最後的平均分數欄位
      if (getline(ss, token, '\t')) {
        try {
          stu.average = stof(token); // 轉型成浮點數
        } catch (...) {
          stu.average = 0.0f;
        }
      }

      // 將此筆結構資料寫入二進位檔
      outBin.write((char *)&stu, sizeof(Student));
      // 同時將此筆資料推入記憶體的 vector 中，供後續任務使用
      dataList.push_back(stu);
    }

    txtFile.close(); // 關閉文字檔
    outBin.close();  // 關閉二進位檔
    return true;     // 回傳讀取成功
  }

  // 任務一：以平方探測 (Quadratic Probing) 建立雜湊表 X
  void runQuadraticProbing(const string &fileNum) {
    if (dataList.empty())
      return; // 防呆機制：若無資料則直接返回

    int totalData = dataList.size(); // 取得資料總筆數
    // 雜湊表大小：大於 1.15 倍資料總筆數的最小質數
    int tableSize = getSmallestPrime((int)(1.15 * totalData));

    // 初始化雜湊表動態陣列，預設所有節點均為 isEmpty = true
    vector<HashNode> hashTable(tableSize);
    int insertedCount = 0;              // 記錄成功插入的資料筆數
    int totalSuccessfulComparisons = 0; // 記錄成功搜尋的總比較次數

    // 開始將每一筆資料依序插入雜湊表
    for (const auto &stu : dataList) {
      string sidStr(stu.sid, 8); // 老師指定學號為固定八位數，強制截斷至8位以忽略最後兩個位元的垃圾值
      sidStr = sidStr.c_str();   // 安全截斷至第一個 \0（若有短於8碼的防呆）

      unsigned long long asciiProduct = getAsciiProduct(sidStr); // 計算學號 ASCII 乘積
      int baseHash = asciiProduct %
                     tableSize; // 基本雜湊函數：(ASCII編碼相乘) % 雜湊表大小

      bool inserted = false; // 標記是否成功插入
      int comparisons = 0;   // 記錄單筆資料插入過程的比較次數

      // 平方探測處理碰撞：(hash + i^2) % tableSize
      for (int i = 0; i < tableSize; ++i) {
        comparisons++; // 每次探測位址，視為一次比較
        int probeHash = (baseHash + (long long)i * i) % tableSize; // 轉 long long 防止 i*i 溢位為負數造成RE

        // 若找到空位，則將資料放入該位址
        if (hashTable[probeHash].isEmpty) {
          hashTable[probeHash].hvalue = baseHash;        // 記錄原始的雜湊值
          hashTable[probeHash].sid = sidStr;             // 記錄學號
          string snameStr(stu.sname, 10);                // 指定長度防止越界
          snameStr = snameStr.c_str();                   // 安全截斷
          hashTable[probeHash].sname = snameStr;         // 記錄姓名
          hashTable[probeHash].mean = stu.average;       // 記錄平均分數
          hashTable[probeHash].isEmpty = false;          // 標記該位址已被占用

          inserted = true; // 標記為成功插入
          totalSuccessfulComparisons +=
              comparisons; // 將此次比較的次數累加到成功總次數中
          insertedCount++; // 成功插入筆數加一
          break;           // 已經插入，跳出碰撞處理迴圈
        }
      }

      // 若走訪了 tableSize
      // 次都找不到空位，則顯示無法加入的錯誤訊息並忽略該筆資料
      if (!inserted) {
        cout << "Cannot insert student ID: " << sidStr << endl;
      }
    }

    // 計算搜尋現存值 (successful search) 的平均比較次數
    double avgSuccessful =
        (insertedCount > 0) ? (double)totalSuccessfulComparisons / insertedCount
                            : 0.0;

    // 計算搜尋不存在值 (unsuccessful search) 的平均比較次數
    int totalUnsuccessfulComparisons = 0; // 記錄不成功搜尋的總比較次數
    // 對於每一個可能的 hash 值 k (從 0 走訪到 tableSize-1)
    for (int k = 0; k < tableSize; ++k) {
      int comparisons = 0; // 記錄每個 k 的比較次數
      // 模擬平方探測尋找空位，直到遇到空位或達次數上限
      for (int i = 0; i < tableSize; ++i) {
        int probeHash = (k + (long long)i * i) % tableSize; // 防溢位
        if (hashTable[probeHash].isEmpty) {
          break; // 若遇到空位，代表找不到資料，搜尋結束。不計入金鑰比較次數！
        }
        comparisons++; // 探測到已占用位址，需進行金鑰比較，次數加一
      }
      totalUnsuccessfulComparisons += comparisons; // 累加至總次數
    }
    // 平均不存在比較次數 = 總次數除以雜湊表大小
    double avgUnsuccessful = (double)totalUnsuccessfulComparisons / tableSize;

    // 準備將結果寫出至 quadraticXXX.txt 檔案
    string outName = "quadratic" + fileNum + ".txt"; // 組合輸出檔名
    ofstream outFile(outName);                       // 開啟檔案進行寫入
    outFile << " --- Hash table created by Quadratic probing ---"
            << endl; // 寫入標題
    for (int i = 0; i < tableSize; ++i) {
      outFile << "[" << setw(3) << right << i
              << "] "; // 輸出索引值，寬度為3，靠右對齊
      if (!hashTable[i].isEmpty) {
        // 如果位址不為空，則依序印出雜湊值、學號、姓名、平均分數
        // (寬度皆設定為10並靠右)
        outFile << setw(10) << right << hashTable[i].hvalue << ", " << setw(10)
                << right << hashTable[i].sid << ", " << setw(10) << right
                << hashTable[i].sname << ", " << setw(10) << right
                << hashTable[i].mean;
      }
      outFile << endl; // 每一筆佔一行
    }
    outFile << " ----------------------------------------------------- "
            << endl;
    outFile.close(); // 關閉檔案

    // 在螢幕上輸出成功建立的訊息，並顯示兩種搜尋的平均比較次數
    // (精準至小數點後四位)
    cout << "\nHash table has been successfully created by Quadratic probing\n";
    cout << "unsuccessful search: " << fixed << setprecision(4)
         << avgUnsuccessful << " comparisons on average\n";
    cout << "successful search: " << fixed << setprecision(4) << avgSuccessful
         << " comparisons on average\n";
  }

  // 任務二：以雙重雜湊 (Double Hashing) 建立雜湊表 Y
  void runDoubleHashing(const string &fileNum) {
    if (dataList.empty())
      return; // 防呆機制：若無記憶體內資料則不執行

    int totalData = dataList.size(); // 取得現有資料筆數
    // 雜湊表大小：計算方式與任務一相同
    int tableSize = getSmallestPrime((int)(1.15 * totalData));
    // 最高步階：大於(資料總筆數/5)的最小質數
    int maxStep = getSmallestPrime(totalData / 5);

    // 初始化雜湊表動態陣列，預設為空位
    vector<HashNode> hashTable(tableSize);
    int insertedCount = 0;              // 記錄成功插入筆數
    int totalSuccessfulComparisons = 0; // 記錄成功搜尋總比較次數

    // 開始將每一筆資料依序插入雜湊表
    for (const auto &stu : dataList) {
      string sidStr(stu.sid, 8); // 老師指定學號為固定八位數，忽略最後兩個位元的垃圾值
      sidStr = sidStr.c_str();   // 截斷至 \0

      unsigned long long asciiProduct = getAsciiProduct(sidStr); // 計算學號 ASCII 乘積
      int baseHash = asciiProduct % tableSize; // 基礎雜湊函數 (與任務一相同)

      // 步階函數：最高步階 - (ASCII相乘結果 % 最高步階)
      int step = maxStep - (asciiProduct % maxStep);

      bool inserted = false; // 標記是否成功插入
      int comparisons = 0;   // 單次插入的比較次數

      // 雙重雜湊處理碰撞：(hash + i * step) % tableSize
      for (int i = 0; i < tableSize; ++i) {
        comparisons++; // 每次探測即增加一次比較
        int probeHash = (baseHash + (long long)i * step) % tableSize; // 防止溢位

        // 找到空位則將資料填入
        if (hashTable[probeHash].isEmpty) {
          hashTable[probeHash].hvalue = baseHash;        // 記錄基礎雜湊值
          hashTable[probeHash].sid = sidStr;             // 記錄學號
          string snameStr(stu.sname, 10);                // 指定長度防止越界
          snameStr = snameStr.c_str();                   // 安全截斷
          hashTable[probeHash].sname = snameStr;         // 記錄姓名
          hashTable[probeHash].mean = stu.average;       // 記錄平均分數
          hashTable[probeHash].isEmpty = false;          // 標記為已被占用

          inserted = true;                           // 標記成功插入
          totalSuccessfulComparisons += comparisons; // 累加成功比較的次數
          insertedCount++;                           // 成功插入筆數加一
          break;                                     // 插入後跳出碰撞迴圈
        }
      }

      // 若所有探測均被占用，則顯示無法加入訊息並忽略資料
      if (!inserted) {
        cout << "Cannot insert student ID: " << sidStr << endl;
      }
    }

    // 計算搜尋現存值 (successful search) 的平均比較次數
    double avgSuccessful =
        (insertedCount > 0) ? (double)totalSuccessfulComparisons / insertedCount
                            : 0.0;

    // 準備將結果寫出至 doubleXXX.txt 檔案
    string outName = "double" + fileNum + ".txt"; // 組合輸出檔名
    ofstream outFile(outName);                    // 開啟寫入檔案

    // 寫入標題 (注意尾部留有空格，以和任務一對齊長度)
    outFile << " --- Hash table created by Double hashing    ---" << endl;
    for (int i = 0; i < tableSize; ++i) {
      outFile << "[" << setw(3) << right << i << "] "; // 印出索引，寬度3靠右
      if (!hashTable[i].isEmpty) {
        // 如果位址有資料，印出欄位內容並對齊
        outFile << setw(10) << right << hashTable[i].hvalue << ", " << setw(10)
                << right << hashTable[i].sid << ", " << setw(10) << right
                << hashTable[i].sname << ", " << setw(10) << right
                << hashTable[i].mean;
      }
      outFile << endl; // 每筆資料換行
    }
    outFile << " ----------------------------------------------------- "
            << endl;
    outFile.close(); // 關閉檔案

    // 在螢幕輸出成功建立訊息，以及現存搜尋的平均比較次數 (精準至小數點後四位)
    cout << "\nHash table has been successfully created by Double hashing   \n";
    cout << "successful search: " << fixed << setprecision(4) << avgSuccessful
         << " comparisons on average\n";
  }
};

// 檢查輸入字串是否為純數字組成的有效整數 (防呆機制)
bool isInteger(const string &input) {
  if (input.empty())
    return false;           // 空字串直接回傳否
  stringstream temp(input); // 利用字串串流進行解析
  int number;
  char c;
  // 必須完全由數字構成，且不能有其他殘留字元 (如英文字母)
  if (temp >> number && !(temp >> c)) {
    return true; // 是合法的整數
  }
  return false; // 不是合法的整數
}

int main() {
  HashTableManager manager;   // 實例化雜湊表管理類別
  bool isTask1Done = false;   // 用來記錄任務一是否已成功執行過，供任務二檢查
  string currentFileNum = ""; // 儲存任務一成功讀取時的檔案編號，讓任務二沿用

  while (true) {
    // 印出主選單介面
    cout << "* Data Structures and Algorithms *\n"
         << "************ Hash Table **********\n"
         << "* 0. QUIT                        *\n"
         << "* 1. Quadratic probing           *\n"
         << "* 2. Double hashing              *\n"
         << "**********************************\n"
         << "Input a choice(0, 1, 2): ";

    string command;
    if (!getline(cin, command)) {
      break; // 若讀到 EOF，結束程式避免無限迴圈
    }

    // 處理可能因跨平台系統產生的 \r 換行字元
    if (!command.empty() && command.back() == '\r') {
      command.pop_back();
    }

    // 呼叫防呆機制驗證輸入是否為整數
    if (isInteger(command)) {
      int cmd = stoi(command); // 轉型成整數

      if (cmd == 0) {
        break; // 如果使用者輸入 0，則跳出迴圈結束程式

      } else if (cmd == 1) {
        // 任務一：要求使用者輸入檔案編號
        cout << "\nInput a file number ([0] Quit): ";
        string fileNum;
        if (!getline(cin, fileNum)) {
          break; // 若讀到 EOF，結束程式
        }

        // 去除可能的多餘換行字元
        if (!fileNum.empty() && fileNum.back() == '\r') {
          fileNum.pop_back();
        }

        if (fileNum == "0") {
          cout << '\n';
          continue; // 選擇 0 則退回主選單
        }

        manager.clearData(); // 先清空前次可能殘留的記憶體資料

        // 呼叫 loadFile 進行讀檔 (任務零自動判斷並處理 txt 或 bin)
        if (manager.loadFile(fileNum)) {
          // 若讀檔成功，開始執行任務一：平方探測
          manager.runQuadraticProbing(fileNum);
          isTask1Done = true;       // 標記任務一已成功完成
          currentFileNum = fileNum; // 儲存這個檔案編號，讓任務二使用
        }

      } else if (cmd == 2) {
        // 任務二：執行雙重雜湊
        if (!isTask1Done) {
          // 防呆：必須先成功執行過任務一，否則顯示錯誤並拒絕執行
          cout << "### Command 1 first. ###\n\n";
        } else {
          // 呼叫任務二函式，並沿用任務一所儲存的檔案編號
          manager.runDoubleHashing(currentFileNum);
        }

      } else {
        // 如果輸入的整數不為 0, 1, 2，顯示找不到指令
        cout << "\nCommand does not exist!\n\n";
      }
    } else {
      // 如果輸入的根本不是整數，顯示找不到指令
      cout << "\nCommand does not exist!\n\n";
    }
    cout << '\n'; // 每次執行完指令後多空一行，與 DEMO 畫面保持一致
  }

  return 0; // 程式正常結束
}
