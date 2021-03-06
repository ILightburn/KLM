#ifndef KARAOKEFILE_H
#define KARAOKEFILE_H

#include <QString>
#include <vector>
#include <QObject>
#include <QDirIterator>
#include <QSharedPointer>
#include <QVariant>

struct NamingPattern {
    QString name{"SongID - Artist - Title"};
    QString sep{" - "};
    int songIdPos{0};
    int artistPos{1};
    int titlePos{2};

    [[maybe_unused]] static NamingPattern PatternSAT() {
        return NamingPattern{"SongID - Artist - Title", " - ", 0, 1, 2};
    }

    [[maybe_unused]] static NamingPattern PatternSTA() {
        return NamingPattern{"SongID - Title - Artist", " - ", 0, 2, 1};
    }

    [[maybe_unused]] static NamingPattern PatternATS() {
        return NamingPattern{"Artist - Title - SongID", " - ", 2, 0, 1};
    }

    [[maybe_unused]] static NamingPattern PatternTAS() {
        return NamingPattern{"Title - Artist - SongID", " - ", 2, 1, 0};
    }

    [[maybe_unused]] static NamingPattern PatternAT() {
        return NamingPattern{"Artist - Title", " - ", -1, 0, 1};
    }

    [[maybe_unused]] static NamingPattern PatternTA() {
        return NamingPattern{"Title - Artist", " - ", -1, 1, 0};
    }
};




class KaraokeFile : public QObject {
    Q_OBJECT
private:
    enum class ProcessingMode {
        AudioGZip,
        AudioG,
        VideoFile
    };
    QString m_artist;
    QString m_title;
    QString m_atCombo;
    QString m_songid;
    QString m_path;
    uint64_t m_duration{0};
    uint32_t m_crc32{0};
    uint m_audioBitrate{0};
    bool m_fileScanned{false};
    NamingPattern m_namingPattern;
    static std::vector<std::string> supportedAFileExtensions() { return {".mp3", ".ogg", ".wav", ".flac", ".m4a"}; }
    void scanFile();
    void scanZipFile();
    void scanCdgFile();
    void scanVideoFile();
    ProcessingMode m_procMode{ProcessingMode::AudioGZip};
    static uint64_t getDurationFromCdgSize(uint64_t size) {
        return ((size / 96) / 75) * 1000;
    }

    void applyNamingPattern();

public:
    enum class ErrorCode{
        OK,
        BadZip,
        MissingCdg,
        MissingAudio,
        ZeroByteCdg,
        ZeroByteAudio,
        ZeroByte
    };
    static std::vector<std::string> supportedKFileExtensions() { return {"cdg", "zip", "mp4", "mkv", "avi", "ogv"}; }
    [[nodiscard]] std::string getErrCodeStr() const;
    [[nodiscard]] ErrorCode getErrorCode() const {return m_errorCode;}
    explicit KaraokeFile(QString path, NamingPattern pattern, QObject *parent = nullptr);
    [[nodiscard]] QString artist() const;
    [[nodiscard]] QString title() const { return m_title; }
    [[nodiscard]] QString atCombo() const { return m_atCombo; }
    [[nodiscard]] QString songid() const { return m_songid; }
    [[nodiscard]] QString path() const { return m_path; }
    uint32_t crc();
    uint64_t duration();
    uint getBitrate();
    ErrorCode m_errorCode{ErrorCode::OK};
    QString findAudioFileForCdg();
    [[nodiscard]] static QString findAudioFileForCdg(const QString& cdgFilePath);

    void setNamingPattern(const NamingPattern &pattern);
};

namespace KLM {
typedef QVector<QSharedPointer<KaraokeFile>> KaraokeFileList;
static KaraokeFileList getKaraokeFiles(const QString &path, NamingPattern &pattern) {
    KaraokeFileList kFiles;
    QDirIterator it(path, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        auto ext = it.fileInfo().completeSuffix().toLower();
        for (const auto &sExt : KaraokeFile::supportedKFileExtensions())
            if (ext == sExt.c_str()) {
                kFiles.push_back(QSharedPointer<KaraokeFile>(new KaraokeFile(it.filePath(), pattern)));
            }
    }
    return kFiles;
}
}

class KaraokePath : public QObject {
    Q_OBJECT
    QString m_path;
    NamingPattern m_pattern;
    KLM::KaraokeFileList m_files;

public:
    explicit KaraokePath(QString path, NamingPattern pattern, QObject *parent = nullptr);
    KLM::KaraokeFileList& files(const bool forceRefresh = false);
    const QString &path() const;
    const NamingPattern &pattern() const;
    void setPattern(const NamingPattern &newPattern);
};


namespace KLM {
    typedef QVector<QSharedPointer<KaraokePath>> KaraokePathList;
}


#endif // KARAOKEFILE_H
