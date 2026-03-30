//11227141 鍾博竣
//11327229 游啓揚

#include <iostream>
#include <string>  //字串
#include <sstream>  //檢查輸入
#include <fstream> //read file
#include <vector>
#include <iomanip> //控制輸出格式

using namespace std;

struct Record {
    int seqId;
    string schoolId;
    string schoolName;
    string deptId;
    string deptName;
    string dayNight; 
    string level;    
    int students;
    int teachers;
    int graduates;
    string city;
    string type;

    Record() : seqId(0), students(0), teachers(0), graduates(0) {}
};

struct Node {
    //二維陣列，第一維是放相同的值，第二層是放不同的值，最多三個相同的值，最多三個不同的值
    vector<vector<Record>> keys;
    Node* children[4];
    Node* parent;
    int key_count;

    //initialize the node
    Node() : parent(nullptr), key_count(0) {
      for (int i = 0; i < 4; ++i) {
        children[i] = nullptr;
      }
      keys.resize(3); //最多三個不同的值
    }
};

class TwoThreeTree {
 private:
  Node *root;

  int getHeight(Node* node) {
    if (node == nullptr) return 0;
    return 1 + getHeight(node->children[0]);
  }

  int countNodes(Node* node) {
    if (node == nullptr) return 0;
    int count = 1;
    for (int i = 0; i <= node->key_count; ++i) {
      count += countNodes(node->children[i]);
    }
    return count;
  }

  bool isLeaf(Node* node) {
    // if the first child is null, then it's a leaf node
    return (node->children[0] == nullptr);
  }

  Node* findLeafNode(Record &record) {
    Node* current = root;

    //由於2-3樹往上生長，故不用檢查nullptr，他一定會有兩~三個節點存在
    while (!isLeaf(current)) {
      if (record.students < current->keys[0][0].students) {
        current = current->children[0];
      } else if (record.students == current->keys[0][0].students) {
        current->keys[0].push_back(record);
        return nullptr; //直接存相同的值
      } else if (current->key_count == 1) {
        current = current->children[1];
      } else {
        if (record.students == current->keys[1][0].students) {
          current->keys[1].push_back(record);
          return nullptr; //直接存相同的值
        }
        else if (record.students < current->keys[1][0].students) {
          current = current->children[1];
        } else {
          current = current->children[2];
        }
      }
    }

    // 抵達葉節點時也要檢查是否已經存在相同的值
    if (record.students == current->keys[0][0].students) {
      current->keys[0].push_back(record);
      return nullptr;
    } else if (current->key_count == 2 && record.students == current->keys[1][0].students) {
      current->keys[1].push_back(record);
      return nullptr;
    }

    return current;
  }

  void sortKeys(Node* node, Record &record) {
    if (record.students < node->keys[0][0].students) {
      node->keys[2] = node->keys[1];
      node->keys[1] = node->keys[0];
      node->keys[0].clear();
      node->keys[0].push_back(record);
    } else if (record.students < node->keys[1][0].students) {
      node->keys[2] = node->keys[1];
      node->keys[1].clear();
      node->keys[1].push_back(record);
    } else {
      node->keys[2].clear();
      node->keys[2].push_back(record);
    }
  }

  void splitNode(Node* node) {
    // 1. 新建右側分裂出來的節點
    Node* new_node = new Node();
    new_node->keys[0] = node->keys[2];
    new_node->key_count = 1;

    // 將 node 的第 3 和第 4 個小孩過繼給 new_node (內部節點分裂)
    new_node->children[0] = node->children[2];
    new_node->children[1] = node->children[3];
    if (new_node->children[0] != nullptr) new_node->children[0]->parent = new_node;
    if (new_node->children[1] != nullptr) new_node->children[1]->parent = new_node;

    // 將準備提升的鍵暫存起來
    vector<Record> promote_key = node->keys[1];

    // 清空 node 右半部的值跟小孩
    node->keys[1].clear();
    node->keys[2].clear();
    node->children[2] = nullptr;
    node->children[3] = nullptr;
    node->key_count = 1;

    // 2. 處理父節點
    if (node->parent == nullptr) {
      // 升級成全新的 Root
      Node* new_root = new Node();
      new_root->keys[0] = promote_key;
      new_root->key_count = 1;
      
      new_root->children[0] = node;
      new_root->children[1] = new_node;
      
      node->parent = new_root;
      new_node->parent = new_root;
      root = new_root;
    } else {
      Node* parent_node = node->parent;
      
      // 尋找此 node 是父節點的第幾個小孩
      int child_idx = 0;
      while (child_idx <= parent_node->key_count && parent_node->children[child_idx] != node) {
        child_idx++;
      }

      // 將父節點的 key 和小孩往右移動，騰出空間給新晉升的 key 和 new_node
      for (int i = parent_node->key_count; i > child_idx; i--) {
        parent_node->keys[i] = parent_node->keys[i - 1];
        parent_node->children[i + 1] = parent_node->children[i];
      }

      // 將值與新節點插入父節點
      parent_node->keys[child_idx] = promote_key;
      parent_node->children[child_idx + 1] = new_node;
      new_node->parent = parent_node;
      parent_node->key_count++;

      // 若父節點也滿了（達到 3 個 key），則連鎖觸發分裂
      if (parent_node->key_count == 3) {
        splitNode(parent_node);
      }
    }
  }

  void insertOne(Record &record) {
    //建立 root 節點
    if (root == nullptr) {
      root = new Node();
      root->keys[0].push_back(record);
      root->key_count = 1;
      return;
    }

    /*找到葉子層*/
    Node* probe = findLeafNode(record);

    if (probe == nullptr) {
      return; //已經插入相同的值了
    } else {
      if (probe->key_count == 1) {
        if (record.students < probe->keys[0][0].students) {
          probe->keys[1] = probe->keys[0];
          probe->keys[0].clear();
          probe->keys[0].push_back(record);
        } else {
          probe->keys[1].clear();
          probe->keys[1].push_back(record);
        }
        probe->key_count = 2;
      } else {
        //節點內的值大於兩個，先排序後分裂
        sortKeys(probe, record);
        splitNode(probe);
      }
    }

    return;
  }
 public:
  TwoThreeTree() {
    root = nullptr;
  }

  void insert(vector<Record> records_list) {
    for (int i = 0; i < records_list.size(); ++i) {
      insertOne(records_list[i]);
    }
    return;
  }

  void printTargetNodes() {
    if (root == nullptr) return;
    int height = getHeight(root);
    int nodes = countNodes(root);
    cout << "Tree height = " << height << "\n";
    cout << "Number of nodes = " << nodes << "\n";

    int count = 1;
    for (int i = 0; i < root->key_count; ++i) {
      for (const auto& r : root->keys[i]) {
        cout << count++ << ": [" << r.seqId << "] " 
             << r.schoolName << ", " 
             << r.deptName << ", " 
             << r.dayNight << ", " 
             << r.level << ", " 
             << r.students << ", " 
             << r.graduates << "\n";
      }
    }
  }
};

bool loadFile(const string &filename, vector<Record> &records_list);
bool isInteger(const string &input);
bool processCommand(int cmd);

int main() {
  while (true) {
    cout << "* Data Structures and Algorithms *\n" 
    << "****** Balanced Search Tree ******\n"
    << "* 0. QUIT                        *\n" 
    << "* 1. Build 23 tree               *\n" 
    << "* 2. Build AVL tree              *\n" 
    << "**********************************\n" 
    << "Input a choice(0, 1, 2): ";

    string command;
    getline(cin, command);
    if (!command.empty() && command.back() == '\r') command.pop_back();
    
    if (isInteger(command)) {
      int cmd = stoi(command);
      if (cmd == 0) {
        return 0;
      } else if (!processCommand(cmd)) {
        cout << "\nCommand does not exist!" << endl;
      }
    } else {
      cout << "\nCommand does not exist!" << endl;
    }
    cout << '\n';
  }

  return 0;
}

bool loadFile(const string &filename, vector<Record> &records_list) {
    if (filename.empty()) {
      cout << "\n### " << "input" + filename + ".txt" << " does not exist! ###\n";
      return false;
    }
    string fullPath = "input" + filename + ".txt";
    ifstream file(fullPath);

    if (!file.is_open()) {
      cout << "\n### " << fullPath << " does not exist! ###\n";
      return false;
    }

    string line;
    // Skip 3 header lines
    getline(file, line); 
    getline(file, line);
    getline(file, line);

    int seqBase = 1;
    // Read each line and parse the fields
    while (getline(file, line)) {
      if (line.empty()) continue;
      if (line.back() == '\r') line.pop_back();

      stringstream ss(line);
      string token;
      vector<string> tokens;
      // Split line by tabs
      while(getline(ss, token, '\t')) {
          tokens.push_back(token);
      }
        
      if (tokens.size() < 11) {
        continue; // skip this line if it doesn't have enough fields
      }

      Record r;
      r.seqId = seqBase++;
      r.schoolId = tokens[0];
      r.schoolName = tokens[1];
      r.deptId = tokens[2];
      r.deptName = tokens[3];
      r.dayNight = tokens[4];
      r.level = tokens[5];
      r.students = stoi(tokens[6]);
      r.teachers = stoi(tokens[7]);
      r.graduates = stoi(tokens[8]);
      r.city = tokens[9];
      r.type = tokens[10];
        
      records_list.push_back(r);
    }
    file.close();
    return true;
}

bool isInteger(const string &input) {
  if (input.empty()) return false;

  stringstream temp(input);
  int number;
  char c;
  if (temp >> number && !(temp >> c)) {
    return true;
  }
  return false;
}

bool processCommand(int cmd) {
  string filename {};
  static vector<Record> records_list;

  if (cmd == 1) {
    TwoThreeTree tree;
    records_list.clear();

    do {
      cout << "\nInput a file number ([0] Quit): ";

      getline(cin, filename);
      if (filename.back() == '\r') filename.pop_back();
    
      if (filename == "0") {
        return true;
      }
    } while(!loadFile(filename, records_list));

    tree.insert(records_list);
    tree.printTargetNodes();
    return true;
  } else if (cmd == 2) {
    return true;
  }

  return false;
}