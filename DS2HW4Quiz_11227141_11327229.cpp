// 11227141 鍾博竣
// 11327229 游啓揚

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

using namespace std;

// 為了符合二進位檔結構，嚴格對齊 1 byte，避免自動補齊造成讀取錯誤
#pragma pack(1)
struct PairRecord {
  char putID[12]; // 發訊者學號，12 個字元的陣列
  char getID[12]; // 收訊者學號，12 個字元的陣列
  float weight;   // 訊息量權重，浮點數
};
#pragma pack()

// 相鄰串列中的節點 (代表一條有向邊的終點與權重)
struct AdjNode {
  string getID; // 收訊者學號
  float weight; // 訊息權重

  // 多載小於運算子，供排序使用：依【收訊者學號】由小到大排序
  bool operator<(const AdjNode &other) const { return getID < other.getID; }
};

// 相鄰串列的主陣列節點 (代表一個學生/發訊者)
struct HeadNode {
  string putID;         // 學生學號 (發訊者或純收訊者)
  vector<AdjNode> list; // 對應的相鄰串列 (採用 vector 實作動態陣列)

  // 多載小於運算子，供主陣列排序使用：依【學號】由小到大排序
  bool operator<(const HeadNode &other) const { return putID < other.putID; }
};

// 任務二用來儲存與排序連通數結果的結構
struct CntNode {
  string putID;                // 學生學號
  int count;                   // 連通數 (可走訪到的相異收訊者總數)
  vector<string> connectedIDs; // 紀錄走訪過的所有相異收訊者學號

  // 排序規則：連通數由大到小排序；若相同，則依學號由小到大排序
  bool operator<(const CntNode &other) const {
    if (count != other.count) {
      return count > other.count; // 連通數大者排前面 (DESC)
    }
    return putID < other.putID; // 學號小者排前面 (ASC)
  }
};

// 負責圖形資料操作的類別
class GraphManager {
private:
  vector<HeadNode> graph; // 相鄰串列的主陣列 (Graph)
  int totalNodesCount;    // 總節點(邊)數

  // 輔助函式：將 char 陣列轉為 C++ string，並安全截除多餘的 null 字元
  string charToString(const char *arr, int len) {
    string str(arr, len);
    size_t nullPos = str.find('\0'); // 尋找字串結束字元
    if (nullPos != string::npos) {
      str = str.substr(0, nullPos); // 截斷
    }
    return str;
  }

  // 輔助函式：利用二分搜尋法 (Binary Search) 在已排序的主陣列中找尋學號索引
  int getIndex(const string &id) const {
    int left = 0;
    int right = (int)graph.size() - 1;
    while (left <= right) {
      int mid = left + (right - left) / 2;
      if (graph[mid].putID == id)
        return mid; // 找到對應索引
      if (graph[mid].putID < id)
        left = mid + 1;
      else
        right = mid - 1;
    }
    return -1; // 找不到 (理應不會發生，因為有收集所有 ID)
  }

public:
  GraphManager() : totalNodesCount(0) {}

  // 清空現有圖形資料，供重新讀取檔案時使用
  void clearGraph() {
    graph.clear();
    totalNodesCount = 0;
  }

  // 判斷是否已經有圖形資料 (任務一是否成功執行)
  bool hasGraph() const { return !graph.empty(); }

  // 任務一：建立相鄰串列 (Build adjacency lists)
  bool buildAdjacencyLists(const string &fileNum) {
    string binName = "pairs" + fileNum + ".bin"; // 組合出二進位檔名
    ifstream inFile(binName, ios::binary);       // 以二進位模式開啟檔案

    if (!inFile.is_open()) {
      cout << "\n### " << binName << " does not exist! ###\n";
      return false;
    }

    // 為了提升效率，先暫存所有的邊 (edges) 以及蒐集所有獨立學號
    struct Edge {
      string u, v;
      float w;
    };
    vector<Edge> edges;
    vector<string> uniqueIDs; // 收集所有的學號

    PairRecord record;
    totalNodesCount = 0; // 重置邊數計數器

    // 逐筆讀取 28 bytes 直到檔尾
    while (inFile.read((char *)&record, sizeof(PairRecord))) {
      string u = charToString(record.putID, 12);
      string v = charToString(record.getID, 12);
      if (u.empty() || v.empty())
        continue; // 防呆：略過空資料

      edges.push_back({u, v, record.weight}); // 儲存該筆互動紀錄
      uniqueIDs.push_back(u);                 // 加入發訊者學號
      uniqueIDs.push_back(v);                 // 加入收訊者學號
      totalNodesCount++;                      // 總邊數 (節點) 遞增
    }
    inFile.close(); // 關閉讀取完成的二進位檔

    // 針對 uniqueIDs 進行排序並移除重複元素，獲得獨立的學號清單
    sort(uniqueIDs.begin(), uniqueIDs.end());
    uniqueIDs.erase(unique(uniqueIDs.begin(), uniqueIDs.end()),
                    uniqueIDs.end());

    // 初始化主陣列大小，確保每一個學號都有一個對應的 HeadNode
    graph.resize(uniqueIDs.size());
    for (size_t i = 0; i < uniqueIDs.size(); ++i) {
      graph[i].putID = uniqueIDs[i]; // 學號本身已經是排序狀態
    }

    // 將先前記下的每一條邊，加入對應的發訊者串列中
    for (const auto &edge : edges) {
      int uIndex = getIndex(edge.u); // 以二分搜尋找發訊者索引 (O(logV))
      if (uIndex != -1) {
        graph[uIndex].list.push_back({edge.v, edge.w});
      }
    }

    // 針對每一條相鄰串列內部，依「收訊者學號」由小到大排序
    for (auto &head : graph) {
      sort(head.list.begin(), head.list.end());
    }

    // 輸出至 .adj 檔案
    string outName = "pairs" + fileNum + ".adj";
    ofstream outFile(outName);

    // 依格式寫入總 ID 數
    outFile << "<<< There are " << graph.size() << " IDs in total. >>>\n";
    for (size_t i = 0; i < graph.size(); ++i) {
      // 寫入主陣列的學號，索引寬度為 3 並靠右
      outFile << "[" << setw(3) << right << (i + 1) << "] " << graph[i].putID
              << ": \n";
      // 寫入串列中的收訊者資訊
      for (size_t j = 0; j < graph[i].list.size(); ++j) {
        if (j % 12 == 0)
          outFile << "\t";

        outFile << "(" << setw(2) << right << (j + 1) << ") "
                << graph[i].list[j].getID << ", " << setw(6)
                << graph[i].list[j].weight;

        if (j % 12 == 11) {
          outFile << "\n";
        } else if (j != graph[i].list.size() - 1) {
          outFile << "\t";
        }
      }
      if (!graph[i].list.empty()) {
        outFile << "\n";
      }
    }
    outFile << "<<< There are " << totalNodesCount << " nodes in total. >>>\n";
    outFile.close(); // 關閉輸出檔案

    // 在螢幕顯示成功建立的資訊
    cout << "\n<<< There are " << graph.size() << " IDs in total. >>>\n\n";
    cout << "<<< There are " << totalNodesCount << " nodes in total. >>>\n";

    return true; // 回傳建立成功
  }

  // 任務二：計算連通數 (Compute connection counts)
  void computeConnectionCounts(const string &fileNum) {
    if (graph.empty())
      return; // 防呆：無圖形時不執行

    vector<CntNode> results(graph.size()); // 宣告儲存連通數計算結果的陣列

    // 為每一個學號執行 BFS 寬度優先走訪
    for (size_t i = 0; i < graph.size(); ++i) {
      results[i].putID = graph[i].putID; // 記錄發訊者學號
      results[i].count = 0;              // 初始化連通數為 0

      vector<bool> visited(graph.size(), false); // 走訪標記，初始皆為未走訪
      queue<int> q;                              // BFS 用的佇列

      q.push(i);         // 起點加入佇列
      visited[i] = true; // 標記起點為已走訪 (但起點不計入連通數)

      // 開始 BFS 走訪
      while (!q.empty()) {
        int u = q.front();
        q.pop();

        // 巡迴該點的每一條邊
        for (const auto &edge : graph[u].list) {
          int v = getIndex(edge.getID); // 取得相鄰點在主陣列的索引
          if (v != -1 && !visited[v]) {
            visited[v] = true;                             // 標記為已走訪
            results[i].connectedIDs.push_back(edge.getID); // 紀錄此相異收訊者
            results[i].count++;                            // 連通數遞增
            q.push(v); // 將該相鄰點加入佇列繼續走訪
          }
        }
      }
      // 將走訪過的所有收訊者學號，依字串由小到大排序
      sort(results[i].connectedIDs.begin(), results[i].connectedIDs.end());
    }

    // 依照連通數大到小排序，若相同則學號小到大 (利用多載的運算子)
    sort(results.begin(), results.end());

    // 輸出至 .cnt 檔案
    string outName = "pairs" + fileNum + ".cnt";
    ofstream outFile(outName);

    // 依格式寫入總 ID 數
    outFile << "<<< There are " << results.size() << " IDs in total. >>>\n";
    for (size_t i = 0; i < results.size(); ++i) {
      // 寫入發訊者學號及連通數
      outFile << "[" << setw(3) << right << (i + 1) << "] " << results[i].putID
              << "(" << results[i].count << "): \n";

      // 依序寫入走訪到的收訊者學號
      for (size_t j = 0; j < results[i].connectedIDs.size(); ++j) {
        // 每列開頭的縮排
        if (j % 12 == 0)
          outFile << "\t";
        // 序號寬度為 2 並靠右，學號直接印出
        outFile << "(" << setw(2) << right << (j + 1) << ") "
                << results[i].connectedIDs[j];

        if (j % 12 == 11) {
          outFile << "\n";
        } else if (j != results[i].connectedIDs.size() - 1) {
          outFile << "\t"; // 元素間的間隔
        }
      }
      if (!results[i].connectedIDs.empty()) {
        outFile << "\n";
      }
    }
    outFile.close(); // 關閉輸出檔案

    // 在螢幕顯示成功計算的資訊
    cout << "\n<<< There are " << results.size() << " IDs in total. >>>\n";
  }
};

int main() {
  GraphManager manager;       // 實例化圖形管理類別
  string currentFileNum = ""; // 用來記錄成功執行的檔案編號

  while (true) {
    // 印出主選單介面，依照 DEMO 指定第一列與標題
    cout << "* Data Structures and Algorithms *\n"
         << "**** Graph data manipulation *****\n"
         << "* 0. QUIT                        *\n"
         << "* 1. Build adjacency lists       *\n"
         << "* 2. Compute connection counts   *\n"
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

    // 嚴格比對字串，避免 "00" 等非預期輸入被當作合法指令
    if (command == "0") {
      break; // 如果使用者輸入 0，則跳出迴圈結束程式

    } else if (command == "1") {
      manager
          .clearGraph(); // 先清空前次可能殘留的圖形資料，只要選擇 1 就立刻清空

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
        cout << "\n";
        continue; // 選擇 0 則退回主選單
      }

      // 呼叫 buildAdjacencyLists 讀取二進位檔並建立相鄰串列
      if (manager.buildAdjacencyLists(fileNum)) {
        currentFileNum = fileNum; // 若成功，則記錄這個檔案編號，供任務二使用
      }

    } else if (command == "2") {
      // 任務二：執行計算連通數
      if (!manager.hasGraph()) {
        // 防呆：必須先有圖形資料存在
        // (先成功執行過任務一)，否則顯示錯誤並拒絕執行
        cout << "### There is no graph and choose 1 first. ###\n";
      } else {
        // 呼叫計算連通數的函式
        manager.computeConnectionCounts(currentFileNum);
      }

    } else {
      // 如果輸入不為 "0", "1", "2"，顯示找不到指令
      cout << "\nCommand does not exist!\n";
    }
    cout
        << '\n'; // 每次執行完指令後多空一行，維持排版整潔並與 DEMO 畫面保持一致
  }

  return 0; // 程式正常結束
}
