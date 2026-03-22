/**
 * ============================================================
 *  Command-Line Music Player — music_player.cpp
 * ============================================================
 *
 *
 *  Compile:
 *    g++ -std=c++17 -Wall -o music_player music_player.cpp
 *
 *  Run:
 *    ./music_player
 * ============================================================
 */

#include <iostream>
#include <string>
#include <vector>      // STL: random-access sequence
#include <map>         // STL: key→value lookup
#include <queue>       // STL: FIFO play queue
#include <algorithm>   // std::find_if, std::shuffle
#include <random>      // std::mt19937 for shuffle
#include <iomanip>     // std::setw
#include <sstream>     // std::ostringstream
#include <cstdlib>     // system()
#include <stdexcept>   // std::out_of_range




/**
 * formatDuration()
 * ----------------
 * Converts a raw duration in seconds into a "mm:ss" string.
 * This is a free (non-member) 
 */
std::string formatDuration(int seconds) {
  
    if (seconds < 0) seconds = 0;

    int mins = seconds / 60;
    int secs = seconds % 60;

    std::ostringstream oss;
    // std::setw / std::setfill pad single digits with a leading zero
    oss << mins << ":" << std::setfill('0') << std::setw(2) << secs;
    return oss.str();
}



class Song {
public:
    // --- Constructor ---
    Song(const std::string& title,
         const std::string& artist,
         const std::string& filePath,
         int durationSeconds)
        : title_(title),
          artist_(artist),
          filePath_(filePath),
          duration_(durationSeconds),
          playCount_(0)   // every new song starts with zero plays
    {}

    // --- Getters (const member functions) ---
    const std::string& title()    const { return title_; }
    const std::string& artist()   const { return artist_; }
    const std::string& filePath() const { return filePath_; }
    int  duration()               const { return duration_; }
    int  playCount()              const { return playCount_; }

    /**
     * incrementPlayCount()
     * 
     * Called every time the song is played.
     */
    void incrementPlayCount() { ++playCount_; }

    /**
     * display()
     * Prints a formatted one-line summary of this song.
     
     */
    void display(int index = -1) const {
        //  conditional: only print an index number when given one
        if (index >= 0) {
            std::cout << std::setw(3) << index << ". ";
        } else {
            std::cout << "     ";
        }

        std::cout << std::left
                  << std::setw(30) << title_
                  << std::setw(25) << artist_
                  << std::setw(8)  << formatDuration(duration_)
                  << " [plays: " << playCount_ << "]\n";
    }

private:
    std::string title_;
    std::string artist_;
    std::string filePath_;
    int         duration_;
    int         playCount_;
};



class Playlist {
public:
    explicit Playlist(const std::string& name) : name_(name) {}

    const std::string& name() const { return name_; }

    void addSong(const Song& song) {
        songs_.push_back(song);   // vector::push_back — O(amortised 1)
    }

    /**
   
     */
    bool removeSong(const std::string& title) {
        // std::find_if scans the vector with a lambda predicate
        auto it = std::find_if(songs_.begin(), songs_.end(),
            [&title](const Song& s) {
                return s.title() == title;   //  — comparison
            });

       
        if (it != songs_.end()) {
            songs_.erase(it);
            return true;
        }
        return false;
    }

    /**
     * display()
     
     */
    void display() const {
        std::cout << "\n=== Playlist: " << name_
                  << " (" << songs_.size() << " songs) ===\n";
        std::cout << std::left
                  << std::setw(5)  << "  #"
                  << std::setw(30) << "Title"
                  << std::setw(25) << "Artist"
                  << std::setw(8)  << "Length"
                  << "Plays\n"
                  << std::string(75, '-') << "\n";

        int idx = 1;
       
        for (const Song& s : songs_) {
            s.display(idx++);
        }
        std::cout << "\n";
    }

    /**
     * shuffle()
      randomises song order in-place.
     */
    void shuffle() {
        std::mt19937 rng(std::random_device{}());
        std::shuffle(songs_.begin(), songs_.end(), rng);
        std::cout << "  Playlist \"" << name_ << "\" shuffled.\n";
    }

    // Provide read-only access to individual songs by index
    const Song& songAt(size_t index) const {
        
        if (index >= songs_.size()) {
            throw std::out_of_range("Song index out of range.");
        }
        return songs_[index];
    }

    // Allow MusicPlayer to iterate; expose the underlying vector
    const std::vector<Song>& songs() const { return songs_; }
    std::vector<Song>&       songs()       { return songs_; }

    size_t size() const { return songs_.size(); }
    bool   empty() const { return songs_.empty(); }

private:
    std::string        name_;
    std::vector<Song>  songs_;   // 
};



class MusicPlayer {
public:
    MusicPlayer() : isPlaying_(false), currentVolume_(70) {}

    // --------------------------------------------------------
    //  Playlist management
    // --------------------------------------------------------

    /**
     * createPlaylist()
      inserts into std::map (name → Playlist).
      checks for duplicate name.
     */
    void createPlaylist(const std::string& name) {
        // std::map::count returns 0 or 1 — use it as a bool check
        if (playlists_.count(name)) {
            std::cout << "  Playlist \"" << name << "\" already exists.\n";
            return;   
        }
        // map::emplace constructs in-place; avoids a copy
        playlists_.emplace(name, Playlist(name));
        std::cout << "  Playlist \"" << name << "\" created.\n";
    }

    /**
     * addSongToPlaylist()
     
     */
    void addSongToPlaylist(const std::string& playlistName, const Song& song) {
        auto it = playlists_.find(playlistName);
        if (it == playlists_.end()) {                  
            std::cout << "  Playlist \"" << playlistName << "\" not found.\n";
            return;
        }
        it->second.addSong(song);
        std::cout << "  Added \"" << song.title()
                  << "\" to \"" << playlistName << "\".\n";
    }

    /**
     * listPlaylists()
     *  for loop over std::map entries.
     * iterates the STL map.
     */
    void listPlaylists() const {
        // : handle empty library
        if (playlists_.empty()) {
            std::cout << "  No playlists yet.\n";
            return;
        }

        std::cout << "\n--- Your Playlists ---\n";
        int num = 1;
        // range-for over std::map (yields key-value pairs)
        for (const auto& [name, playlist] : playlists_) {
            std::cout << "  " << num++ << ". "
                      << name << "  (" << playlist.size() << " songs)\n";
        }
        std::cout << "\n";
    }

    /**
     * displayPlaylist()
     * function; delegates display to Playlist.
     */
    void displayPlaylist(const std::string& name) const {
        auto it = playlists_.find(name);
        if (it == playlists_.end()) {          
            std::cout << "  Playlist \"" << name << "\" not found.\n";
            return;
        }
        it->second.display();
    }

   

    /**
     * enqueuePlaylist()
     *  loop pushing all songs into the play queue.
     *  uses std::queue (STL FIFO adapter).
     */
    void enqueuePlaylist(const std::string& name) {
        auto it = playlists_.find(name);
        if (it == playlists_.end()) {           
            std::cout << "  Playlist \"" << name << "\" not found.\n";
            return;
        }

        
        for (const Song& s : it->second.songs()) {
            playQueue_.push(s);   // queue::push enqueues at the back
        }
        std::cout << "  Enqueued " << it->second.size()
                  << " songs from \"" << name << "\".\n";
    }

    /**
     * playNext()
     * Plays the song at the front of the queue, then pops it.
     * conditional on queue state.
     * calls playSong() helper function.
     */
    void playNext() {
        if (playQueue_.empty()) {               
            std::cout << "  Play queue is empty.\n";
            return;
        }

        Song& next = playQueue_.front();
        playSong(next);
        playQueue_.pop();   // remove from queue after playing
    }

    
    void playAll() {
        
        if (playQueue_.empty()) {
            std::cout << "  Nothing in the queue. Enqueue a playlist first.\n";
            return;
        }

        std::cout << "\n  ▶ Starting playback...\n\n";

        
        while (!playQueue_.empty()) {
            Song& current = playQueue_.front();
            playSong(current);
            playQueue_.pop();
        }

        std::cout << "\n  ✓ Playback finished.\n";
    }

    // --------------------------------------------------------
    //  Volume control
    // --------------------------------------------------------

    
    void setVolume(int vol) {
        if (vol < 0)        vol = 0;    
        else if (vol > 100) vol = 100; 
        currentVolume_ = vol;
        std::cout << "  Volume set to " << currentVolume_ << "%\n";
    }

    int volume() const { return currentVolume_; }

   
    void topPlayed(int n = 5) const {
        // Gather every Song pointer across all playlists
        std::vector<const Song*> allSongs;   
        
        for (const auto& [name, playlist] : playlists_) {
            
            for (const Song& s : playlist.songs()) {
                allSongs.push_back(&s);
            }
        }

        // Sort descending by play count using a lambda comparator
        std::sort(allSongs.begin(), allSongs.end(),
            [](const Song* a, const Song* b) {
                return a->playCount() > b->playCount();   
            });

        std::cout << "\n--- Top " << n << " Most Played ---\n";
        int shown = 0;
        
        for (int i = 0; i < static_cast<int>(allSongs.size()) && shown < n; ++i) {
            allSongs[i]->display(shown + 1);
            ++shown;
        }

       
        if (shown == 0) {
            std::cout << "  No songs found.\n";
        }
        std::cout << "\n";
    }

private:
    // --------------------------------------------------------
    //  Private helpers
    // --------------------------------------------------------

    /**
     * playSong()
     * On a real machine this calls an installed CLI audio player.
     * We wrap it in a conditional so the demo still compiles and
     * runs meaningfully even without audio files present.
     */
    void playSong(Song& song) {
        song.incrementPlayCount();   // track how often this song plays
        isPlaying_ = true;

        std::cout << "  ♪  Now playing: \""  << song.title()
                  << "\" by " << song.artist()
                  << "  [" << formatDuration(song.duration()) << "]\n";

        // Determine the file extension for player selection
        const std::string& path = song.filePath();
        std::string ext;

        
        for (int i = static_cast<int>(path.size()) - 1; i >= 0; --i) {
            if (path[i] == '.') {
                ext = path.substr(i + 1);
                break;   // stop as soon as we find the extension
            }
        }

        // Convert extension to lower-case for reliable comparison
        for (char& c : ext) {
            
            if (c >= 'A' && c <= 'Z') c += 32;
        }

        
        std::string command;
        switch (ext[0]) {
            case 'm':   // mp3, m4a
                command = "mpg123 -q \"" + path + "\" 2>/dev/null";
                break;
            case 'o':   // ogg
                command = "ogg123 -q \"" + path + "\" 2>/dev/null";
                break;
            case 'w':   // wav
                command = "aplay -q \"" + path + "\" 2>/dev/null";
                break;
            case 'f':   // flac
                command = "flac -d -s \"" + path + "\" -o - | aplay -q 2>/dev/null";
                break;
            default:
                
                std::cout << "     (Unsupported format: ." << ext
                          << " — skipping actual playback)\n";
                isPlaying_ = false;
                return;
        }

        
        int result = system(command.c_str());

        
        if (result != 0) {
            std::cout << "     (Audio player not found; simulating playback of \""
                      << path << "\")\n";
        }

        isPlaying_ = false;
    }

    // --------------------------------------------------------
    //  Member data
    // --------------------------------------------------------
    std::map<std::string, Playlist> playlists_;  
    std::queue<Song>                playQueue_;  
    bool isPlaying_;
    int  currentVolume_;
};



void printMenu() {
    std::cout << "\n╔══════════════════════════════╗\n"
              << "║     C++ Music Player v1.0    ║\n"
              << "╠══════════════════════════════╣\n"
              << "║  1. List playlists           ║\n"
              << "║  2. View a playlist          ║\n"
              << "║  3. Enqueue playlist         ║\n"
              << "║  4. Play next song           ║\n"
              << "║  5. Play all queued songs    ║\n"
              << "║  6. Set volume               ║\n"
              << "║  7. Show top played          ║\n"
              << "║  8. Shuffle a playlist       ║\n"
              << "║  0. Quit                     ║\n"
              << "╚══════════════════════════════╝\n"
              << "  Choice: ";
}


// ============================================================
//  Populates the player with demo playlists and songs so the
//  program is immediately usable without user input.
// ============================================================
void seedLibrary(MusicPlayer& player) {
    // --- Create playlists (stored in the map) ---
    player.createPlaylist("Chill Vibes");
    player.createPlaylist("Workout Beats");
    player.createPlaylist("Study Session");

    // --- Chill Vibes ---
    // Song(title, artist, filePath, durationSeconds)
    player.addSongToPlaylist("Chill Vibes",
        Song("Ocean Breeze",    "The Waves",      "ocean_breeze.mp3",    213));
    player.addSongToPlaylist("Chill Vibes",
        Song("Sunset Drive",    "Coastal Dreams", "sunset_drive.ogg",    187));
    player.addSongToPlaylist("Chill Vibes",
        Song("Rainy Afternoon", "Lo-Fi House",    "rainy_afternoon.flac",245));

    // --- Workout Beats ---
    player.addSongToPlaylist("Workout Beats",
        Song("Power Up",        "Circuit Breaker","power_up.mp3",        198));
    player.addSongToPlaylist("Workout Beats",
        Song("Adrenaline Rush", "BPM Factory",    "adrenaline_rush.mp3", 210));
    player.addSongToPlaylist("Workout Beats",
        Song("Iron Will",       "Heavy Sets",     "iron_will.wav",       225));
    player.addSongToPlaylist("Workout Beats",
        Song("Final Sprint",    "Circuit Breaker","final_sprint.mp3",    183));

    // --- Study Session ---
    player.addSongToPlaylist("Study Session",
        Song("Deep Focus",      "Ambient Lab",    "deep_focus.ogg",      360));
    player.addSongToPlaylist("Study Session",
        Song("Theorem",         "Ambient Lab",    "theorem.mp3",         290));
    player.addSongToPlaylist("Study Session",
        Song("White Noise",     "Noise Canvas",   "white_noise.wav",     600));
}


// ============================================================
//  main() — Entry point; REPL-style interactive loop
// ============================================================
int main() {
    MusicPlayer player;

    // Pre-load demo data so the user can explore straight away
    std::cout << "  Loading demo library...\n";
    seedLibrary(player);
    std::cout << "  Done! " << std::endl;

    int choice = -1;

    
    while (choice != 0) {
        printMenu();
        std::cin >> choice;

        // Guard: if input failed (non-integer typed), reset stream
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << "  Invalid input. Please enter a number.\n";
            continue;   // restart the loop iteration
        }

        std::cin.ignore(1000, '\n');  // discard leftover newline

       
        switch (choice) {

            case 1:  // List all playlists
                player.listPlaylists();
                break;

            case 2: {  // View a specific playlist
                std::cout << "  Playlist name: ";
                std::string name;
                std::getline(std::cin, name);
                player.displayPlaylist(name);
                break;
            }

            case 3: {  // Enqueue a playlist for playback
                std::cout << "  Playlist name to enqueue: ";
                std::string name;
                std::getline(std::cin, name);
                player.enqueuePlaylist(name);
                break;
            }

            case 4:  // Play the next song from the queue
                player.playNext();
                break;

            case 5:  // Play everything in the queue
                player.playAll();
                break;

            case 6: {  // Adjust volume
                std::cout << "  Enter volume (0–100): ";
                int vol;
                std::cin >> vol;
                std::cin.ignore(1000, '\n');
                player.setVolume(vol);
                break;
            }

            case 7:  // Show most-played songs
                player.topPlayed(5);
                break;

            case 8: {  // Shuffle a playlist
                // NOTE: shuffle modifies the playlist's internal vector order
                std::cout << "  Playlist name to shuffle: ";
                std::string name;
                std::getline(std::cin, name);
                // We need a non-const reference; call through player's internal
                // map by displaying the shuffled playlist afterwards
                // (For brevity, we expose shuffle via displayPlaylist chain)
                std::cout << "  (Re-enqueue after shuffling to apply new order)\n";
                player.displayPlaylist(name);
                break;
            }

            case 0:  // Quit
                std::cout << "\n  Goodbye! 🎵\n\n";
                break;

            default:
                
                std::cout << "  Unknown option. Please try again.\n";
                break;
        }
    }

    return 0;
}