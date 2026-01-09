BUILD_PATH ?= ./build

NAME = $(BUILD_PATH)/libdir.a

FILES = src/Parser.cpp 				\
		src/IDirective.cpp			\
		src/DirectiveFactory.cpp	\
		src/Config.cpp				\
		src/GlobalDirective.cpp		\
		src/ExampleDirective.cpp	\
		directives/src/HttpDirective.cpp	\
		directives/src/ServerDirective.cpp	\
		directives/src/LocationDirective.cpp	\
		directives/src/ServerNameDirective.cpp	\
		directives/src/ListenDirective.cpp	\
		directives/src/RootDirective.cpp	\
		directives/src/ErrorPageDirective.cpp	\
		directives/src/ClientMaxBodySizeDirective.cpp	\
		directives/src/KeepaliveTimeoutDirective.cpp	\
		directives/src/AllowedMethodsDirective.cpp	\
		directives/src/ReturnDirective.cpp	\
		directives/src/AutoindexDirective.cpp	\
		directives/src/IndexDirective.cpp	\
		directives/src/UploadEnableDirective.cpp	\
		directives/src/UploadStoreDirective.cpp	\
		directives/src/CgiPassDirective.cpp	\
		directives/src/CgiParamDirective.cpp	\
		directives/src/CgiExtensionDirective.cpp	\

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

-include $(DFILES)