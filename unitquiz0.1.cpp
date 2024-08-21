#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>

using namespace std;

// Global map to store each tag's value
map<string, double> tag_values;

// Structure to represent a unit
struct Unit {
    string name;
    map<string, double> tags;
    string description;
    double satisfaction;
};

// Function to process each line of question and tags
void processLine(const string& line, int user_input) {
    size_t pos = line.find("tag:");
    if (pos != string::npos) {
        string tag_section = line.substr(pos + 4);
        
        stringstream ss(tag_section);
        string tag;
        while (getline(ss, tag, ';')) {  // Split tags by semicolon
            size_t split_pos = tag.find(']');
            string tag_name = tag.substr(1, split_pos - 1);
            int tag_value = stoi(tag.substr(split_pos + 2));
            
            tag_values[tag_name] += tag_value * user_input;  // Update the tag value based on user input
        }
    }
}

// Function to read questions from the file and process user responses
void askQuestionsFromFile(const string& filename) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Unable to open file: " << filename << endl;
        return;
    }

    string line;
    while (getline(infile, line)) {
        if (line.empty()) continue;
        if (line[0] == '[') {  // Detect a question
            cout << line.substr(line.find(']') + 1) << endl;

            int user_input;
            cout << "Please enter your rating (-2 to 2): ";
            cin >> user_input;
            while (user_input < -2 || user_input > 2) {
                cout << "Invalid input, please enter a value between -2 and 2: ";
                cin >> user_input;
            }

            // Process the next tag line
            if (getline(infile, line)) {
                processLine(line, user_input);
            }
        }
    }
    infile.close();
}

// Function to process unit information
Unit processUnit(const string& name_line, const string& tag_line, const string& description) {
    Unit unit;
    unit.name = name_line.substr(1, name_line.size() - 2);  // Extract unit name

    string tag_section = tag_line.substr(4);
    stringstream ss(tag_section);
    string tag;

    while (getline(ss, tag, ';')) {  // Split tags by semicolon
        size_t split_pos = tag.find('[');
        if (split_pos != string::npos) {
            string tag_name = tag.substr(0, split_pos);
            double tag_value = stod(tag.substr(split_pos + 1, tag.size() - split_pos - 2));  // Extract tag value
            unit.tags[tag_name] = tag_value;
        } else {
            unit.tags[tag] = 1.0;  // Default tag value is 1
        }
    }

    unit.description = description;  // Assign description
    return unit;
}

// Function to calculate the satisfaction of a unit
double calculateSatisfaction(const Unit& unit) {
    double required_sum = 0;
    double actual_sum = 0;

    for (const auto& tag : unit.tags) {
        required_sum += tag.second;  // Sum required tag values
        actual_sum += min(tag.second, tag_values[tag.first]);  // Sum the actual tag values
    }

    return actual_sum / required_sum;  // Return the satisfaction percentage
}

// Function to read units data from file
vector<Unit> readUnitsFromFile(const string& filename) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Unable to open file: " << filename << endl;
        return {};
    }

    vector<Unit> units;
    string line, name, tag_line, description;
    while (getline(infile, line)) {
        if (line.empty()) continue;

        if (line[0] == '[' && line.back() == ']') {
            name = line;  // Extract unit name line
        } else if (line.find("tag:") == 0) {
            tag_line = line;  // Extract tag line
        } else {
            description = line;  // Extract description
            units.push_back(processUnit(name, tag_line, description));  // Process and store unit
        }
    }
    infile.close();
    return units;
}

// Function to print the user's acquired tag values
void printTagValues() {
    cout << "\nUser-acquired tag values:\n";
    for (const auto& tag : tag_values) {
        cout << tag.first << ": " << tag.second << endl;
    }
}

// Function to find and print the top satisfaction units and their descriptions
void findTopSatisfactionUnits(const vector<Unit>& units) {
    vector<Unit> sorted_units = units;

    // Calculate satisfaction for each unit
    for (auto& unit : sorted_units) {
        unit.satisfaction = calculateSatisfaction(unit);
    }

    // Sort units by satisfaction
    sort(sorted_units.begin(), sorted_units.end(), [](const Unit& a, const Unit& b) {
        return a.satisfaction > b.satisfaction;
    });

    // Print the top 5 units with highest satisfaction
    cout << "\nTop 5 units by satisfaction:\n";
    for (int i = 0; i < min(5, (int)sorted_units.size()); ++i) {
        cout << sorted_units[i].name << " Satisfaction: " << sorted_units[i].satisfaction * 100 << "%" << endl;
    }

    // Print the description of the unit with highest satisfaction
    if (!sorted_units.empty()) {
        cout << "\nDescription of the top unit:\n";
        cout << sorted_units[0].description << endl;
    }
}

int main() {
    // Read questions from file and process them
    askQuestionsFromFile("quizs");

    // Print the user's acquired tag values
    printTagValues();

    // Read unit data from file
    vector<Unit> units = readUnitsFromFile("units");

    // Find and print the top satisfaction units and the highest unit's description
    findTopSatisfactionUnits(units);

    return 0;
}
