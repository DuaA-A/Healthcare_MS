#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <utility>

using namespace std;
using std::literals::string_literals::operator""s;

string ignoreKeysCasePermutations(string key) {
    //until the first qoute
    long end = (long)key.find('\'');
    transform(key.begin(), key.begin() + end, key.begin(), ::tolower);
    return key;
}

bool isValidQuery(const string& query) {

    bool startsWithSelectFrom = query.substr(0, 11) == "select*from";
    bool startsWithSelectDoctorNameFrom = query.substr(0, 20) == "selectdoctornamefrom";

    if (!startsWithSelectFrom && !startsWithSelectDoctorNameFrom) {
        return false; // Invalid query if it doesn't start with either format
    }

    // After 'from', check for table names: 'doctors' or 'appointments'
    size_t fromPos = startsWithSelectFrom ? 11 : 20;  // Adjust starting point based on the format
    if (query.substr(fromPos, 7) != "doctors" && query.substr(fromPos, 12) != "appointments") {
        return false; // Invalid table name
    }

    // Find the 'where' clause in the query
    size_t wherePos = query.find("where");
    if (wherePos == string::npos) {
        return false; // 'where' clause missing
    }

    // Fields should be either 'doctorid', 'doctorname', or 'appointmentid'
    size_t fieldStartPos = wherePos + 5; // Skip "where"
    if (query.substr(fieldStartPos, 8) != "doctorid" &&
        query.substr(fieldStartPos, 10) != "doctorname" &&
        query.substr(fieldStartPos, 13) != "appointmentid") {
        return false; // Invalid field name
    }

    // The field value should be between single quotes
    size_t firstQuotePos = query.find('\'', wherePos);
    if (firstQuotePos == string::npos) {
        return false; // Missing opening quote
    }

    size_t secondQuotePos = query.find('\'', firstQuotePos + 1);
    if (secondQuotePos == string::npos) {
        return false; // Missing closing quote
    }

    string fieldValue = query.substr(firstQuotePos + 1, secondQuotePos - firstQuotePos - 1);
    if (fieldValue.empty()) {
        return false; // Empty field value
    }

    return true;
}

template <typename T>
int binarySearch(const vector<pair<T, int>>& index, const T& key) {
    int low = 0, high = index.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (index[mid].first == key)
            return mid;
        else if (index[mid].first < key)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return -1;
}
struct DoctorNode {
    string doctorID;
    DoctorNode* next;
};
class LinkedList {
public:
    DoctorNode* head;
    LinkedList() : head(nullptr) {}

    void insert(const string& doctorID) {
        DoctorNode* newNode = new DoctorNode{doctorID, head};
        head = newNode;
    }

    bool find(const string& doctorID) {
        DoctorNode* current = head;
        while (current) {
            if (current->doctorID == doctorID)
                return true;
            current = current->next;
        }
        return false;
    }

    void remove(const string& doctorID) {
        DoctorNode* current = head;
        DoctorNode* prev = nullptr;
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

class DoctorSecondaryIndex {
public:
    map<string, LinkedList> Index; 
    const string DOCTOR_SECONDARY_INDEX_FILE = "doctor_secondary.index";

    void Insert(const string& secondaryKey, const string& doctorID);
    bool find(const string& secondaryKey, const string& doctorID);
    void remove(const string& secondaryKey, const string& doctorID);
    void load(); 
    void save(); 
};


void DoctorSecondaryIndex::Insert(const string& secondaryKey, const string& doctorID) {
    Index[secondaryKey].insert(doctorID);
}

bool DoctorSecondaryIndex::find(const string& secondaryKey, const string& appointmentID) {
    auto it = Index.find(secondaryKey);
    if (it != Index.end()) {
        return it->second.find(appointmentID);
    }
    return false;
}

void DoctorSecondaryIndex::remove(const string& secondaryKey, const string& doctorID) {
    LinkedList& doctorList = Index[secondaryKey];
    doctorList.remove(doctorID);
    if (doctorList.head == nullptr) {
        Index.erase(secondaryKey);
    }
}

void DoctorSecondaryIndex::load() {
    fstream file(DOCTOR_SECONDARY_INDEX_FILE, ios::in);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string name;
            string doctorID;
            getline(ss, name, '|');
            while (getline(ss, doctorID, '|')) {
                Insert(name, doctorID);
            }
        }
        file.close();
    }
}

void DoctorSecondaryIndex::save() {
    fstream file(DOCTOR_SECONDARY_INDEX_FILE, ios::out | ios::trunc);
    if (file.is_open()) {
        for (const auto& entry : Index) {
            file << entry.first;
            DoctorNode* current = entry.second.head;
            while (current) {
                file << "|" << current->doctorID;
                current = current->next;
            }
            file << "\n";
        }
        file.close();
    }
}
//---------------------------------------------------
struct AppointmentNode {
    string appointmentID;
    string doctorID;
    string date;
    AppointmentNode* next;

    AppointmentNode(const string& id, const string& docId, const string& d, AppointmentNode* next = nullptr)
        : appointmentID(id), doctorID(docId), date(d), next(next) {}
};

class AppointmentLinkedList {
public:
    AppointmentNode* head;

    AppointmentLinkedList() : head(nullptr) {}

    void insert(const string& appointmentID, const string& doctorID, const string& date) {
        AppointmentNode* newNode = new AppointmentNode(appointmentID, doctorID, date, head);
        head = newNode;
    }

    bool find(const string& appointmentID) {
        AppointmentNode* current = head;
        while (current) {
            if (current->appointmentID == appointmentID)
                return true;
            current = current->next;
        }
        return false;
    }

    void remove(const string& appointmentID) {
        AppointmentNode* current = head;
        AppointmentNode* prev = nullptr;
        while (current) {
            if (current->appointmentID == appointmentID) {
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

class AppointmentSecondaryIndex {
public:
    map<string, AppointmentLinkedList> Index;
    const string APPOINTMENT_SECONDARY_INDEX_FILE = "appointment_secondary.index";

    void insert(const string& doctorID, const string& appointmentID, const string& date);
    void remove(const string& doctorID, const string& appointmentID);
    bool find(const string& secondaryKey, const string& appointmentID);
    void load();
    void save();
};

void AppointmentSecondaryIndex::insert(const string& doctorID, const string& appointmentID, const string& date) {
    if (Index.find(doctorID) == Index.end()) {
        Index[doctorID] = AppointmentLinkedList();
    }
    Index[doctorID].insert(appointmentID, doctorID, date);
}

void AppointmentSecondaryIndex::remove(const string& doctorID, const string& appointmentID) {
    auto it = Index.find(doctorID);
    if (it != Index.end()) {
        it->second.remove(appointmentID);
        if (it->second.head == nullptr) {
            Index.erase(it);
        }
    }
}

bool AppointmentSecondaryIndex::find(const string& doctorID, const string& appointmentID) {
    auto it = Index.find(doctorID);
    if (it != Index.end()) {
        return it->second.find(appointmentID);
    }
    return false;
}

void AppointmentSecondaryIndex::load() {
    fstream file(APPOINTMENT_SECONDARY_INDEX_FILE, ios::in);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string doctorID, appointmentID, date;
            getline(ss, doctorID, '|');
            while (getline(ss, appointmentID, '|') && getline(ss, date, '|')) {
                insert(doctorID, appointmentID, date);
            }
        }
        file.close();
    }
}

void AppointmentSecondaryIndex::save() {
    fstream file(APPOINTMENT_SECONDARY_INDEX_FILE, ios::out | ios::trunc);
    if (file.is_open()) {
        for (const auto& entry : Index) {
            file << entry.first;
            AppointmentNode* current = entry.second.head;
            while (current) {
                file << "|" << current->appointmentID << "|" << current->date;
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
    DoctorSecondaryIndex doctorSecondaryIndex;
    AppointmentSecondaryIndex appointmentSecondaryIndex;
    vector<int> doctorAvailList;
    vector<int> appointmentAvailList;

    const string DOCTOR_FILE = "doctors.txt";
    const string DOCTOR_INDEX_FILE = "doctor.index";
    const string APPOINTMENT_FILE = "appointments.txt";
    const string APPOINTMENT_INDEX_FILE = "appointment.index";

    string readRecordFromFile(const string& fileName, int position);
    int static findAvailableSlot(vector<int>& availList, const string& fileName);
    void markDeleted(vector<int>& availList, int position, const string& fileName);
    string extractField(const string& record, int fieldIndex);

public:
    void displayMenu();
    void addDoctor(const string& doctorID, const string& name, const string& address);
    void addAppointment(const string& appointmentID, const string& doctorID, const string& date);
    void updateDoctor();
    void updateAppointment();
    void searchNameForQuary(string doctorID);
    void deleteDoctor();
    void deleteAppointment();
    void searchDoctorByID(string doctorID);
    void searchDoctorByName();
    void searchAppointmentsByID(string arg);
    void searchAppointmentsByDoctorID(string arg);
    void loadIndexes();
    void saveIndexes();
    void loadAvailList(vector<int>& availList, const string& fileName);
    void saveAvailList(const vector<int>& availList, const string& fileName);
    void processQuery(const string& query);

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
    cout << "9. Search Appointments by Appointment ID\n";
    cout << "10. Search Appointments by Doctor ID\n";
    cout << "11. Write Quary\n";
    cout << "12. Exit\n";
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
    if (doctorID.length() > 15 || name.length() > 30 || address.length() > 30) {
        cout << "Error: Input exceeds the maximum allowed length.\n";
        return;
    }
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
    doctorSecondaryIndex.Insert(name, doctorID);

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
    int pos = binarySearch(doctorPrimaryIndex, doctorID);
    if (pos == -1) {
        cout << "Doctor not found.\n";
        return;
    }
    string record = readRecordFromFile(DOCTOR_FILE, doctorPrimaryIndex[pos].second);
    size_t d1 = record.find('|');
    string id = record.substr(4, d1 - 4);
    size_t d2 = record.find('|', d1 + 1);
    string name = record.substr(d1 + 1, d2 - d1 - 1);
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

    DoctorNode* doctorIDs = doctorSecondaryIndex.Index[name].head;
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

            cout << "\n--- Doctor Details ---\n";
            cout << "Doctor ID: " << doctorID << "\n";
            cout << "Name: " << doctorName << "\n";
            cout << "Address: " << doctorAddress << "\n";
            cout << "-----------------------\n";
        }
        doctorIDs = doctorIDs->next;
    }
}




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

void HealthcareManagementSystem::deleteAppointment() {
    string appointmentID;
    cout << "Enter Appointment ID to delete: ";
    cin >> appointmentID;

    int pos = binarySearch(appointmentPrimaryIndex, appointmentID);
    if (pos == -1) {
        cout << "Appointment not found.\n";
        return;
    }
    int position = appointmentPrimaryIndex[pos].second;
    markDeleted(appointmentAvailList, position, APPOINTMENT_FILE);
    string record = readRecordFromFile(APPOINTMENT_FILE, position);
    size_t delim2 = record.find('|', record.find('|') + 1);
    string doctorID = record.substr(delim2 + 1);

    appointmentPrimaryIndex.erase(appointmentPrimaryIndex.begin() + pos);
    auto it = appointmentSecondaryIndex.Index.find(doctorID);
    if (it != appointmentSecondaryIndex.Index.end()) {
        AppointmentLinkedList& appointmentList = it->second;
        appointmentList.remove(appointmentID);
        if (appointmentList.head == nullptr) {
            appointmentSecondaryIndex.Index.erase(doctorID);
        }
    }
    appointmentSecondaryIndex.save();
    saveIndexes();
    cout << "Appointment deleted successfully.\n";
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
        appointmentFile.seekp(0, ios::end);
        position = appointmentFile.tellp();
        appointmentFile.close();
    }

    string fullRecord = appointmentID + "|" + date + "|" + doctorID;
    fstream appointmentFile(APPOINTMENT_FILE, ios::in | ios::out);
    appointmentFile.seekp(position, ios::beg);
    appointmentFile << setw(4) << setfill('0') << fullRecord.length() << fullRecord << "\n";
    appointmentFile.close();

    auto it = lower_bound(appointmentPrimaryIndex.begin(), appointmentPrimaryIndex.end(), make_pair(appointmentID, 0));
    appointmentPrimaryIndex.insert(it, {appointmentID, position});
    appointmentSecondaryIndex.insert(doctorID, appointmentID, date);

    appointmentSecondaryIndex.save();
    saveIndexes();
    cout << "Appointment added successfully.\n";
}

void HealthcareManagementSystem::updateAppointment() {
    string appointmentID, newDate, newDoctorID;
    cout << "Enter Appointment ID to update: ";
    cin >> appointmentID;

    int pos = binarySearch(appointmentPrimaryIndex, appointmentID);
    if (pos == -1) {
        cout << "Appointment not found.\n";
        return;
    }
    string record = readRecordFromFile(APPOINTMENT_FILE, appointmentPrimaryIndex[pos].second);
    size_t delim1 = record.find('|');
    size_t delim2 = record.find('|', delim1 + 1);
    string id = record.substr(0, delim1);
    string date = record.substr(delim1 + 1, delim2 - delim1 - 1);
    string doctorID = record.substr(delim2 + 1);
    cout << "Enter new appointment date (leave blank to skip): ";
    cin.ignore();
    getline(cin, newDate);
    if (!newDate.empty()) {
        date = newDate;
    }
    cout << "Enter new doctor ID (leave blank to skip): ";
    getline(cin, newDoctorID);
    if (!newDoctorID.empty() && newDoctorID != doctorID) {
        appointmentSecondaryIndex.Index[doctorID].remove(appointmentID);
        if (appointmentSecondaryIndex.Index[doctorID].head == nullptr) {
            appointmentSecondaryIndex.Index.erase(doctorID);
        }
        appointmentSecondaryIndex.insert(newDoctorID, appointmentID, date);
        doctorID = newDoctorID;
    }

    string updatedRecord = id + "|" + date + "|" + doctorID;
    fstream file(APPOINTMENT_FILE, ios::in | ios::out);
    if (!file) {
        cerr << "Error: Unable to open " << APPOINTMENT_FILE << "\n";
        return;
    }
    file.seekp(appointmentPrimaryIndex[pos].second, ios::beg);
    file << setw(4) << setfill('0') << updatedRecord.length() << updatedRecord << "\n";
    file.close();

    appointmentSecondaryIndex.save();
    saveIndexes();
    cout << "Appointment updated successfully.\n";
}

void HealthcareManagementSystem::searchAppointmentsByID(string arg = ""s) {
    string appointmentID;
    if (arg.empty()) {
        cout << "Enter Appointment ID to search: ";
        cin >> appointmentID;
    } else {
        appointmentID = arg;
    }
    int pos = binarySearch(appointmentPrimaryIndex, appointmentID);
    if (pos == -1) {
        cout << "Appointment not found.\n";
        return;
    }
    string record = readRecordFromFile(APPOINTMENT_FILE, appointmentPrimaryIndex[pos].second);
    if (record.empty()) { 
        cout << "Error: Unable to retrieve appointment record.\n";
        return;
    }
    size_t delim1 = record.find('|');
    size_t delim2 = record.find('|', delim1 + 1);
    string id = record.substr(0, delim1);
    string date = record.substr(delim1 + 1, delim2 - delim1 - 1);
    string docID = record.substr(delim2 + 1);
    cout << "\n--- Appointment Details ---\n";
    cout << "Appointment ID: " << id << "\n";
    cout << "Date: " << date << "\n";
    cout << "Doctor ID: " << docID << "\n";
    cout << "---------------------------\n";
}
void HealthcareManagementSystem::searchAppointmentsByDoctorID(string arg = ""s) {
    string doctorID = arg;
    if (doctorID.empty()) {
        cout << "Enter Doctor ID to search: ";
        cin >> doctorID;
    }
    auto it = appointmentSecondaryIndex.Index.find(doctorID);
    if (it == appointmentSecondaryIndex.Index.end() || it->second.head == nullptr) {
        cout << "No appointments found for Doctor ID: " << doctorID << endl;
        return;
    }
    cout << "\nAppointments for Doctor ID: " << doctorID << "\n";
    AppointmentNode* current = it->second.head;
    while (current) {
        int pos = binarySearch(appointmentPrimaryIndex, current->appointmentID);
        if (pos != -1) {
            string record = readRecordFromFile(APPOINTMENT_FILE, appointmentPrimaryIndex[pos].second);
            if (!record.empty()) {
                string appointmentID = extractField(record, 0);
                string date = extractField(record, 1);
                string docID = extractField(record, 2);

                cout << "\n--- Appointment Details ---\n";
                cout << "Appointment ID: " << appointmentID << "\n";
                cout << "Date: " << date << "\n";
                cout << "Doctor ID: " << docID << "\n";
                cout << "---------------------------\n";
            } else {
                cout << "Error: Unable to read record for Appointment ID: " << current->appointmentID << "\n";
            }
        } else {
            cout << "Warning: Appointment ID " << current->appointmentID << " not found in primary index.\n";
        }
        current = current->next;
    }
}


string HealthcareManagementSystem::extractField(const string& record, int fieldIndex) {
    size_t start = 0, end = 0;
    int currentIndex = 0;
    while ((end = record.find('|', start)) != string::npos) {
        if (currentIndex == fieldIndex) {
            return record.substr(start, end - start);
        }
        start = end + 1;
        currentIndex++;
    }
    if (currentIndex == fieldIndex) 
        return record.substr(start);
    
    return ""; 
}

void HealthcareManagementSystem::loadIndexes() {
    fstream doctorIndexFile(DOCTOR_INDEX_FILE, ios::in);
    if (doctorIndexFile.is_open()) {
        string record;
        while (getline(doctorIndexFile, record)) {
            stringstream ss(record);
            string doctorID;
            int position;
            getline(ss, doctorID, '|');
            ss >> position;
            doctorPrimaryIndex.push_back({doctorID, position});
        }
        doctorIndexFile.close();
    }
    doctorSecondaryIndex.load();
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
    appointmentSecondaryIndex.load();  
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

    doctorSecondaryIndex.save();
    appointmentSecondaryIndex.save();

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
    doctorSecondaryIndex.Insert(newName, doctorID);
    saveIndexes();
    doctorSecondaryIndex.save();
    cout << "Doctor record updated successfully.\n";
}
void HealthcareManagementSystem::searchNameForQuary(string doctorID) {
    for (auto& entry : doctorSecondaryIndex.Index) {
        // Get the list of doctor IDs for a particular doctor name
        DoctorNode* doctorIDs = entry.second.head;  // The list of doctor IDs for this name

        while (doctorIDs) {
            // If we find the doctorID in the secondary index, we found the name
            if (doctorIDs->doctorID == doctorID) {
                // Now that we found the doctor ID, extract the name from the current entry
                string doctorName = entry.first;  // The key of the map is the doctorName

                cout << doctorName << "\n";
                return;
            }
            doctorIDs = doctorIDs->next;  // Move to the next doctor ID in the linked list
        }
    }

    // If we reach here, no doctor with the specified ID was found in the secondary index
    cout << "No doctor found with the ID: " << doctorID << endl;
}

void HealthcareManagementSystem::processQuery(const string& query) {
    // Preprocess the query: remove spaces and ignore case permutations
    string processedQuery = ignoreKeysCasePermutations(query);
    processedQuery.erase(remove(processedQuery.begin(), processedQuery.end(), ' '), processedQuery.end());

    // Validate the query
    while (!isValidQuery(processedQuery)) {
        cout << "Invalid query. Please try again.\n";
        cout << "Enter your query: ";
        getline(cin, processedQuery);
        processedQuery = ignoreKeysCasePermutations(processedQuery);
        processedQuery.erase(remove(processedQuery.begin(), processedQuery.end(), ' '), processedQuery.end());
    }

    // Extract the field name after 'select' (between 'select' and 'from')
    size_t selectPos = processedQuery.find("select") + 6;  // Skip 'select' (length = 6)
    size_t fromPos = processedQuery.find("from");

    // Extract the field (e.g., 'doctorname' or '*')
    string fieldName = processedQuery.substr(selectPos, fromPos - selectPos);

    // Extract table name (between 'from' and 'where')
    size_t wherePos = processedQuery.find("where");
    string tableName = processedQuery.substr(fromPos + 4, wherePos - fromPos - 4);

    // Extract field name (between 'where' and '=')
    size_t fieldStartPos = wherePos + 5;  // Skip 'where'
    size_t equalPos = processedQuery.find('=');
    string queryFieldName = processedQuery.substr(fieldStartPos, equalPos - fieldStartPos);

    // Extract field value (between the single quotes)
    size_t firstQuotePos = processedQuery.find('\'') + 1;  // Skip the first quote
    size_t secondQuotePos = processedQuery.find('\'', firstQuotePos);
    string fieldValue = processedQuery.substr(firstQuotePos, secondQuotePos - firstQuotePos);

    if (tableName == "doctors") {
        if (queryFieldName == "doctorid"&& fieldName == "*") {
            searchDoctorByID(fieldValue);
        } else if (queryFieldName == "doctorid" &&fieldName == "doctorname") {
            cout << "dddd";
            searchNameForQuary(fieldValue);
        }
    } else if (tableName == "appointments") {
        if (queryFieldName == "appointmentid") {
            searchAppointmentsByID(fieldValue);
        } else if (queryFieldName == "doctorid") {
            searchAppointmentsByDoctorID(fieldValue);
        }
    }
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
            case 6: {  // Delete Appointment
                system.deleteAppointment();
                break;
            }
            case 7: {//search doctor by id
                string doctorID;
                cout << "Enter Doctor ID to search: ";
                cin >> doctorID;
                system.searchDoctorByID(doctorID);
                break;
            }
            case 8: {// search Doctor By Name
                system.searchDoctorByName();
                break;
            }
            case 9: {  // Search Appointments by Appointment ID
                system.searchAppointmentsByID();
                break;
            }
            case 10:{
                system.searchAppointmentsByDoctorID();
                break;
            }

            case 11: {
                string query;
                cout << "Enter your query: ";
                cin.ignore();
                getline(cin, query);
                system.processQuery(query);
                break;
            }
            case 12: {
                cout << "Exiting...\n";
                std::exit(0);
                // no need to break here
            }
            default: {
                cout << "Invalid choice. Please try again.\n";
                break;
            }
        }
    } while (choice>0 && choice<13);
    system.saveIndexes();
    return 0;
}