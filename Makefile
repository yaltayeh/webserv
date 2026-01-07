NAME		    		= $(BUILD_PATH)/webserv
CXX 					= c++
CXXFLAGS 				= -Wall -Wextra -Werror -std=c++98 -Iinclude
BUILD_PATH	      		?= ./build

# Dependencies (order matters: libhttp depends on libutils)
DEPENDENCIES	    	= libutils libhttp
DEPENDENCIES_DIR		= ./dependencies
DEPENDENCIES_FLAGS      = $(foreach lib,$(DEPENDENCIES),-L$(BUILD_PATH)/$(lib)) \
							$(foreach lib,$(patsubst lib%,%, $(DEPENDENCIES)),-l$(lib))
INCLUDE_FLAGS	   		= -Iinclude $(foreach lib,$(DEPENDENCIES),-I$(DEPENDENCIES_DIR)/$(lib)/include)

SRCS		    		=   src/main.cpp \
							src/Server.cpp \
							src/ConnectionManager.cpp \
							src/EventLoop.cpp
OBJS		   			 = $(SRCS:src/%.cpp=$(BUILD_PATH)/obj/%.o)

all: $(NAME)

$(NAME): $(OBJS) $(foreach lib,$(DEPENDENCIES),$(BUILD_PATH)/$(lib)/$(lib).a)
		$(CXX) $(CXXFLAGS) $(OBJS) $(DEPENDENCIES_FLAGS) $(INCLUDE_FLAGS) -o $(NAME)

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
