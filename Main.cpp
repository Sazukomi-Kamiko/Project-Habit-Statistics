// habit.cpp - Habit tracker ala HabitKit versi C++ (terminal)
// Compile: g++ -std=c++17 habit.cpp -o habit
// Jalankan: ./habit   (Windows: habit.exe)
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <ctime>
using namespace std;

const int DAYS = 70;                 // 10 minggu (7 baris x 10 kolom)
const char* FILE_NAME = "habits.txt";
const char* COLORS[] = {"32", "34", "33", "35", "36"};  // kode warna ANSI

struct Habit {
    string name;
    set<string> done;                // kumpulan tanggal "YYYY-MM-DD"
};

// Tanggal "offset" hari yang lalu (0 = hari ini)
string dateKey(int daysAgo) {
    time_t t = time(nullptr) - (time_t)daysAgo * 86400;
    tm* lt = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", lt);
    return buf;
}

int getStreak(const Habit& h) {
    int streak = 0, i = h.done.count(dateKey(0)) ? 0 : 1;  // hari ini belum dicentang -> mulai dari kemarin
    while (h.done.count(dateKey(i))) { streak++; i++; }
    return streak;
}

// ---------- Simpan / muat ----------
void save(const vector<Habit>& habits) {
    ofstream f(FILE_NAME);
    for (auto& h : habits) {
        f << h.name << "|";
        bool first = true;
        for (auto& d : h.done) { f << (first ? "" : ",") << d; first = false; }
        f << "\n";
    }
}

vector<Habit> load() {
    vector<Habit> habits;
    ifstream f(FILE_NAME);
    string line;
    while (getline(f, line)) {
        size_t bar = line.find('|');
        if (bar == string::npos) continue;
        Habit h;
        h.name = line.substr(0, bar);
        stringstream ss(line.substr(bar + 1));
        string d;
        while (getline(ss, d, ',')) if (!d.empty()) h.done.insert(d);
        habits.push_back(h);
    }
    return habits;
}

// ---------- Tampilan ----------
void show(const vector<Habit>& habits) {
    if (habits.empty()) { cout << "\nBelum ada habit. Pilih 1 untuk menambah.\n"; return; }
    for (size_t n = 0; n < habits.size(); n++) {
        const Habit& h = habits[n];
        const char* color = COLORS[n % 5];
        cout << "\n[" << n + 1 << "] " << h.name << "   🔥 " << getStreak(h) << "\n";
        // 7 baris x 10 kolom; kolom paling kanan = minggu terbaru
        for (int row = 0; row < 7; row++) {
            cout << "  ";
            for (int col = 0; col < DAYS / 7; col++) {
                int idx = col * 7 + row;          // 0 = paling lama
                int daysAgo = DAYS - 1 - idx;
                bool isDone = h.done.count(dateKey(daysAgo));
                if (isDone) cout << "\033[" << color << "m■\033[0m ";
                else        cout << "\033[90m□\033[0m ";
            }
            cout << "\n";
        }
    }
}

int askIndex(const vector<Habit>& habits) {
    cout << "Nomor habit: ";
    int n; cin >> n;
    cin.ignore(1000, '\n');
    if (n < 1 || n > (int)habits.size()) { cout << "Nomor tidak valid.\n"; return -1; }
    return n - 1;
}

int main() {
    vector<Habit> habits = load();
    while (true) {
        cout << "\n===== HABIT TRACKER =====";
        show(habits);
        cout << "\n1) Tambah habit\n2) Centang/batal hari ini\n"
                "3) Centang/batal hari lalu\n4) Hapus habit\n0) Keluar\nPilih: ";
        int menu;
        if (!(cin >> menu)) break;
        cin.ignore(1000, '\n');

        if (menu == 0) break;
        else if (menu == 1) {
            cout << "Nama habit: ";
            string name; getline(cin, name);
            if (!name.empty()) habits.push_back({name, {}});
        }
        else if (menu == 2 || menu == 3) {
            int i = askIndex(habits);
            if (i < 0) continue;
            int ago = 0;
            if (menu == 3) {
                cout << "Berapa hari yang lalu (1-" << DAYS - 1 << "): ";
                cin >> ago; cin.ignore(1000, '\n');
                if (ago < 0 || ago >= DAYS) { cout << "Di luar jangkauan.\n"; continue; }
            }
            string k = dateKey(ago);
            if (habits[i].done.count(k)) habits[i].done.erase(k);
            else habits[i].done.insert(k);
        }
        else if (menu == 4) {
            int i = askIndex(habits);
            if (i >= 0) habits.erase(habits.begin() + i);
        }
        save(habits);
    }
    cout << "Sampai jumpa!\n";
    return 0;
}
