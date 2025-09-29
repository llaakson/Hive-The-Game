#include "Hive.hpp"

#include <algorithm>
#include <functional>
#include <utility>
#include <cassert>
#include <cmath>

namespace {

int pieceTypeDisplayOrder(PieceType type) {
	switch (type) {
	case PieceType::QueenBee:    return 0;
	case PieceType::Beetle:      return 1;
	case PieceType::Spider:      return 2;
	case PieceType::Grasshopper: return 3;
	case PieceType::SoldierAnt:  return 4;
	}
	return 5;
}
}

int directionIndex(const HexCoord& delta) {
	const auto& dirs = hexDirections();
	for (int i = 0; i < static_cast<int>(dirs.size()); ++i) {
		if (delta == dirs[i]) {
			return i;
		}
	}
	return -1;
}

std::string pieceTypeName(PieceType type) {
	switch (type) {
	case PieceType::QueenBee:    return "Queen Bee";
	case PieceType::Beetle:      return "Beetle";
	case PieceType::Spider:      return "Spider";
	case PieceType::Grasshopper: return "Grasshopper";
	case PieceType::SoldierAnt:  return "Soldier Ant";
	}
	return "Unknown";
}

char pieceTypeLabel(PieceType type) {
	switch (type) {
	case PieceType::QueenBee:    return 'Q';
	case PieceType::Beetle:      return 'B';
	case PieceType::Spider:      return 'S';
	case PieceType::Grasshopper: return 'G';
	case PieceType::SoldierAnt:  return 'A';
	}
	return '?';
}

HiveGame::HiveGame() {
	m_pieces.reserve(22);

	auto addPiece = [&](int owner, PieceType type) {
		Piece piece;
		piece.id = static_cast<int>(m_pieces.size());
		piece.type = type;
		piece.owner = owner;
		m_pieces.push_back(piece);
		return piece.id;
	};

	for (int owner = 0; owner < 2; ++owner) {
		int queenId = addPiece(owner, PieceType::QueenBee);
		m_queenPieceId[owner] = queenId;
		addPiece(owner, PieceType::Beetle);
		addPiece(owner, PieceType::Beetle);
		addPiece(owner, PieceType::Spider);
		addPiece(owner, PieceType::Spider);
		addPiece(owner, PieceType::Grasshopper);
		addPiece(owner, PieceType::Grasshopper);
		addPiece(owner, PieceType::Grasshopper);
		addPiece(owner, PieceType::SoldierAnt);
		addPiece(owner, PieceType::SoldierAnt);
		addPiece(owner, PieceType::SoldierAnt);
	}
}

bool HiveGame::isTopPiece(int pieceId) const {
	if (pieceId < 0 || pieceId >= static_cast<int>(m_pieces.size())) {
		return false;
	}
	const Piece& piece = m_pieces[pieceId];
	if (!piece.placed) {
		return false;
	}
	auto it = m_board.find(piece.coord);
	if (it == m_board.end() || it->second.empty()) {
		return false;
	}
	return it->second.back() == pieceId;
}

std::vector<int> HiveGame::unplacedPieces(int player) const {
	std::vector<int> result;
	for (const Piece& piece : m_pieces) {
		if (piece.owner == player && !piece.placed) {
			result.push_back(piece.id);
		}
	}
	std::sort(result.begin(), result.end(), [&](int lhs, int rhs) {
		const Piece& a = m_pieces[lhs];
		const Piece& b = m_pieces[rhs];
		int orderA = pieceTypeDisplayOrder(a.type);
		int orderB = pieceTypeDisplayOrder(b.type);
		if (orderA != orderB) {
			return orderA < orderB;
		}
		return lhs < rhs;
	});
	return result;
}

const std::vector<Move>& HiveGame::legalMoves() const {
	if (m_legalMovesDirty) {
		m_cachedLegalMoves = computeLegalMoves();
		m_legalMovesDirty = false;
	}
	return m_cachedLegalMoves;
}

bool HiveGame::moveIsLegal(const Move& move) const {
	const auto& moves = legalMoves();
	for (const Move& candidate : moves) {
		if (sameMove(candidate, move)) {
			return true;
		}
	}
	return false;
}

bool HiveGame::applyMove(const Move& move) {
	if (m_gameOver) {
		return false;
	}

	const std::vector<Move>& moves = legalMoves();
	const Move* selected = nullptr;
	for (const Move& candidate : moves) {
		if (sameMove(candidate, move)) {
			selected = &candidate;
			break;
		}
	}
	if (!selected) {
		return false;
	}

	const Move& applied = *selected;
	int player = m_currentPlayer;

	if (applied.type == MoveType::Place) {
		assert(applied.to.has_value());
		Piece& piece = m_pieces[applied.pieceId];
		piece.placed = true;
		piece.coord = applied.to.value();
		auto& stack = m_board[piece.coord];
		stack.push_back(piece.id);
		if (piece.type == PieceType::QueenBee) {
			m_queenPlaced[player] = true;
		}
	} else if (applied.type == MoveType::Move) {
		assert(applied.to.has_value());
		assert(applied.from.has_value());
		Piece& piece = m_pieces[applied.pieceId];
		auto it = m_board.find(piece.coord);
		if (it != m_board.end()) {
			auto& stackFrom = it->second;
			if (!stackFrom.empty() && stackFrom.back() == piece.id) {
				stackFrom.pop_back();
				if (stackFrom.empty()) {
					m_board.erase(it);
				}
			} else {
				// Should never happen for legal moves.
				return false;
			}
		}
		piece.coord = applied.to.value();
		auto& stackTo = m_board[piece.coord];
		stackTo.push_back(piece.id);
	} else if (applied.type == MoveType::Pass) {
		// nothing to do
	}

	m_movesPlayed[player] += 1;
	m_turnNumber += 1;

	updateGameResult();

	m_currentPlayer = 1 - m_currentPlayer;
	m_legalMovesDirty = true;

	return true;
}

std::optional<int> HiveGame::winner() const {
	if (!m_gameOver) {
		return std::nullopt;
	}
	if (m_result == Result::Player0Wins) {
		return 0;
	}
	if (m_result == Result::Player1Wins) {
		return 1;
	}
	return std::nullopt;
}

std::vector<Move> HiveGame::computeLegalMoves() const {
	std::vector<Move> moves;
	if (m_gameOver) {
		return moves;
	}

	appendPlacementMoves(moves);
	appendMovementMoves(moves);

	if (moves.empty()) {
		Move pass;
		pass.type = MoveType::Pass;
		pass.pieceId = -1;
		moves.push_back(pass);
	}

	return moves;
}

void HiveGame::appendPlacementMoves(std::vector<Move>& moves) const {
	const int player = m_currentPlayer;
	bool mustPlaceQueen = !m_queenPlaced[player] && m_movesPlayed[player] >= 3;

	const std::vector<HexCoord> positions = computePlacementHexes(player);
	if (positions.empty()) {
		return;
	}

	for (const Piece& piece : m_pieces) {
		if (piece.owner != player || piece.placed) {
			continue;
		}
		if (mustPlaceQueen && piece.type != PieceType::QueenBee) {
			continue;
		}
		for (const HexCoord& target : positions) {
			// Ensure the space is empty in the actual board (should already be).
			if (isOccupied(m_board, target)) {
				continue;
			}
			Move move;
			move.type = MoveType::Place;
			move.pieceId = piece.id;
			move.to = target;
			moves.push_back(move);
		}
	}
}

void HiveGame::appendMovementMoves(std::vector<Move>& moves) const {
	const int player = m_currentPlayer;
	if (!m_queenPlaced[player]) {
		return; // cannot move until queen placed
	}

	for (const Piece& piece : m_pieces) {
		if (piece.owner != player || !piece.placed) {
			continue;
		}
		if (!isTopPiece(piece.id)) {
			continue;
		}
		if (wouldBreakHive(piece.id)) {
			continue;
		}
		switch (piece.type) {
		case PieceType::QueenBee:
			addQueenMoves(piece, moves);
			break;
		case PieceType::Beetle:
			addBeetleMoves(piece, moves);
			break;
		case PieceType::Grasshopper:
			addGrasshopperMoves(piece, moves);
			break;
		case PieceType::Spider:
			addSpiderMoves(piece, moves);
			break;
		case PieceType::SoldierAnt:
			addAntMoves(piece, moves);
			break;
		}
	}
}

std::vector<HexCoord> HiveGame::computePlacementHexes(int player) const {
	std::vector<HexCoord> result;
	if (m_board.empty()) {
		result.push_back({0, 0});
		return result;
	}

	bool hasOwnPieceInPlay = false;
	for (const Piece& piece : m_pieces) {
		if (piece.owner == player && piece.placed) {
			hasOwnPieceInPlay = true;
			break;
		}
	}

	std::unordered_set<HexCoord, HexCoordHash> candidates;
	for (const auto& [coord, stack] : m_board) {
		if (stack.empty()) {
			continue;
		}
		for (const HexCoord& dir : hexDirections()) {
			HexCoord candidate = coord + dir;
			if (isOccupied(m_board, candidate)) {
				continue;
			}

			bool touchesOpponent = false;
			bool touchesOwn = false;
			for (const HexCoord& adjDir : hexDirections()) {
				HexCoord neighbor = candidate + adjDir;
				auto it = m_board.find(neighbor);
				if (it == m_board.end() || it->second.empty()) {
					continue;
				}
				int topPieceId = it->second.back();
				const Piece& top = m_pieces[topPieceId];
				if (top.owner == player) {
					touchesOwn = true;
				} else {
					touchesOpponent = true;
				}
			}

			if (!hasOwnPieceInPlay) {
				// First placement for this player: allowed even if touching opponent.
				candidates.insert(candidate);
			} else {
				if (touchesOpponent) {
					continue;
				}
				if (!touchesOwn) {
					continue;
				}
				candidates.insert(candidate);
			}
		}
	}

	result.assign(candidates.begin(), candidates.end());
	std::sort(result.begin(), result.end(), [](const HexCoord& a, const HexCoord& b) {
		if (a.q != b.q) {
			return a.q < b.q;
		}
		return a.r < b.r;
	});
	return result;
}

bool HiveGame::wouldBreakHive(int pieceId) const {
	const Piece& piece = m_pieces[pieceId];
	if (!piece.placed) {
		return false;
	}

	StackMap state = m_board;
	removeFromState(state, piece.id, piece.coord);

	std::size_t occupied = 0;
	for (const auto& [coord, stack] : state) {
		if (!stack.empty()) {
			++occupied;
		}
	}
	if (occupied <= 1) {
		return false;
	}

	// Find starting coordinate
	HexCoord start{};
	bool foundStart = false;
	for (const auto& [coord, stack] : state) {
		if (!stack.empty()) {
			start = coord;
			foundStart = true;
			break;
		}
	}
	if (!foundStart) {
		return false;
	}

	std::queue<HexCoord> frontier;
	std::unordered_set<HexCoord, HexCoordHash> visited;
	frontier.push(start);
	visited.insert(start);

	while (!frontier.empty()) {
		HexCoord current = frontier.front();
		frontier.pop();
		for (const HexCoord& dir : hexDirections()) {
			HexCoord neighbor = current + dir;
			if (!isOccupied(state, neighbor)) {
				continue;
			}
			if (visited.insert(neighbor).second) {
				frontier.push(neighbor);
			}
		}
	}

	return visited.size() != occupied;
}

bool HiveGame::canSlide(const StackMap& state, const HexCoord& from, const HexCoord& to) const {
	HexCoord delta = to - from;
	int dirIndexValue = directionIndex(delta);
	if (dirIndexValue < 0) {
		return false;
	}
	const auto& dirs = hexDirections();
	HexCoord left = from + dirs[(dirIndexValue + 5) % 6];
	HexCoord right = from + dirs[(dirIndexValue + 1) % 6];
	bool leftBlocked = isOccupied(state, left);
	bool rightBlocked = isOccupied(state, right);
	return !(leftBlocked && rightBlocked);
}

std::vector<HexCoord> HiveGame::slidingNeighbors(const StackMap& state, const HexCoord& from) const {
	std::vector<HexCoord> result;
	for (const HexCoord& dir : hexDirections()) {
		HexCoord neighbor = from + dir;
		if (isOccupied(state, neighbor)) {
			continue;
		}
		if (canSlide(state, from, neighbor)) {
			result.push_back(neighbor);
		}
	}
	return result;
}

void HiveGame::removeFromState(StackMap& state, int pieceId, const HexCoord& coord) const {
	auto it = state.find(coord);
	if (it == state.end()) {
		return;
	}
	auto& stack = it->second;
	auto removeIt = std::find(stack.begin(), stack.end(), pieceId);
	if (removeIt != stack.end()) {
		stack.erase(removeIt);
	}
	if (stack.empty()) {
		state.erase(it);
	}
}

void HiveGame::addQueenMoves(const Piece& piece, std::vector<Move>& moves) const {
	StackMap state = m_board;
	removeFromState(state, piece.id, piece.coord);

	for (const HexCoord& dir : hexDirections()) {
		HexCoord dest = piece.coord + dir;
		if (isOccupied(m_board, dest)) {
			continue;
		}
		if (!canSlide(state, piece.coord, dest)) {
			continue;
		}
		Move move;
		move.type = MoveType::Move;
		move.pieceId = piece.id;
		move.from = piece.coord;
		move.to = dest;
		move.path.push_back(dest);
		moves.push_back(std::move(move));
	}
}

void HiveGame::addBeetleMoves(const Piece& piece, std::vector<Move>& moves) const {
	StackMap state = m_board;
	removeFromState(state, piece.id, piece.coord);

	for (const HexCoord& dir : hexDirections()) {
		HexCoord dest = piece.coord + dir;
		bool occupiedDest = isOccupied(m_board, dest);
		if (!occupiedDest && !canSlide(state, piece.coord, dest)) {
			continue;
		}
		Move move;
		move.type = MoveType::Move;
		move.pieceId = piece.id;
		move.from = piece.coord;
		move.to = dest;
		move.path.push_back(dest);
		moves.push_back(std::move(move));
	}
}

void HiveGame::addGrasshopperMoves(const Piece& piece, std::vector<Move>& moves) const {
	for (const HexCoord& dir : hexDirections()) {
		HexCoord current = piece.coord + dir;
		bool jumpedAtLeastOne = false;
		while (isOccupied(m_board, current)) {
			jumpedAtLeastOne = true;
			current = current + dir;
		}
		if (jumpedAtLeastOne && !isOccupied(m_board, current)) {
			Move move;
			move.type = MoveType::Move;
			move.pieceId = piece.id;
			move.from = piece.coord;
			move.to = current;
			move.path.push_back(current);
			moves.push_back(std::move(move));
		}
	}
}

void HiveGame::addSpiderMoves(const Piece& piece, std::vector<Move>& moves) const {
	StackMap state = m_board;
	removeFromState(state, piece.id, piece.coord);

	std::vector<HexCoord> path;
	std::unordered_set<HexCoord, HexCoordHash> visited;
	visited.insert(piece.coord);

	const int steps = 3;

	std::function<void(const HexCoord&, int)> dfs = [&](const HexCoord& current, int remaining) {
		if (remaining == 0) {
			if (!path.empty()) {
				Move move;
				move.type = MoveType::Move;
				move.pieceId = piece.id;
				move.from = piece.coord;
				move.to = path.back();
				move.path = path;
				moves.push_back(std::move(move));
			}
			return;
		}

		for (const HexCoord& neighbor : slidingNeighbors(state, current)) {
			if (visited.count(neighbor)) {
				continue;
			}
			visited.insert(neighbor);
			path.push_back(neighbor);
			dfs(neighbor, remaining - 1);
			path.pop_back();
			visited.erase(neighbor);
		}
	};

	dfs(piece.coord, steps);
}

void HiveGame::addAntMoves(const Piece& piece, std::vector<Move>& moves) const {
	StackMap state = m_board;
	removeFromState(state, piece.id, piece.coord);

	std::unordered_set<HexCoord, HexCoordHash> visited;
	std::unordered_map<HexCoord, HexCoord, HexCoordHash> parent;
	std::queue<HexCoord> frontier;

	visited.insert(piece.coord);

	for (const HexCoord& startNeighbor : slidingNeighbors(state, piece.coord)) {
		if (visited.insert(startNeighbor).second) {
			parent[startNeighbor] = piece.coord;
			frontier.push(startNeighbor);
		}
	}

	auto buildPath = [&](const HexCoord& destination) {
		std::vector<HexCoord> resultPath;
		HexCoord current = destination;
		while (current != piece.coord) {
			resultPath.push_back(current);
			auto it = parent.find(current);
			if (it == parent.end()) {
				break;
			}
			current = it->second;
		}
		std::reverse(resultPath.begin(), resultPath.end());
		return resultPath;
	};

	while (!frontier.empty()) {
		HexCoord current = frontier.front();
		frontier.pop();

		Move move;
		move.type = MoveType::Move;
		move.pieceId = piece.id;
		move.from = piece.coord;
		move.to = current;
		move.path = buildPath(current);
		moves.push_back(std::move(move));

		for (const HexCoord& neighbor : slidingNeighbors(state, current)) {
			if (visited.insert(neighbor).second) {
				parent[neighbor] = current;
				frontier.push(neighbor);
			}
		}
	}
}

bool HiveGame::isOccupied(const StackMap& state, const HexCoord& coord) const {
	auto it = state.find(coord);
	if (it == state.end()) {
		return false;
	}
	return !it->second.empty();
}

bool HiveGame::isQueenSurrounded(int player) const {
	if (!m_queenPlaced[player]) {
		return false;
	}
	const Piece& queen = m_pieces[m_queenPieceId[player]];
	if (!queen.placed) {
		return false;
	}
	int occupiedSides = 0;
	for (const HexCoord& dir : hexDirections()) {
		HexCoord neighbor = queen.coord + dir;
		if (isOccupied(m_board, neighbor)) {
			++occupiedSides;
		}
	}
	return occupiedSides == 6;
}

void HiveGame::updateGameResult() {
	bool whiteSurrounded = isQueenSurrounded(0);
	bool blackSurrounded = isQueenSurrounded(1);

	if (whiteSurrounded && blackSurrounded) {
		m_gameOver = true;
		m_result = Result::Draw;
	} else if (whiteSurrounded) {
		m_gameOver = true;
		m_result = Result::Player1Wins;
	} else if (blackSurrounded) {
		m_gameOver = true;
		m_result = Result::Player0Wins;
	}

	if (m_gameOver) {
		m_legalMovesDirty = true;
	}
}

bool HiveGame::sameMove(const Move& lhs, const Move& rhs) const {
	if (lhs.type != rhs.type) {
		return false;
	}
	if (lhs.pieceId != rhs.pieceId) {
		return false;
	}
	if (lhs.from.has_value() != rhs.from.has_value()) {
		return false;
	}
	if (lhs.to.has_value() != rhs.to.has_value()) {
		return false;
	}
	if (lhs.from && lhs.from.value() != rhs.from.value()) {
		return false;
	}
	if (lhs.to && lhs.to.value() != rhs.to.value()) {
		return false;
	}
	return true;
}

