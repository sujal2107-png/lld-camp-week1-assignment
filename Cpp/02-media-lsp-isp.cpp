// 02-media-lsp-isp.cpp
// Messy starter: Fat interface + LSP surprises (violates ISP + LSP)

#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

class IPlayable{
public:    
    virtual ~IPlayable() = default;
    virtual void play(const string& source) = 0;
};
class IPausable{
public:     
    virtual ~IPausable() = default;
    virtual void pause() = 0;
};
class IRecordable{
public:     
    virtual ~IRecordable() = default;
    virtual void record(const string& destination) = 0; 
};
class ILiveStreamable{
public:     
    virtual ~ILiveStreamable() = default;
    virtual void streamLive(const string& url) = 0; 
};
class IDownloadable{
public:     
    virtual ~IDownloadable() = default;
    virtual void download(const string& url) = 0; 
};


class AudioPlayer : public IPlayable,IPausable,IDownloadable {
    bool playing{false};
public:
    void play(const string& source) override { (void)source; playing = true; }
    void pause() override { playing = false; }
    void download(const string& url)override{ (void)url; /* pretend */ }
    bool isPlaying() const { return playing; }
};

class CameraStreamPlayer : public IPausable,IRecordable,ILiveStreamable {
    bool liveStarted{false};
    bool playing{false};
public:
    void play(const string& src){
        (void)src;
        // Surprise: needs streamLive first for “real” play
        if (!liveStarted) {
            cout << "[WARN] playing without live stream started.\n";
        }
        playing = true;
    }
    void pause() override { playing = false; }
    void record(const string& dest) override { (void)dest; /* pretend */ }
    void streamLive(const string& url) override { (void)url; liveStarted = true; }
    bool isPlaying() const { return playing; }
    bool isLive() const { return liveStarted; }
};

int main() {
    AudioPlayer ap;
    ap.play("song.mp3");
    cout << "Audio playing: " << boolalpha << ap.isPlaying() << "\n";
    ap.pause();

    CameraStreamPlayer cam;      // warning surprise
    cam.streamLive("rtsp://camera"); // required order
    cam.play("rtsp://camera");
    return 0;
}
