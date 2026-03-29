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

class TwoThreeTree {
 private:
  vector<Record> heap;
 public:
  TwoThreeTree() {
    heap.reserve(1000); //配置1000個Record空間
  }

  void PushBack(Record records_list) {
    for (int i = 0; i < records_list.size(); ++i) {
     heap.push_back(records_list[i]);
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
    while (getline(file, line)) {
      if (line.empty()) continue;
      if (line.back() == '\r') line.pop_back();

      stringstream ss(line);
      string token;
      vector<string> tokens;
      while(getline(ss, token, '\t')) {
          tokens.push_back(token);
      }
        
      while(tokens.size() < 11) tokens.push_back("");

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
    records_list = Record();

    do {
      cout << "\nInput a file number ([0] Quit): ";

      getline(cin, filename);
      if (filename.back() == '\r') filename.pop_back();
    
      if (filename == "0") {
        return true;
      }
    } while(!loadFile(filename, records_list));

    TwoThreeTree.PushBack(records_list);
    return true;
  } else if (cmd == 2) {
    return true;
  }

  return false;
}