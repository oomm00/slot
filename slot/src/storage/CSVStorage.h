#pragma once

#include <string>
#include <vector>
#include <fstream>

#include "Bet.h"
#include "GameRecord.h"
#include "Player.h"
#include "StrategyReport.h"

/// Reusable persistence layer for CSV read/write of all system entities.
///
/// Design:
///   - All save/load operations are O(n) sequential file scans.
///   - No random access — CSV is inherently sequential; this is optimal.
///   - Memory: O(1) aux per row during load; O(n) for the result vector.
///   - Malformed rows are skipped; partial results are returned.
///   - If the file does not exist, save creates it; load returns empty.
class CSVStorage {
public:
    /// Construct with a data directory path (default: "data").
    /// All read/write operations use files within this directory.
    explicit CSVStorage(std::string dataDir = "data");
public:
    // ── Players ──────────────────────────────────────────────────────
    bool savePlayers(const std::vector<player>& players);
    std::vector<player> loadPlayers();

    // ── Bets ─────────────────────────────────────────────────────────
    bool saveBets(const std::vector<Bet>& bets);
    std::vector<Bet> loadBets();

    // ── Game Records ─────────────────────────────────────────────────
    bool saveGames(const std::vector<GameRecord>& games);
    std::vector<GameRecord> loadGames();

    // ── Strategy Reports ─────────────────────────────────────────────
    bool saveReports(const std::vector<strategyreport>& reports);
    std::vector<strategyreport> loadReports();

private:
    // ── Reusable CSV helpers ─────────────────────────────────────────

    /// Open a file for writing. Creates the file if it does not exist.
    bool openOutput(const std::string& path, std::ofstream& out) const;

    /// Open a file for reading. Returns false if file is missing.
    bool openInput(const std::string& path, std::ifstream& in) const;

    /// Write a CSV header line.
    void writeHeader(std::ofstream& out, const std::string& header);

    /// Split a CSV line into fields by comma.
    /// Handles basic quoting: fields wrapped in " " are kept intact.
    static std::vector<std::string> splitLine(const std::string& line);

    /// Read the header and verify it matches expected columns.
    /// Returns false if header is missing or does not match.
    bool readHeader(std::ifstream& in, const std::string& expected);

    // ── Type serialization helpers ───────────────────────────────────

    /// Serialize a time_point to epoch-seconds string.
    static std::string timePointToString(
        const std::chrono::system_clock::time_point& tp);

    /// Deserialize epoch-seconds string to time_point.
    static std::chrono::system_clock::time_point stringToTimePoint(
        const std::string& s);

    /// Serialize year_month_day to "YYYY-MM-DD".
    static std::string yearMonthDayToString(
        const std::chrono::year_month_day& ymd);

    /// Deserialize "YYYY-MM-DD" to year_month_day.
    static std::chrono::year_month_day stringToYearMonthDay(
        const std::string& s);

    /// Join a vector of strings with a delimiter.
    static std::string join(const std::vector<std::string>& parts,
                            char delim);

    /// Split a string by a delimiter into a vector.
    static std::vector<std::string> split(const std::string& s,
                                          char delim);

    /// Build a full file path from a relative filename.
    std::string filePath(const std::string& filename) const;

    std::string dataDir_;
};

// ── Complexity (for DAA documentation) ──────────────────────────────
//
// save*(vector<T>)   — O(n) time, O(n) space (output buffer)
//   Writing is a single sequential pass through the vector. Each field
//   is formatted and written to the stream immediately. The OS buffers
//   the write, so aux memory is O(1) per row.
//
// load*()            — O(n) time, O(n) space (result vector)
//   A single sequential pass over the file. Each row is parsed,
//   validated, and the object is appended to the result. Malformed
//   rows are O(1) to skip. The result vector holds all parsed objects.
//
// Why sequential processing is optimal:
//   CSV is a row-oriented, append-only format with no index structure.
//   Random access would require O(log n) seeks (filesystem) plus
//   O(n) scan to locate row boundaries — worse than a single linear
//   pass. Sequential I/O also maximizes disk throughput via read-ahead
//   caching. Both save and load are bandwidth-bound, making O(n) the
//   theoretical lower bound.
