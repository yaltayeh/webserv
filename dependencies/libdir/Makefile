BUILD_PATH ?= ./build

NAME = $(BUILD_PATH)/libdir.a

FILES = src/DirectiveFactory.cpp	\
		src/Parser.cpp					\

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -MMD -g -std=c++98
CPPFLAGS = -I./include -I./directives/include

OFILES = $(patsubst src/%.cpp,$(BUILD_PATH)/%.o,$(filter src/%,$(FILES))) \
         $(patsubst directives/src/%.cpp,$(BUILD_PATH)/directives/%.o,$(filter directives/%,$(FILES)))
DFILES = ${OFILES:.o=.d}

all: $(NAME)

$(NAME): $(OFILES)
	$(AR) rcs $@ $^

$(BUILD_PATH)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_PATH)/directives/%.o: directives/src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(OFILES) $(DFILES)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

test: all
	c++ -I./include -I./directives/include -std=c++98 tests/test.cpp -L$(BUILD_PATH) -Wl,-force_load,$(NAME) -o tests/test_executable
	@echo "==================================================================="
	@echo "Testing with tests/server.conf"
	@echo "==================================================================="
	./tests/test_executable tests/server.conf
	@echo ""
	@echo "==================================================================="
	@echo "Testing with tests/webserve.conf"
	@echo "==================================================================="
	./tests/test_executable tests/webserve.conf
	@echo ""
	@echo "==================================================================="
	@echo "Testing with tests/complete.conf"
	@echo "==================================================================="
	./tests/test_executable tests/complete.conf

-include $(DFILES)