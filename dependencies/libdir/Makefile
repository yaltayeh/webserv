BUILD_PATH ?= ./build

NAME = $(BUILD_PATH)/libdir.a

FILES = src/Parser.cpp 							\
		src/IDirective.cpp						\
		src/DirectiveFactory.cpp				\
		src/Config.cpp							\
		src/Directives/GlobalDirective.cpp		\
		src/Directives/ExampleDirective.cpp		\

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -MMD -g -std=c++98
CPPFLAGS = -I./include

OFILES = ${FILES:src/%.cpp=$(BUILD_PATH)/%.o}
DFILES = ${OFILES:$(BUILD_PATH)/%.o=$(BUILD_PATH)/%.d}

all: $(NAME)

$(NAME): $(OFILES)
	$(AR) rcs $@ $^

$(BUILD_PATH)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OFILES) $(DFILES)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include $(DFILES)