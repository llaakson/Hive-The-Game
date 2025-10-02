NAME = hive
all: $(NAME)

CPP := c++
FLAGS := -Wall -Wextra -Werror -std=c++17

SFML_LIBS := -lsfml-graphics -lsfml-window -lsfml-system -pthread

SOURCE := main.cpp Engine.cpp Player.cpp Board.cpp
OBJ := $(SOURCE:.cpp=.o)
HEADERS := Hive.hpp Hex.hpp Player.hpp Board.hpp Engine.hpp Tile.hpp

$(NAME) : $(OBJ)
	$(CPP) $(FLAGS) $(OBJ) -o $(NAME) $(SFML_LIBS)

%.o : %.cpp $(HEADERS)
	$(CPP) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
