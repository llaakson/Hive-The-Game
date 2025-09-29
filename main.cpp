#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <optional>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <array>

#include "Hive.hpp"

namespace {

constexpr float kHexRadius = 48.f;
constexpr float kPanelWidth = 260.f;
constexpr float kHexOutline = 2.5f;
constexpr float kHighlightAlpha = 110.f;
constexpr float kSqrt3 = 1.7320508075688772f;

struct Selection {
	enum class Kind { None, Pool, Board };
	Kind kind{Kind::None};
	int pieceId{-1};
	std::vector<Move> moves;

	void reset() {
		kind = Kind::None;
		pieceId = -1;
		moves.clear();
	}
};

struct InventoryItem {
	int pieceId{-1};
	int owner{0};
	PieceType type{};
	bool enabled{false};
	sf::RectangleShape shape;
	sf::Text label;
};

std::string playerName(int player) {
	return player == 0 ? "White" : "Black";
}

sf::Color playerFill(int player) {
	return player == 0 ? sf::Color(236, 231, 206) : sf::Color(70, 78, 96);
}

sf::Color playerOutline(int player) {
	return player == 0 ? sf::Color(90, 90, 90) : sf::Color(220, 220, 220);
}

sf::Vector2f axialToPixel(const HexCoord& hex) {
	float x = kHexRadius * (1.5f * static_cast<float>(hex.q));
	float y = kHexRadius * (kSqrt3 * (static_cast<float>(hex.r) + static_cast<float>(hex.q) / 2.f));
	return {x, y};
}

HexCoord pixelToHex(const sf::Vector2f& point) {
	float q = (2.f / 3.f * point.x) / kHexRadius;
	float r = (-1.f / 3.f * point.x + kSqrt3 / 3.f * point.y) / kHexRadius;

	float x = q;
	float z = r;
	float y = -x - z;

	float rx = std::round(x);
	float ry = std::round(y);
	float rz = std::round(z);

	float xDiff = std::fabs(rx - x);
	float yDiff = std::fabs(ry - y);
	float zDiff = std::fabs(rz - z);

	if (xDiff > yDiff && xDiff > zDiff) {
		rx = -ry - rz;
	} else if (yDiff > zDiff) {
		ry = -rx - rz;
	} else {
		rz = -rx - ry;
	}

	return {static_cast<int>(rx), static_cast<int>(rz)};
}

float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return std::sqrt(dx * dx + dy * dy);
}

bool containsPoint(const sf::RectangleShape& shape, const sf::Vector2f& point) {
	return shape.getGlobalBounds().contains(point);
}

void buildInventoryItems(const HiveGame& game,
						 const std::unordered_map<int, std::vector<Move>>& movesByPiece,
						 const sf::Font& font,
						 float panelX,
						 std::vector<InventoryItem>& outItems,
						 std::vector<sf::Text>& outHeaders,
						 std::vector<sf::Text>& outTypeLabels) {
	outItems.clear();
	outHeaders.clear();
	outTypeLabels.clear();

	const float margin = 16.f;
	const float itemSize = 56.f;
	const float spacing = 10.f;
	const int itemsPerRow = 3;
	const std::array<PieceType, 5> typeOrder = {
		PieceType::QueenBee,
		PieceType::Beetle,
		PieceType::Spider,
		PieceType::Grasshopper,
		PieceType::SoldierAnt
	};

	float y = margin;
	for (int player = 0; player < 2; ++player) {
		sf::Text header;
		header.setFont(font);
		header.setCharacterSize(20);
		header.setFillColor(sf::Color::White);
		header.setString(playerName(player) + " Reserve");
		header.setPosition(panelX + margin, y);
		outHeaders.push_back(header);
		y += header.getLocalBounds().height + 10.f;

		auto unplaced = game.unplacedPieces(player);
		for (PieceType type : typeOrder) {
			std::vector<int> ids;
			for (int pieceId : unplaced) {
				if (game.piece(pieceId).type == type) {
					ids.push_back(pieceId);
				}
			}
			if (ids.empty()) {
				continue;
			}

			sf::Text typeLabel;
			typeLabel.setFont(font);
			typeLabel.setCharacterSize(16);
			typeLabel.setFillColor(sf::Color(200, 200, 210));
			typeLabel.setString(pieceTypeName(type));
			typeLabel.setPosition(panelX + margin, y);
			outTypeLabels.push_back(typeLabel);
			y += typeLabel.getLocalBounds().height + 8.f;

			int rows = static_cast<int>((ids.size() + itemsPerRow - 1) / itemsPerRow);
			for (std::size_t index = 0; index < ids.size(); ++index) {
				int pieceId = ids[index];
				int row = static_cast<int>(index) / itemsPerRow;
				int col = static_cast<int>(index) % itemsPerRow;

				float x = panelX + margin + col * (itemSize + spacing);
				float itemY = y + row * (itemSize + spacing);

				InventoryItem item;
				item.pieceId = pieceId;
				item.owner = player;
				item.type = type;
				item.enabled = movesByPiece.count(pieceId) > 0;
				item.shape.setSize({itemSize, itemSize});
				item.shape.setPosition(x, itemY);
				item.shape.setFillColor(item.enabled ? sf::Color(60, 70, 90) : sf::Color(45, 52, 66));
				item.shape.setOutlineColor(item.enabled ? sf::Color(180, 190, 210) : sf::Color(90, 100, 120));
				item.shape.setOutlineThickness(2.f);

				item.label.setFont(font);
				item.label.setString(std::string(1, pieceTypeLabel(type)));
				item.label.setCharacterSize(24);
				item.label.setFillColor(sf::Color(230, 230, 240));
				sf::FloatRect bounds = item.label.getLocalBounds();
				item.label.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
				item.label.setPosition(x + itemSize / 2.f, itemY + itemSize / 2.f);

				outItems.push_back(item);
			}

			y += rows * (itemSize + spacing) + 12.f;
		}

		y += 24.f; // space between players
	}
}

std::unordered_map<int, std::vector<Move>> mapMovesByPiece(const std::vector<Move>& legalMoves) {
	std::unordered_map<int, std::vector<Move>> result;
	for (const Move& move : legalMoves) {
		if (move.pieceId >= 0) {
			result[move.pieceId].push_back(move);
		}
	}
	return result;
}

std::optional<Move> extractPassMove(const std::vector<Move>& legalMoves) {
	for (const Move& move : legalMoves) {
		if (move.type == MoveType::Pass) {
			return move;
		}
	}
	return std::nullopt;
}

sf::Color highlightColorForMove(const Move& move, const HiveGame& game) {
	if (move.type == MoveType::Place) {
		return sf::Color(90, 160, 150, static_cast<sf::Uint8>(kHighlightAlpha));
	}
	if (move.type == MoveType::Move && move.to.has_value() && game.stacks().count(move.to.value()) && !game.stacks().at(move.to.value()).empty()) {
		return sf::Color(180, 110, 60, static_cast<sf::Uint8>(kHighlightAlpha));
	}
	return sf::Color(120, 180, 90, static_cast<sf::Uint8>(kHighlightAlpha));
}

} // namespace

int main() {
	sf::RenderWindow window(sf::VideoMode(1280, 900), "Hive");
	window.setFramerateLimit(60);

	sf::Font font;
	if (!font.loadFromFile("arial.ttf")) {
		std::cerr << "Failed to load font.\n";
		return 1;
	}

	HiveGame game;
	Selection selection;

	sf::Vector2f boardOffset(0.f, 0.f);

	while (window.isOpen()) {
		float windowWidth = static_cast<float>(window.getSize().x);
		float windowHeight = static_cast<float>(window.getSize().y);
		float boardWidth = windowWidth - kPanelWidth;
		sf::Vector2f boardOrigin(boardWidth / 2.f, windowHeight / 2.f);

		const std::vector<Move>& legalMoves = game.legalMoves();
		auto movesByPiece = mapMovesByPiece(legalMoves);
		auto passMove = extractPassMove(legalMoves);

		std::vector<InventoryItem> inventoryItems;
		std::vector<sf::Text> inventoryHeaders;
		std::vector<sf::Text> inventoryTypeLabels;
		buildInventoryItems(game, movesByPiece, font, boardWidth, inventoryItems, inventoryHeaders, inventoryTypeLabels);

		bool stateChanged = false;

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
				break;
			}
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Escape) {
					window.close();
					break;
				}
				if (event.key.code == sf::Keyboard::R) {
					game = HiveGame();
					selection.reset();
					boardOffset = {0.f, 0.f};
					stateChanged = true;
					break;
				}
				if (event.key.code == sf::Keyboard::Left) {
					boardOffset.x += 25.f;
				}
				if (event.key.code == sf::Keyboard::Right) {
					boardOffset.x -= 25.f;
				}
				if (event.key.code == sf::Keyboard::Up) {
					boardOffset.y += 25.f;
				}
				if (event.key.code == sf::Keyboard::Down) {
					boardOffset.y -= 25.f;
				}
			}
			if (event.type == sf::Event::MouseButtonPressed) {
				sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
				if (event.mouseButton.button == sf::Mouse::Right) {
					selection.reset();
				}
				if (event.mouseButton.button == sf::Mouse::Left) {
					if (mousePos.x >= boardWidth) {
						// Panel interaction
						bool consumed = false;
						for (const InventoryItem& item : inventoryItems) {
							if (!item.enabled) {
								continue;
							}
							if (containsPoint(item.shape, mousePos) && item.owner == game.currentPlayer()) {
								selection.kind = Selection::Kind::Pool;
								selection.pieceId = item.pieceId;
								selection.moves = movesByPiece[item.pieceId];
								consumed = true;
								break;
							}
						}
						if (!consumed && passMove.has_value()) {
							sf::FloatRect passArea(boardWidth + 16.f, windowHeight - 70.f, kPanelWidth - 32.f, 48.f);
							if (passArea.contains(mousePos)) {
								if (game.applyMove(passMove.value())) {
									selection.reset();
									stateChanged = true;
									break;
								}
							}
						}
					} else {
						// Board interaction
						sf::Vector2f relative = mousePos - boardOrigin - boardOffset;
						HexCoord targetHex = pixelToHex(relative);
						sf::Vector2f center = boardOrigin + boardOffset + axialToPixel(targetHex);
						if (distance(mousePos, center) <= kHexRadius * 0.95f) {
							auto stackIt = game.stacks().find(targetHex);
							bool occupied = stackIt != game.stacks().end() && !stackIt->second.empty();
							if (occupied) {
								int topPieceId = stackIt->second.back();
								const Piece& topPiece = game.piece(topPieceId);
								if (topPiece.owner == game.currentPlayer() && movesByPiece.count(topPieceId)) {
									selection.kind = Selection::Kind::Board;
									selection.pieceId = topPieceId;
									selection.moves = movesByPiece[topPieceId];
								} else if (selection.kind != Selection::Kind::None) {
									// Attempt to move/ place onto occupied hex (e.g., beetle move)
									bool applied = false;
									for (const Move& move : selection.moves) {
										if (move.to.has_value() && move.to.value() == targetHex) {
											if (game.applyMove(move)) {
												selection.reset();
												stateChanged = true;
												applied = true;
											}
											break;
										}
									}
									if (applied) {
										break;
									}
								}
							} else {
								// Empty hex: attempt to apply selected move or placement
								bool applied = false;
								for (const Move& move : selection.moves) {
									if (move.to.has_value() && move.to.value() == targetHex) {
										if (game.applyMove(move)) {
											selection.reset();
											stateChanged = true;
											applied = true;
										}
										break;
									}
								}
								if (applied) {
									break;
								}
							}
						}
					}
				}
			}
		}

		if (stateChanged) {
			continue;
		}

		// Ensure selection is still valid
		if (selection.kind != Selection::Kind::None) {
			auto it = movesByPiece.find(selection.pieceId);
			if (it == movesByPiece.end() || it->second.empty()) {
				selection.reset();
			} else {
				// Keep only current legal moves
				selection.moves = it->second;
			}
		}

		window.clear(sf::Color(28, 32, 39));

		// Panel background
		sf::RectangleShape panelBg({kPanelWidth, windowHeight});
		panelBg.setPosition(boardWidth, 0.f);
		panelBg.setFillColor(sf::Color(34, 39, 48));
		window.draw(panelBg);

		// Highlight legal destinations
		for (const Move& move : selection.moves) {
			if (!move.to.has_value()) {
				continue;
			}
			sf::CircleShape highlight(kHexRadius, 6);
			highlight.setOrigin(kHexRadius, kHexRadius);
			highlight.setRotation(30.f);
			highlight.setPosition(boardOrigin + boardOffset + axialToPixel(move.to.value()));
			highlight.setFillColor(highlightColorForMove(move, game));
			window.draw(highlight);
		}

		// Draw board pieces
		for (const auto& [coord, stack] : game.stacks()) {
			if (stack.empty()) {
				continue;
			}
			int topId = stack.back();
			const Piece& piece = game.piece(topId);
			sf::Vector2f center = boardOrigin + boardOffset + axialToPixel(coord);

			sf::CircleShape hex(kHexRadius, 6);
			hex.setOrigin(kHexRadius, kHexRadius);
			hex.setRotation(30.f);
			hex.setPosition(center);
			sf::Color fill = playerFill(piece.owner);
			if (stack.size() > 1) {
				fill = sf::Color(static_cast<sf::Uint8>(fill.r * 0.85f), static_cast<sf::Uint8>(fill.g * 0.85f), static_cast<sf::Uint8>(fill.b * 0.85f));
			}
			hex.setFillColor(fill);
			hex.setOutlineColor(playerOutline(piece.owner));
			hex.setOutlineThickness((selection.kind == Selection::Kind::Board && selection.pieceId == topId) ? 5.f : kHexOutline);
			window.draw(hex);

			// Piece label
			sf::Text label;
			label.setFont(font);
			label.setString(std::string(1, pieceTypeLabel(piece.type)));
			label.setCharacterSize(26);
			label.setFillColor(piece.owner == 0 ? sf::Color::Black : sf::Color(235, 235, 240));
			sf::FloatRect bounds = label.getLocalBounds();
			label.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
			label.setPosition(center);
			window.draw(label);

			if (stack.size() > 1) {
				sf::Text stackLabel;
				stackLabel.setFont(font);
				stackLabel.setString("+" + std::to_string(stack.size() - 1));
				stackLabel.setCharacterSize(16);
				stackLabel.setFillColor(sf::Color(220, 220, 230));
				sf::FloatRect sBounds = stackLabel.getLocalBounds();
				stackLabel.setOrigin(sBounds.left + sBounds.width, sBounds.top + sBounds.height);
				stackLabel.setPosition(center.x + kHexRadius * 0.6f, center.y + kHexRadius * 0.6f);
				window.draw(stackLabel);
			}
		}

		// Draw inventory headers and type labels
		for (const sf::Text& text : inventoryHeaders) {
			window.draw(text);
		}
		for (const sf::Text& text : inventoryTypeLabels) {
			window.draw(text);
		}

		// Draw inventory items
		for (InventoryItem& item : inventoryItems) {
			if (selection.kind == Selection::Kind::Pool && selection.pieceId == item.pieceId) {
				item.shape.setOutlineColor(sf::Color(255, 215, 100));
				item.shape.setOutlineThickness(3.f);
			}
			window.draw(item.shape);
			window.draw(item.label);
		}

		// Pass button
		if (passMove.has_value()) {
			sf::RectangleShape passButton({kPanelWidth - 32.f, 48.f});
			passButton.setPosition(boardWidth + 16.f, windowHeight - 70.f);
			passButton.setFillColor(sf::Color(78, 86, 102));
			passButton.setOutlineThickness(2.f);
			passButton.setOutlineColor(sf::Color(160, 170, 190));
			window.draw(passButton);

			sf::Text passText;
			passText.setFont(font);
			passText.setString("Pass");
			passText.setCharacterSize(22);
			passText.setFillColor(sf::Color::White);
			sf::FloatRect pbounds = passText.getLocalBounds();
			passText.setOrigin(pbounds.left + pbounds.width / 2.f, pbounds.top + pbounds.height / 2.f);
			passText.setPosition(passButton.getPosition().x + passButton.getSize().x / 2.f,
								 passButton.getPosition().y + passButton.getSize().y / 2.f);
			window.draw(passText);
		}

		// Status information
		sf::Text status;
		status.setFont(font);
		status.setCharacterSize(20);
		status.setFillColor(sf::Color(220, 220, 230));
		std::ostringstream statusStream;
		if (game.isGameOver()) {
			if (game.isDraw()) {
				statusStream << "Game over: Draw";
			} else if (auto win = game.winner()) {
				statusStream << "Game over: " << playerName(*win) << " wins";
			}
		} else {
			statusStream << "Turn: " << playerName(game.currentPlayer());
			bool queenPlaced = game.queenPlaced(game.currentPlayer());
			bool mustPlaceQueen = !queenPlaced && game.movesPlayed(game.currentPlayer()) >= 3;
			if (mustPlaceQueen) {
				statusStream << " (must place Queen)";
			} else if (!queenPlaced) {
				statusStream << " (Queen not placed)";
			}
		}
		status.setString(statusStream.str());
		status.setPosition(boardWidth + 16.f, windowHeight - 120.f);
		window.draw(status);

		window.display();
	}

	return 0;
}

