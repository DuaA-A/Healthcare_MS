#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
using namespace std;

template <typename T>
int binarySearch(const vector<pair<T, int>>& index, const T& key) {
    int low = 0, high = index.size() - 1;
    while (low <= high) {
        int mid = low + (high - low)/ 2;
        if (index[mid].first == key)
            return mid;
        else if (index[mid].first < key)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return -1;
}
struct Node {
    string doctorID;
    Node* next;
};
class LinkedList {
public:
    Node* head;
    LinkedList() : head(nullptr) {}

    void insert(const string& doctorID) {
        Node* newNode = new Node{doctorID, head};
        head = newNode;
    }

    bool find(const string& doctorID) {
        Node* current = head;
        while (current) {
            if (current->doctorID == doctorID)
                return true;
            current = current->next;
        }
        return false;
    }

    void remove(const string& doctorID) {
        Node* current = head;
        Node* prev = nullptr;
        while (current) {
            if (current->doctorID == doctorID) {
                if (prev)
                    prev->next = current->next;
                else
                    head = current->next;
                delete current;
                return;
            }
            prev = current;
            current = current->next;
        }
    }
};
class SecondaryIndex {
public:
    map<string, LinkedList> index;
    const string SECONDARY_INDEX_FILE = "doctor_secondary.index";  
    void insert(const string& secondaryKey, const string& doctorID);
    vector<string> find(const string& secondaryKey);
    void remove(const string& secondaryKey, const string& doctorID);
    void load();
    void save();  
};

void SecondaryIndex::insert(const string& secondaryKey, const string& doctorID) {
    index[secondaryKey].insert(doctorID);
}

// vector<string> SecondaryIndex::find(const string& secondaryKey) {
//     auto it = index.find(secondaryKey);
//     if (it != index.end()) {
//         vector<string> result;
//         for (const auto& item : it->second) {  // Assuming LinkedList is iterable
//             result.push_back(item);
//         }
//         return result;
//     }
//     return {};
// }
void SecondaryIndex::remove(const string& secondaryKey, const string& doctorID) {
    LinkedList& doctorList = index[secondaryKey];

    doctorList.remove(doctorID);
    if (doctorList.head == nullptr) {
        index.erase(secondaryKey);
    }
}

void SecondaryIndex::load() {
    fstream file(SECONDARY_INDEX_FILE, ios::in);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string name;
            string doctorID;
            getline(ss, name, '|');
            while (getline(ss, doctorID, '|')) {
                insert(name, doctorID);
            }
        }
        file.close();
    }
}

void SecondaryIndex::save() {
    fstream file(SECONDARY_INDEX_FILE, ios::out | ios::trunc);
    if (file.is_open()) {
        for (const auto& entry : index) {
            file << entry.first;
            Node* current = entry.second.head;
            while (current) {
                file << "|" << current->doctorID;
                current = current->next;
            }
            file << "\n";
        }
        file.close();
    }
}
class HealthcareManagementSystem {

    vector<pair<string, int>> doctorPrimaryIndex;
    vector<pair<string, int>> appointmentPrimaryIndex; 
    SecondaryIndex doctorSecondaryIndex;
    SecondaryIndex appointmentSecondaryIndex;
    vector<int> doctorAvailList;
    vector<int> appointmentAvailList;

    const string DOCTOR_FILE = "doctors.txt";
    const string DOCTOR_INDEX_FILE = "doctor.index";
    const string APPOINTMENT_FILE = "appointments.txt";
    const string APPOINTMENT_INDEX_FILE = "appointment.index";

public:
    void displayMenu();
    void addDoctor(const string& doctorID, const string& name, const string& address);
    void addAppointment(const string& appointmentID, const string& doctorID, const string& date);
    void updateDoctor();
    void updateAppointment();
    void deleteDoctor();
    void deleteAppointment();
    void searchDoctorByID(string doctorID);
    void searchDoctorByName();
    void searchAppointmentsByDoctorID();
    void loadIndexes();
    void saveIndexes();
    void loadAvailList(vector<int>& availList, const string& fileName);
    void saveAvailList(const vector<int>& availList, const string& fileName);
    void processQuery(const string& query);
    void searchNameForQuary(string doctorID);


private:
    string readRecordFromFile(const string& fileName, int position);
    int static findAvailableSlot(vector<int>& availList, const string& fileName);
    void markDeleted(vector<int>& availList, int position, const string& fileName);
    string extractField(const string& record, int fieldIndex);
};

void HealthcareManagementSystem::displayMenu() {
    cout << "\n--- Healthcare Management System ---\n";
    cout << "1. Add New Doctor\n";
    cout << "2. Add New Appointment\n";
    cout << "3. Update Doctor Name\n";
    cout << "4. Update Appointment Date\n";
    cout << "5. Delete Doctor\n";
    cout << "6. Delete Appointment\n";
    cout << "7. Search Doctor by ID\n";
    cout << "8. Search Doctor by Name\n";
    cout << "9. Search Appointments by Doctor ID\n";
    cout << "10. Exit\n";
    cout << "Enter your choice: ";
}
string HealthcareManagementSystem::readRecordFromFile(const string& fileName, int position) {
    fstream file(fileName, ios::in);
    file.seekg(position, ios::beg);
    string record;
    getline(file, record);
    if (!record.empty() && record.back() == '*') 
        record = record.substr(0, record.length() - 1);
    
    file.close();
    return record;
}

int HealthcareManagementSystem::findAvailableSlot(vector<int>& availList, const string& fileName) {
    if (!availList.empty()) {
        int position = availList.back();
        availList.pop_back();
        return position;
    }
    return -1; 
}

void HealthcareManagementSystem::markDeleted(vector<int>& availList, int position, const string& fileName) {
    availList.push_back(position);
    fstream file(fileName, ios::in | ios::out);
    file.seekp(position, ios::beg);
    string record = readRecordFromFile(fileName, position);
    if (!record.empty()) {
        file.seekp(position + record.length() - 1, ios::beg);
        file.put('*');
    }
    file.close();
}

void HealthcareManagementSystem::addDoctor(const string& doctorID, const string& name, const string& address) {
    if (binarySearch(doctorPrimaryIndex, doctorID) != -1) {
        cout << "Doctor with this ID already exists.\n";
        return;
    }

    string fullRecord = doctorID + "|" + name + "|" + address;
    int recordLength = fullRecord.length();
    int position = -1;

    if (!doctorAvailList.empty()) {

        int availablePosition = doctorAvailList.back();

        string existingRecord = readRecordFromFile(DOCTOR_FILE, availablePosition);
        int existingRecordLength = stoi(existingRecord.substr(0, 4));

        if (existingRecordLength >= recordLength) {

            position = availablePosition;
            doctorAvailList.pop_back();
        }
    }
    if(position == -1 && doctorPrimaryIndex.empty()) {
        position=0;
    }

    if (position == -1) {
        fstream doctorFile(DOCTOR_FILE, ios::in | ios::out | ios::app);
        if (!doctorFile) {
            cerr << "Error opening doctors.txt!" << endl;
            return;
        }
        doctorFile.seekp(0, ios::end);
        position = doctorFile.tellp();
        doctorFile.close();
    }

    // Write the new doctor record
    fstream doctorFile(DOCTOR_FILE, ios::in | ios::out);
    doctorFile.seekp(position, ios::beg);
    stringstream ss;
    ss << setw(4) << setfill('0') << recordLength;
    string fixedLength = ss.str();

    // Append the new doctor record
    doctorFile << fixedLength << fullRecord<<'\n';
    doctorFile.close();

    doctorPrimaryIndex.push_back({doctorID, position});
    sort(doctorPrimaryIndex.begin(), doctorPrimaryIndex.end());

    // Update the secondary index (doctor's name)
    doctorSecondaryIndex.insert(name, doctorID);

    // Save indexes to file
    saveIndexes();
    doctorSecondaryIndex.save();

    cout << "Doctor added successfully.\n";
}


void HealthcareManagementSystem::deleteDoctor() {
    string doctorID;
    cout << "Enter Doctor ID to delete: ";
    cin >> doctorID;
    int pos = binarySearch(doctorPrimaryIndex, doctorID);
    if (pos == -1) {
        cout << "Doctor not found.\n";
        return;
    }
    string doctorIDToDelete = doctorPrimaryIndex[pos].first;
    int recordPosition = doctorPrimaryIndex[pos].second;
    string record = readRecordFromFile(DOCTOR_FILE, recordPosition);
    size_t d1 = record.find('|'); 
    size_t d2 = record.find('|', d1 + 1);  
    string name = record.substr(d1 + 1, d2 - d1 - 1);  
    markDeleted(doctorAvailList, recordPosition, DOCTOR_FILE);
    doctorPrimaryIndex.erase(doctorPrimaryIndex.begin() + pos);
    saveIndexes(); 
    doctorSecondaryIndex.remove(name, doctorID);
    doctorSecondaryIndex.save();

    cout << "Doctor deleted successfully.\n";
}

void HealthcareManagementSystem::searchDoctorByID(string doctorID) {


    // Search doctorID in the primary index
    int pos = binarySearch(doctorPrimaryIndex, doctorID);
    if (pos == -1) {
        cout << "Doctor not found.\n";
        return;
    }

    string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);// Read the doctor record from the file
    // Extract Doctor ID, Name, and Address in a single pass
    size_t d1 = record.find('|');  // Find the first delimiter (after the doctor ID)
    string id = record.substr(4, d1 - 4);  // Doctor ID is after the length field (first 6 chars)

    size_t d2 = record.find('|', d1 + 1);  // Find the second delimiter (after the doctor ID)
    string name = record.substr(d1 + 1, d2 - d1 - 1);  // Extract doctor name between the first and second '|'
    // Extract the address (rest of the string after the second '|')
    string address = record.substr(d2 + 1);

    cout << "\n--- Doctor Details ---\n";
    cout << "Doctor ID: " << id << "\n";
    cout << "Name: " << name << "\n";
    cout << "Address: " << address << "\n";
    cout << "-----------------------\n";
}



void HealthcareManagementSystem::searchDoctorByName() {
    string name;
    cout << "Enter Doctor Name to search: ";
    cin.ignore();
    getline(cin, name);

    Node* doctorIDs = doctorSecondaryIndex.index[name].head;
    if(!doctorIDs) {
        cout << "No doctors found with the name: " << name << endl;
        return;
    }

    while (doctorIDs) {
        int pos = binarySearch(doctorPrimaryIndex, doctorIDs->doctorID);
        if (pos != -1) {
            string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);

            string doctorID = extractField(record.substr(4,record.length()), 0);
            string doctorName = extractField(record, 1);
            string doctorAddress = extractField(record, 2);

            cout << "\n--- Doctor Details ---\n";
            cout << "Doctor ID: " << doctorID << "\n";
            cout << "Name: " << doctorName << "\n";
            cout << "Address: " << doctorAddress << "\n";
            cout << "-----------------------\n";
        }
        doctorIDs = doctorIDs->next;
    }
}
string HealthcareManagementSystem::extractField(const string& record, int fieldIndex) {
    stringstream ss(record);
    string field;
    int currentIndex = 0;

    while (getline(ss, field, '|')) {
        if (currentIndex == fieldIndex) {
            return field;
        }
        currentIndex++;
    }
    return "";  
}
void HealthcareManagementSystem::addAppointment(const string& appointmentID, const string& doctorID, const string& date) {
    if (appointmentID.length() > 15 || doctorID.length() > 15 || date.length() > 30) {
        cout << "Error: Input exceeds the maximum allowed length.\n";
        return;
    }
    if (binarySearch(appointmentPrimaryIndex, appointmentID) != -1) {
        cout << "Appointment with this ID already exists.\n";
        return;
    }
    int position = findAvailableSlot(appointmentAvailList, APPOINTMENT_FILE);
    if (position == -1) {
        fstream appointmentFile(APPOINTMENT_FILE, ios::in | ios::out);
        if (!appointmentFile) {
            cerr << "Error opening appointments.txt!" << endl;
            return;
        }
        appointmentFile.seekp(0, ios::end);
        position = appointmentFile.tellp();
        appointmentFile.close();
    }
    string fullRecord = appointmentID + "|" + date + "|" + doctorID;
    int recordLength = fullRecord.length();
    stringstream ss;
    ss << setw(4) << setfill('0') << recordLength; 
    string fixedLength = ss.str();
    fstream appointmentFile(APPOINTMENT_FILE, ios::in | ios::out);
    if (!appointmentFile) {
        cerr << "Error: Unable to open " << APPOINTMENT_FILE << endl;
        return;
    }
    appointmentFile.seekp(position, ios::beg);
    appointmentFile << fixedLength <<string(1, ' ') <<fullRecord << "\n"; 
    appointmentFile.close();
    auto it = lower_bound(appointmentPrimaryIndex.begin(), appointmentPrimaryIndex.end(), make_pair(appointmentID, 0));
    appointmentPrimaryIndex.insert(it, {appointmentID, position});
    appointmentSecondaryIndex.insert(doctorID, appointmentID);
    saveIndexes();
    cout << "Appointment added successfully.\n";
}



void HealthcareManagementSystem::updateAppointment() {
    string appointmentID, newDate;
    cout << "Enter Appointment ID to update: ";
    cin >> appointmentID;

    int pos = binarySearch(appointmentPrimaryIndex, appointmentID);
    if (pos == -1) {
        cout << "Appointment not found.\n";
        return;
    }
    cout << "Enter new appointment date (leave blank to skip): ";
    cin.ignore();
    getline(cin, newDate);
    string record = readRecordFromFile(APPOINTMENT_FILE, appointmentPrimaryIndex[pos].second);
    size_t delim1 = record.find('|');
    size_t delim2 = record.find('|', delim1 + 1);
    string id = record.substr(0, delim1);  
    string date = record.substr(delim1 + 1, delim2 - delim1 - 1);  
    string doctorID = record.substr(delim2 + 1); 
    if (!newDate.empty()) {
        date = newDate; 
    }
    string updatedRecord = id + "|" + date + "|" + doctorID;
    fstream file(APPOINTMENT_FILE, ios::in | ios::out);
    file.seekp(appointmentPrimaryIndex[pos].second, ios::beg);
    file << setw(4) << setfill('0') << updatedRecord.length() << updatedRecord << "\n";
    file.close();
    cout << "Appointment updated successfully.\n";
}
//there's an error in find() function, it returns linked list but in the below function it needs a vector<string>* 
//error at line #469
// void HealthcareManagementSystem::deleteAppointment() {
//     string appointmentID;
//     cout << "Enter Appointment ID to delete: ";
//     cin >> appointmentID;
//     int pos = binarySearch(appointmentPrimaryIndex, appointmentID);
//     if (pos == -1) {
//         cout << "Appointment not found.\n";
//         return;
//     }
//     int position = appointmentPrimaryIndex[pos].second;
//     markDeleted(appointmentAvailList, position, APPOINTMENT_FILE);
//     appointmentPrimaryIndex.erase(appointmentPrimaryIndex.begin() + pos);
//     string record = readRecordFromFile(APPOINTMENT_FILE, position);
//     size_t delim1 = record.find('|');
//     size_t delim2 = record.find('|', delim1 + 1);
//     string doctorID = record.substr(delim2 + 1);
    
//      vector<string>* appointments = appointmentSecondaryIndex.find(doctorID);
//     if (appointments) {
//         appointments->erase(remove(appointments->begin(), appointments->end(), appointmentID), appointments->end());
//         if (appointments->empty()) {
//             appointmentSecondaryIndex.index.erase(doctorID); 
//         }
//     }
//     saveIndexes();
//     cout << "Appointment deleted successfully.\n";
// }

// same error at line 485 : auto* doesn't match the return value of the find function
// void HealthcareManagementSystem::searchAppointmentsByDoctorID() {
//     string doctorID;
//     cout << "Enter Doctor ID to search for appointments: ";
//     cin >> doctorID;
//     auto* appointmentIDs = appointmentSecondaryIndex.find(doctorID);
//     if (!appointmentIDs || appointmentIDs->empty()) {
//         cout << "No appointments found for Doctor ID: " << doctorID << endl;
//         return;
//     }
//     cout << "Appointments for Doctor ID: " << doctorID << "\n";
//     for (const string& appointmentID : *appointmentIDs) {
//         int pos = binarySearch(appointmentPrimaryIndex, appointmentID);
//         if (pos != -1) {
//             string record = readRecordFromFile(APPOINTMENT_FILE, appointmentPrimaryIndex[pos].second);
//             size_t delim1 = record.find('|');
//             size_t delim2 = record.find('|', delim1 + 1);
//             string id = record.substr(0, delim1); 
//             string date = record.substr(delim1 + 1, delim2 - delim1 - 1);  
//             string docID = record.substr(delim2 + 1);
//             cout << "\n--- Appointment Details ---\n";
//             cout << "Appointment ID: " << id << "\n";
//             cout << "Date: " << date << "\n";
//             cout << "Doctor ID: " << docID << "\n";
//             cout << "---------------------------\n";
//         }
//     }
// }
void HealthcareManagementSystem::loadAvailList(vector<int>& availList, const string& fileName) {
    ifstream file(fileName, ios::in);
    if (!file) {
        cerr << "Error: Unable to open " << fileName << " for reading." << endl;
        return;
    }
    int pos;
    while (file >> pos) {
        availList.push_back(pos);
    }
    file.close();
}

void HealthcareManagementSystem::saveAvailList(const vector<int>& availList, const string& fileName) {
    ofstream file(fileName, ios::out | ios::trunc);
    if (!file) {
        cerr << "Error: Unable to open " << fileName << " for writing." << endl;
        return;
    }
    for (int pos : availList) {
        file << pos << "\n";
    }
    file.close();
}


void HealthcareManagementSystem::loadIndexes() {
    fstream doctorIndexFile(DOCTOR_INDEX_FILE, ios::in);
    if (doctorIndexFile.is_open()) {
        string line;
        while (getline(doctorIndexFile, line)) {
            stringstream ss(line);
            string doctorID;
            int position;
            getline(ss, doctorID, '|');
            ss >> position;
            doctorPrimaryIndex.push_back({doctorID, position});
        }
        doctorIndexFile.close();
    }
    sort(doctorPrimaryIndex.begin(), doctorPrimaryIndex.end());

    fstream appointmentIndexFile(APPOINTMENT_INDEX_FILE, ios::in);
    if (appointmentIndexFile.is_open()) {
        string line;
        while (getline(appointmentIndexFile, line)) {
            stringstream ss(line);
            string appointmentID;
            int position;
            getline(ss, appointmentID, '|');
            ss >> position;
            appointmentPrimaryIndex.push_back({appointmentID, position});
        }
        appointmentIndexFile.close();
    }
    sort(appointmentPrimaryIndex.begin(), appointmentPrimaryIndex.end());
    loadAvailList(doctorAvailList, "doctor.avail");
    loadAvailList(appointmentAvailList, "appointment.avail");
}
void HealthcareManagementSystem::saveIndexes() {
    fstream doctorIndexFile(DOCTOR_INDEX_FILE, ios::out);
    if (!doctorIndexFile) {
        cerr << "Error: Unable to open " << DOCTOR_INDEX_FILE << " for writing." << endl;
        return;
    }
    for (const auto& entry : doctorPrimaryIndex) {
        doctorIndexFile << entry.first << "|" << entry.second << "\n";
    }
    doctorIndexFile.close();

    fstream appointmentIndexFile(APPOINTMENT_INDEX_FILE, ios::out);
    if (!appointmentIndexFile) {
        cerr << "Error: Unable to open " << APPOINTMENT_INDEX_FILE << " for writing." << endl;
        return;
    }
    for (const auto& entry : appointmentPrimaryIndex) {
        appointmentIndexFile << entry.first << "|" << entry.second << "\n";
    }
    appointmentIndexFile.close();

    saveAvailList(doctorAvailList, "doctor.avail");
    saveAvailList(appointmentAvailList, "appointment.avail");
}


void HealthcareManagementSystem::updateDoctor() {
    string doctorID;
    cout << "Enter Doctor ID to update: ";
    cin >> doctorID;
    int pos = binarySearch(doctorPrimaryIndex, doctorID);
    if (pos == -1) {
        cout << "Doctor not found.\n";
        return;
    }
    string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);
    if (record.empty()) {
        cout << "Error reading the doctor record.\n";
        return;
    }
    int recordLength = stoi(record.substr(0, 4));
    size_t d1 = record.find('|');  
    size_t d2 = record.find('|', d1 + 1);  
    string name = record.substr(d1 + 1, d2 - d1 - 1);  
    string address = record.substr(d2 + 1);  
    string newName, newAddress;
    cout << "Enter new name (leave blank to keep current): ";
    cin.ignore();
    getline(cin, newName);
    cout << "Enter new address (leave blank to keep current): ";
    getline(cin, newAddress);
    if (newName.empty()) {
        newName = name;
    }
    if (newAddress.empty()) {
        newAddress = address;
    }
    doctorSecondaryIndex.remove(name, doctorID);
    string newRecord = doctorID + "|" + newName + "|" + newAddress;
    int newRecordLength = newRecord.length();
    if (newRecordLength != recordLength) {
        cout << "Error: New record length should be " << recordLength << " characters.\n";
        return;
    }
    fstream doctorFile(DOCTOR_FILE, ios::in | ios::out);
    doctorFile.seekp(doctorPrimaryIndex[pos].second, ios::beg);
    if (readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second).back() == '*') {
        doctorFile.seekp(doctorPrimaryIndex[pos].second + newRecord.length() - 1, ios::beg);
        doctorFile.put(' ');
    }
    stringstream ss;
    ss << setw(4) << setfill('0') << newRecordLength;
    string fixedLength = ss.str();
    doctorFile << fixedLength << newRecord;
    doctorFile.close();
    doctorPrimaryIndex[pos].first = doctorID;
    doctorSecondaryIndex.insert(newName, doctorID);
    saveIndexes();
    doctorSecondaryIndex.save();
    cout << "Doctor record updated successfully.\n";
}
void HealthcareManagementSystem::searchNameForQuary(string doctorID) {
    string name;
    cout << "Enter Doctor Name to search: ";
    cin.ignore();
    getline(cin, name);
    Node* doctorIDs = doctorSecondaryIndex.index[name].head;
    if (!doctorIDs) {
        cout << "No doctors found with the name: " << name << endl;
        return;
    }
    while (doctorIDs) {
        int pos = binarySearch(doctorPrimaryIndex, doctorIDs->doctorID);
        if (pos != -1) {
            string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);

            string doctorID = extractField(record.substr(4,record.length()), 0);
            string doctorName = extractField(record, 1);
            string doctorAddress = extractField(record, 2);

            cout << "Name: " << doctorName << "\n";
        }
        doctorIDs = doctorIDs->next;
    }
}

void HealthcareManagementSystem::processQuery(const string& query) {
    string sanitizedQuery = query;
    sanitizedQuery.erase(remove(sanitizedQuery.begin(), sanitizedQuery.end(), ' '), sanitizedQuery.end());
    size_t posWhere = sanitizedQuery.find("WHERE");
    if (posWhere == string::npos) {
        cout << "Invalid query format.\n";
        return;
    }
    string condition = sanitizedQuery.substr(posWhere + 5);
    size_t posEqual = condition.find('=');
    if (posEqual == string::npos) {
        cout << "Invalid condition in query.\n";
        return;
    }
    string key = condition.substr(0, posEqual);
    string value = condition.substr(posEqual + 1);
    if (!value.empty() && value.front() == '\'' && value.back() == '\'') 
        value = value.substr(1, value.size() - 2);
    
    value.erase(0, value.find_first_not_of(' '));
    value.erase(value.find_last_not_of(' ') + 1);
    if (key == "DoctorID") 
        searchDoctorByID(value);
    else if (key == "DoctorName") {
        cout << "Searching for Doctor Name: " << value << endl;
        searchNameForQuary(value);
    }
    else
        cout << "Unknown key in query.\n";
    
}


int main() {
    HealthcareManagementSystem system;
    HealthcareManagementSystem query;
    system.loadIndexes();
    int choice;
    
    do {
        system.displayMenu(); 
        cin >> choice; 
        
        switch (choice) {
            case 1: {  // Add New Doctor
                string doctorID, name, address;
                cout << "Enter Doctor ID: ";
                cin >> doctorID;
                cout << "Enter Doctor Name: ";
                cin.ignore(); 
                getline(cin, name);
                cout << "Enter Doctor Address: ";
                getline(cin, address);
                system.addDoctor(doctorID, name, address);  
                break;
            }
            case 2: {  // Add New Appointment
                string appointmentID, doctorID, date;
                cout << "Enter Appointment ID: ";
                cin >> appointmentID;
                cout << "Enter Doctor ID: ";
                cin >> doctorID;
                cout << "Enter Appointment Date: ";
                cin.ignore(); 
                getline(cin, date);
                system.addAppointment(appointmentID, doctorID, date);
                break;
            }
            case 3: {  // Update Doctor Name
                system.updateDoctor(); 
                break;
            }
            case 4: {  // Update Appointment Date
                system.updateAppointment(); 
                break;
            }
            case 5: {  // Delete Doctor
                system.deleteDoctor();  
                break;
            }
            case 6: {// search Doctor By ID
                string doctorID;
                cout << "Enter Doctor ID to search: ";
                cin >> doctorID;
                system.searchDoctorByID(doctorID);
                break;
            }
            case 7: {// search Doctor By Name
                system.searchDoctorByName();
                break;
            }
            case 8: {//get query
                string query;
                cout << "Enter your query: ";
                cin.ignore();
                getline(cin, query);
                system.processQuery(query);
                break;
            }
            case 11: {
                cout << "Exiting...\n";
                break;
            }
            default: { 
                cout << "Invalid choice. Please try again.\n";
                break;
            }
        }
    } while (1); 
    system.saveIndexes();  
    return 0;  
}
