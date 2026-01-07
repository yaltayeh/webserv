NAME = webserve

FILES = src/main.cpp 							\
	    src/Parser.cpp 							\
		src/IDirective.cpp						\
		src/Config.cpp							\
		src/Directives/GlobalDirective.cpp		\
		src/Directives/RootDirective.cpp		\
		src/Directives/HttpDirective.cpp		\
		src/Directives/LocationDirective.cpp	\
		src/Directives/ServerDirective.cpp		\

CC = c++
CFLAGS = -Wall -Wextra -Werror -MMD -g -std=c++98
CPPFLAGS = -I./include

OFILES = ${FILES:src/%.cpp=build/%.o}
DFILES = ${OFILES:build/%.o=build/%.d}

all: $(NAME)

$(NAME): $(OFILES)
	$(CC) $^ -o $@

build:
	mkdir -p build

build/%.o: src/%.cpp | build
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OFILES) $(DFILES)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include $(DFILES)