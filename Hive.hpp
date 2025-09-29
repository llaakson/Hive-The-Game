#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct HexCoord {
	int q{};
	int r{};

	bool operator==(const HexCoord& other) const noexcept {
		return q == other.q && r == other.r;
	}

	bool operator!=(const HexCoord& other) const noexcept {
		return !(*this == other);
	}

	HexCoord operator+(const HexCoord& other) const noexcept {
		return {q + other.q, r + other.r};
	}

	HexCoord operator-(const HexCoord& other) const noexcept {
		return {q - other.q, r - other.r};
	}
};

struct HexCoordHash {
	std::size_t operator()(const HexCoord& coord) const noexcept {
		// Pack q and r into a 64-bit value and hash.
		constexpr std::uint64_t mask = 0xFFFFFFFFull;
		std::uint64_t hq = static_cast<std::uint64_t>(static_cast<std::int64_t>(coord.q)) & mask;
		std::uint64_t hr = static_cast<std::uint64_t>(static_cast<std::int64_t>(coord.r)) & mask;
		std::uint64_t combined = (hq << 32) ^ hr;
		return std::hash<std::uint64_t>{}(combined);
	}
};

inline const std::array<HexCoord, 6>& hexDirections() {
	static const std::array<HexCoord, 6> dirs = {
		HexCoord{1, 0},
		HexCoord{0, 1},
		HexCoord{-1, 1},
		HexCoord{-1, 0},
		HexCoord{0, -1},
		HexCoord{1, -1}
	};
	return dirs;
}

int directionIndex(const HexCoord& delta);

enum class PieceType {
	QueenBee,
	Beetle,
	Spider,
	Grasshopper,
	SoldierAnt
};

std::string pieceTypeName(PieceType type);
char pieceTypeLabel(PieceType type);

enum class MoveType {
	Place,
	Move,
	Pass
};

struct Move {
	MoveType type{MoveType::Pass};
	int pieceId{-1};
	std::optional<HexCoord> from;
	std::optional<HexCoord> to;
	std::vector<HexCoord> path; // optional extra data (e.g., spider route)
};

struct Piece {
	int id{-1};
	PieceType type{};
	int owner{0};
	bool placed{false};
	HexCoord coord{};
};

class HiveGame {
public:
	HiveGame();

	int currentPlayer() const { return m_currentPlayer; }
	const std::vector<Piece>& pieces() const { return m_pieces; }
	const Piece& piece(int id) const { return m_pieces.at(static_cast<std::size_t>(id)); }
	bool isTopPiece(int pieceId) const;
	bool queenPlaced(int player) const { return m_queenPlaced[player]; }
	unsigned movesPlayed(int player) const { return m_movesPlayed[player]; }

	const std::unordered_map<HexCoord, std::vector<int>, HexCoordHash>& stacks() const { return m_board; }
	std::vector<int> unplacedPieces(int player) const;

	const std::vector<Move>& legalMoves() const;
	bool moveIsLegal(const Move& move) const;
	bool applyMove(const Move& move);

	bool isGameOver() const { return m_gameOver; }
	bool isDraw() const { return m_result == Result::Draw; }
	std::optional<int> winner() const;

private:
	using StackMap = std::unordered_map<HexCoord, std::vector<int>, HexCoordHash>;

	std::vector<Move> computeLegalMoves() const;
	void appendPlacementMoves(std::vector<Move>& moves) const;
	void appendMovementMoves(std::vector<Move>& moves) const;

	std::vector<HexCoord> computePlacementHexes(int player) const;

	bool wouldBreakHive(int pieceId) const;
	bool canSlide(const StackMap& state, const HexCoord& from, const HexCoord& to) const;
	std::vector<HexCoord> slidingNeighbors(const StackMap& state, const HexCoord& from) const;

	void removeFromState(StackMap& state, int pieceId, const HexCoord& coord) const;

	void addQueenMoves(const Piece& piece, std::vector<Move>& moves) const;
	void addBeetleMoves(const Piece& piece, std::vector<Move>& moves) const;
	void addGrasshopperMoves(const Piece& piece, std::vector<Move>& moves) const;
	void addSpiderMoves(const Piece& piece, std::vector<Move>& moves) const;
	void addAntMoves(const Piece& piece, std::vector<Move>& moves) const;

	bool isOccupied(const StackMap& state, const HexCoord& coord) const;

	bool isQueenSurrounded(int player) const;
	void updateGameResult();

	bool sameMove(const Move& lhs, const Move& rhs) const;

	StackMap m_board;
	std::vector<Piece> m_pieces;
	std::array<int, 2> m_queenPieceId{};
	int m_currentPlayer{0};
	std::array<bool, 2> m_queenPlaced{false, false};
	std::array<unsigned, 2> m_movesPlayed{0, 0};
	unsigned m_turnNumber{0};

	enum class Result {
		Pending,
		Player0Wins,
		Player1Wins,
		Draw
	};

	Result m_result{Result::Pending};
	bool m_gameOver{false};

	mutable bool m_legalMovesDirty{true};
	mutable std::vector<Move> m_cachedLegalMoves;
};
