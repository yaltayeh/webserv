NAME = webserve

FILES = src/Request.cpp 	\
	src/HttpHeaders.cpp		\
	src/Response.cpp		\
	src/Http.cpp			\
	src/main.cpp			\

CC = c++
CFLAGS = -Wall -Wextra -Werror -MMD -std=c++98
CPPFLAGS = -I./include

OFILES = ${FILES:src/%.cpp=build/%.o}
DFILES = ${OFILES:build/%.o=build/%.d}

all: $(NAME)

$(NAME): $(OFILES)
	$(CC) $^ -o $@

build:
	mkdir -p build

build/%.o: src/%.cpp | build
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OFILES) $(DFILES)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include $(DFILES)