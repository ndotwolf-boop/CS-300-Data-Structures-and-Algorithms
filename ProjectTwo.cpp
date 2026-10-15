// CS 300 Project Two - ABCU Advising Assistance Program
// Uses a binary search tree to store courses by course number.
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <cctype>
using namespace std;

struct Course {
    string number;
    string title;
    vector<string> prerequisites;
};

struct Node {
    Course course;
    Node* left;
    Node* right;
    explicit Node(const Course& value) : course(value), left(nullptr), right(nullptr) {}
};

string trim(const string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

string upper(string value) {
    transform(value.begin(), value.end(), value.begin(),
              [](unsigned char ch) { return static_cast<char>(toupper(ch)); });
    return value;
}

class CourseTree {
private:
    Node* root = nullptr;

    static void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    static void printInOrder(const Node* node) {
        if (!node) return;
        printInOrder(node->left);
        cout << node->course.number << ", " << node->course.title << '\n';
        printInOrder(node->right);
    }

public:
    CourseTree() = default;
    ~CourseTree() { clear(); }
    CourseTree(const CourseTree&) = delete;
    CourseTree& operator=(const CourseTree&) = delete;

    void clear() { destroy(root); root = nullptr; }

    bool insert(const Course& course) {
        Node** current = &root;
        while (*current) {
            if (course.number == (*current)->course.number) return false;
            current = course.number < (*current)->course.number
                ? &(*current)->left : &(*current)->right;
        }
        *current = new Node(course);
        return true;
    }

    const Course* find(const string& number) const {
        const Node* current = root;
        while (current) {
            if (number == current->course.number) return &current->course;
            current = number < current->course.number ? current->left : current->right;
        }
        return nullptr;
    }

    void printList() const { printInOrder(root); }
};

// Read and validate all lines before changing the current course tree.
// Each CSV row contains course number, title, then zero or more prerequisites.
bool loadCourses(const string& fileName, CourseTree& tree) {
    ifstream input(fileName);
    if (!input.is_open()) {
        cout << "Error: Unable to open course file.\n";
        return false;
    }

    vector<Course> parsed;
    unordered_set<string> courseNumbers;
    string line;
    size_t lineNumber = 0;

    while (getline(input, line)) {
        ++lineNumber;
        if (trim(line).empty()) continue;

        stringstream stream(line);
        vector<string> fields;
        string field;
        while (getline(stream, field, ',')) fields.push_back(trim(field));
        if (!line.empty() && line.back() == ',') fields.push_back("");

        if (fields.size() < 2 || fields[0].empty() || fields[1].empty()) {
            cout << "Error: Invalid course data on line " << lineNumber << ".\n";
            return false;
        }
        string number = upper(fields[0]);
        if (!courseNumbers.insert(number).second) {
            cout << "Error: Duplicate course number " << number << ".\n";
            return false;
        }

        Course course;
        course.number = number;
        course.title = fields[1];
        for (size_t i = 2; i < fields.size(); ++i) {
            if (fields[i].empty()) {
                cout << "Error: Empty prerequisite on line " << lineNumber << ".\n";
                return false;
            }
            course.prerequisites.push_back(upper(fields[i]));
        }
        parsed.push_back(course);
    }

    if (parsed.empty()) {
        cout << "Error: Course file contains no courses.\n";
        return false;
    }

    for (const Course& course : parsed) {
        for (const string& prerequisite : course.prerequisites) {
            if (courseNumbers.count(prerequisite) == 0) {
                cout << "Error: Prerequisite " << prerequisite
                     << " for " << course.number << " was not found.\n";
                return false;
            }
        }
    }

    // Validation succeeded: replace the previous tree with the new course list.
    tree.clear();
    for (const Course& course : parsed) tree.insert(course);
    cout << "Course data successfully loaded (" << parsed.size() << " courses).\n";
    return true;
}

void printCourseInformation(const CourseTree& tree, string courseNumber) {
    courseNumber = upper(trim(courseNumber));
    const Course* course = tree.find(courseNumber);
    if (!course) {
        cout << "Course not found.\n";
        return;
    }

    cout << course->number << ", " << course->title << '\n';
    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None\n";
        return;
    }
    cout << "Prerequisites:\n";
    for (const string& number : course->prerequisites) {
        const Course* prerequisite = tree.find(number);
        if (prerequisite) cout << prerequisite->number << ", " << prerequisite->title << '\n';
        else cout << number << '\n';
    }
}

int main() {
    CourseTree courses;
    bool loaded = false;
    string choice;

    cout << "Welcome to the ABCU Advising Assistance Program.\n";
    while (true) {
        cout << "\n1. Load Data Structure\n"
             << "2. Print Course List\n"
             << "3. Print Course\n"
             << "9. Exit\n"
             << "What would you like to do? ";
        if (!getline(cin, choice)) break;
        choice = trim(choice);

        if (choice == "1") {
            cout << "Enter the course data file name: ";
            string fileName;
            if (!getline(cin, fileName)) break;
            // A failed load preserves previously loaded data.
            if (loadCourses(trim(fileName), courses)) loaded = true;
        } else if (choice == "2") {
            if (!loaded) cout << "Please load the course data first.\n";
            else {
                cout << "\nHere is a sample schedule:\n";
                courses.printList();
            }
        } else if (choice == "3") {
            if (!loaded) cout << "Please load the course data first.\n";
            else {
                cout << "What course do you want to know about? ";
                string number;
                if (!getline(cin, number)) break;
                printCourseInformation(courses, number);
            }
        } else if (choice == "9") {
            cout << "Thank you for using the course planner!\n";
            break;
        } else {
            cout << "Invalid option. Please enter 1, 2, 3, or 9.\n";
        }
    }
    return 0;
}
