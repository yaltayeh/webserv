NAME		    		= $(BUILD_PATH)/webserv
CXX 					= c++
CXXFLAGS 				= -Wall -Wextra -Werror -std=c++98 -Iinclude
BUILD_PATH	      		?= ./build

# Dependencies (order matters for linking: libhttp depends on libutils and libdir)
DEPENDENCIES	    	= libhttp libutils libdir
DEPENDENCIES_DIR		= ./dependencies

#if you need to force load static libraries (macOS)
ifeq ($(shell uname),Darwin)
DEPENDENCIES_FLAGS	  = $(foreach lib,$(DEPENDENCIES),-L$(BUILD_PATH)/$(lib)) \
							$(foreach lib,$(patsubst lib%,%, $(DEPENDENCIES)), -Wl,-force_load,$(BUILD_PATH)/lib$(lib)/lib$(lib).a)
else
DEPENDENCIES_FLAGS      = $(foreach lib,$(DEPENDENCIES),-L$(BUILD_PATH)/$(lib)) \
							$(foreach lib,$(patsubst lib%,%, $(DEPENDENCIES)), -l$(lib))
endif

INCLUDE_FLAGS	   		= -Iinclude -Iinclude/directives -Idependencies $(foreach lib,$(DEPENDENCIES),-I$(DEPENDENCIES_DIR)/$(lib)/include)

SRCS		    		=   src/main.cpp 										\
							src/Socket.cpp 										\
							src/ClientSocket.cpp 								\
							src/ListenSocket.cpp 								\
							src/VirtualServer.cpp 								\
							src/EventLoop.cpp									\
							src/RequestRouter.cpp								\
							src/RegEx.cpp										\
							\
							src/directives/Core/LocationDirective.cpp			\
							src/directives/Core/ServerDirective.cpp				\
							src/directives/Core/HttpDirective.cpp				\
							src/directives/AutoindexExactSizeDirective.cpp		\
							src/directives/ErrorPageDirective.cpp				\
							src/directives/ClientBodyTempPathDirective.cpp		\
							src/directives/RewriteDirective.cpp					\
							src/directives/AutoindexFormatDirective.cpp			\
							src/directives/CreateFullPutPathDirective.cpp		\
							src/directives/FastcgiPassDirective.cpp				\
							src/directives/RootDirective.cpp					\
							src/directives/ServerNameDirective.cpp				\
							src/directives/IncludeDirective.cpp					\
							src/directives/FastcgiParamDirective.cpp			\
							src/directives/IndexDirective.cpp					\
							src/directives/ClientMaxBodySizeDirective.cpp		\
							src/directives/ListenDirective.cpp					\
							src/directives/FastcgiSplitPathInfoDirective.cpp	\
							src/directives/AutoindexDirective.cpp				\
							src/directives/FastcgiIndexDirective.cpp			\
							src/directives/ReturnDirective.cpp					\
							src/directives/AliasDirective.cpp					\
							src/directives/AllowedMethodsDirective.cpp			\
							src/directives/UploadEnableDirective.cpp			\
							src/directives/UploadStoreDirective.cpp				\
							src/directives/CgiPassDirective.cpp					\
							src/directives/CgiExtensionDirective.cpp			\
							src/directives/DenyDirective.cpp					\


OBJS		   			 = $(SRCS:src/%.cpp=$(BUILD_PATH)/obj/%.o)

all: $(NAME)

$(NAME): $(OBJS) $(foreach lib,$(DEPENDENCIES),$(BUILD_PATH)/$(lib)/$(lib).a)
		$(CXX) $(OBJS) $(DEPENDENCIES_FLAGS) -o $(NAME)

$(foreach lib,$(DEPENDENCIES),$(BUILD_PATH)/$(lib)/$(lib).a):
		$(MAKE) -C $(DEPENDENCIES_DIR)/$(basename $(@F)) BUILD_PATH=../../$(BUILD_PATH)/$(basename $(@F)) all
	
$(BUILD_PATH)/obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_PATH)/obj
	@rm -rf $(BUILD_PATH)/lib*/obj

fclean: clean
	@rm -rf $(NAME)
	@rm -rf $(BUILD_PATH)

re: fclean all

# Test target
TEST_NAME = $(BUILD_PATH)/test_cgi
TEST_SRCS = tests/test_cgi.cpp
TEST_OBJS = $(TEST_SRCS:tests/%.cpp=$(BUILD_PATH)/obj/tests/%.o)

test: $(TEST_NAME)
	./$(TEST_NAME)

$(TEST_NAME): $(TEST_OBJS) $(foreach lib,$(DEPENDENCIES),$(BUILD_PATH)/$(lib)/$(lib).a)
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) $(DEPENDENCIES_FLAGS) $(INCLUDE_FLAGS) -o $(TEST_NAME)

$(BUILD_PATH)/obj/tests/%.o: tests/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

.PHONY: all clean re fclean test
